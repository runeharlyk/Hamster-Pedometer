#ifndef ESP32SvelteKit_h
#define ESP32SvelteKit_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32
 *platforms with responsive Sveltekit front-end built with TailwindCSS and
 *DaisyUI. https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Arduino.h>

#include <APSettingsService.h>
#include <AnalyticsService.h>
#include <AuthenticationService.h>
#include <BatteryService.h>
#include <DownloadFirmwareService.h>
#include <ESPFS.h>
#include <ESPmDNS.h>
#include <EventSocket.h>
#include <FeaturesService.h>
#include <MqttSettingsService.h>
#include <MqttStatus.h>
#include <NTPSettingsService.h>
#include <NotificationService.h>
#include <PedoMeter.h>
#include <PsychicHttp.h>
#include <SecuritySettingsService.h>
#include <UploadFirmwareService.h>
#include <WiFi.h>
#include <WiFiSettingsService.h>
#include <system_service.h>

#include <WWWData.h>

#ifndef CORS_ORIGIN
#define CORS_ORIGIN "*"
#endif

#ifndef APP_VERSION
#define APP_VERSION "demo"
#endif

#ifndef APP_NAME
#define APP_NAME "ESP32 SvelteKit Demo"
#endif

class ESP32SvelteKit {
public:
  ESP32SvelteKit(PsychicHttpServer *server, unsigned int numberEndpoints = 115);

  void begin();

  FS *getFS() { return &ESPFS; }

  PsychicHttpServer *getServer() { return _server; }

  SecurityManager *getSecurityManager() { return &_securitySettingsService; }

  EventSocket *getSocket() { return &_socket; }

#if FT_ENABLED(FT_SECURITY)
  StatefulService<SecuritySettings> *getSecuritySettingsService() {
    return &_securitySettingsService;
  }
#endif

  StatefulService<WiFiSettings> *getWiFiSettingsService() {
    return &_wifiSettingsService;
  }

  StatefulService<APSettings> *getAPSettingsService() {
    return &_apSettingsService;
  }

  NotificationService *getNotificationService() {
    return &_notificationService;
  }

#if FT_ENABLED(FT_NTP)
  StatefulService<NTPSettings> *getNTPSettingsService() {
    return &_ntpSettingsService;
  }
#endif

#if FT_ENABLED(FT_MQTT)
  StatefulService<MqttSettings> *getMqttSettingsService() {
    return &_mqttSettingsService;
  }

  PsychicMqttClient *getMqttClient() {
    return _mqttSettingsService.getMqttClient();
  }
#endif

#if FT_ENABLED(FT_BATTERY)
  BatteryService *getBatteryService() { return &_batteryService; }
#endif

  void setMDNSAppName(String name) { _appName = name; }

  void recoveryMode() { _apSettingsService.recoveryMode(); }

private:
  PsychicHttpServer *_server;
  unsigned int _numberEndpoints;
  SecuritySettingsService _securitySettingsService;
  WiFiSettingsService _wifiSettingsService;
  APSettingsService _apSettingsService;
  EventSocket _socket;
  NotificationService _notificationService;
#if FT_ENABLED(FT_NTP)
  NTPSettingsService _ntpSettingsService;
#endif
#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
  UploadFirmwareService _uploadFirmwareService;
#endif
#if FT_ENABLED(FT_DOWNLOAD_FIRMWARE)
  DownloadFirmwareService _downloadFirmwareService;
#endif
#if FT_ENABLED(FT_MQTT)
  MqttSettingsService _mqttSettingsService;
  MqttStatus _mqttStatus;
#endif
#if FT_ENABLED(FT_SECURITY)
  AuthenticationService _authenticationService;
#endif
#if FT_ENABLED(FT_BATTERY)
  BatteryService _batteryService;
#endif
#if FT_ENABLED(FT_ANALYTICS)
  AnalyticsService _analyticsService;
#endif
  PedoMeter _pedoMeter;

  String _appName = APP_NAME;

protected:
  static void _loopImpl(void *_this) {
    static_cast<ESP32SvelteKit *>(_this)->_loop();
  }
  void _loop();
};

#endif
