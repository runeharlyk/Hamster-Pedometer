#include <Arduino.h>

#include <ArduinoJson.h>
#include <JsonUtils.h>
#include <stateful_result.h>
#include <SettingValue.h>

#ifndef FACTORY_WIFI_SSID
#define FACTORY_WIFI_SSID ""
#endif

#ifndef FACTORY_WIFI_PASSWORD
#define FACTORY_WIFI_PASSWORD ""
#endif

#ifndef FACTORY_WIFI_HOSTNAME
#define FACTORY_WIFI_HOSTNAME "#{platform}-#{unique_id}"
#endif

#ifndef FACTORY_WIFI_RSSI_THRESHOLD
#define FACTORY_WIFI_RSSI_THRESHOLD -80
#endif

typedef struct {
    String ssid;
    uint8_t bssid[6];
    int32_t channel;
    String password;
    bool staticIPConfig;
    IPAddress localIP;
    IPAddress gatewayIP;
    IPAddress subnetMask;
    IPAddress dnsIP1;
    IPAddress dnsIP2;
    bool available;

    // Serialize this wifi_settings_t instance to a JSON object
    void serialize(JsonObject &json) const {
        json["ssid"] = ssid;
        json["password"] = password;
        json["static_ip_config"] = staticIPConfig;

        if (staticIPConfig) {
            JsonUtils::writeIP(json, "local_ip", localIP);
            JsonUtils::writeIP(json, "gateway_ip", gatewayIP);
            JsonUtils::writeIP(json, "subnet_mask", subnetMask);
            JsonUtils::writeIP(json, "dns_ip_1", dnsIP1);
            JsonUtils::writeIP(json, "dns_ip_2", dnsIP2);
        }
    }

    // Deserialize from a JSON object into this wifi_settings_t instance
    // Returns true if deserialization was successful, false otherwise
    bool deserialize(const JsonObject &json) {
        // Validate SSID and password lengths
        String newSsid = json["ssid"].as<String>();
        String newPassword = json["password"].as<String>();

        if (newSsid.length() < 1 || newSsid.length() > 31 || newPassword.length() > 64) {
            ESP_LOGE("WiFiSettings", "SSID or password length is invalid");
            return false;
        }

        // Basic settings
        ssid = newSsid;
        password = newPassword;
        staticIPConfig = json["static_ip_config"] | false;

        // Extended settings
        if (staticIPConfig) {
            JsonUtils::readIP(json, "local_ip", localIP);
            JsonUtils::readIP(json, "gateway_ip", gatewayIP);
            JsonUtils::readIP(json, "subnet_mask", subnetMask);
            JsonUtils::readIP(json, "dns_ip_1", dnsIP1);
            JsonUtils::readIP(json, "dns_ip_2", dnsIP2);

            // Swap DNS servers if needed
            if (IPUtils::isNotSet(dnsIP1) && IPUtils::isSet(dnsIP2)) {
                dnsIP1 = dnsIP2;
                dnsIP2 = INADDR_NONE;
            }

            // Validate static IP configuration
            if (IPUtils::isNotSet(localIP) || IPUtils::isNotSet(gatewayIP) || IPUtils::isNotSet(subnetMask)) {
                staticIPConfig = false;
                ESP_LOGW("WiFiSettings", "Invalid static IP configuration - falling back to DHCP");
            }
        }

        // Reset scan result
        available = false;
        return true;
    }

} wifi_settings_t;

inline wifi_settings_t createDefaultWiFiSettings() {
    return wifi_settings_t {.ssid = FACTORY_WIFI_SSID,
                            .password = FACTORY_WIFI_PASSWORD,
                            .staticIPConfig = false,
                            .localIP = INADDR_NONE,
                            .gatewayIP = INADDR_NONE,
                            .subnetMask = INADDR_NONE,
                            .dnsIP1 = INADDR_NONE,
                            .dnsIP2 = INADDR_NONE,
                            .available = false};
}

class WiFiSettings {
  public:
    String hostname;
    bool priorityBySignalStrength;
    std::vector<wifi_settings_t> wifiSettings;

    static void read(WiFiSettings &settings, JsonObject &root) {
        root["hostname"] = settings.hostname;
        root["priority_RSSI"] = settings.priorityBySignalStrength;

        JsonArray wifiNetworks = root["wifi_networks"].to<JsonArray>();
        for (const auto &wifi : settings.wifiSettings) {
            JsonObject wifiNetwork = wifiNetworks.add<JsonObject>();
            wifi.serialize(wifiNetwork);
        }

        ESP_LOGV("WiFiSettings", "WiFi Settings read");
    }

    static StateUpdateResult update(JsonObject &root, WiFiSettings &settings) {
        settings.hostname = root["hostname"] | SettingValue::format(FACTORY_WIFI_HOSTNAME);
        settings.priorityBySignalStrength = root["priority_RSSI"] | true;
        settings.wifiSettings.clear();

        if (root["wifi_networks"].is<JsonArray>()) {
            JsonArray wifiNetworks = root["wifi_networks"];
            int networkCount = 0;

            for (JsonObject wifiNetwork : wifiNetworks) {
                if (networkCount >= 5) {
                    ESP_LOGE("WiFiSettings", "Too many wifi networks");
                    break;
                }

                wifi_settings_t newSettings;
                if (newSettings.deserialize(wifiNetwork)) {
                    settings.wifiSettings.push_back(newSettings);
                    networkCount++;
                }
            }
        } else if (String(FACTORY_WIFI_SSID).length() > 0) {
            // Add factory default if no networks configured
            settings.wifiSettings.push_back(createDefaultWiFiSettings());
        }

        ESP_LOGV("WiFiSettings", "WiFi Settings updated");
        return StateUpdateResult::CHANGED;
    }
};