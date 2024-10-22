#include <FeaturesService.h>

namespace feature_service {

void features(JsonObject &root) {
#if FT_ENABLED(FT_SECURITY)
    root["security"] = true;
#else
    root["security"] = false;
#endif
#if FT_ENABLED(FT_MQTT)
    root["mqtt"] = true;
#else
    root["mqtt"] = false;
#endif
#if FT_ENABLED(FT_NTP)
    root["ntp"] = true;
#else
    root["ntp"] = false;
#endif
#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
    root["upload_firmware"] = true;
#else
    root["upload_firmware"] = false;
#endif
#if FT_ENABLED(FT_DOWNLOAD_FIRMWARE)
    root["download_firmware"] = true;
#else
    root["download_firmware"] = false;
#endif
#if FT_ENABLED(FT_SLEEP)
    root["sleep"] = true;
#else
    root["sleep"] = false;
#endif
#if FT_ENABLED(FT_BATTERY)
    root["battery"] = true;
#else
    root["battery"] = false;
#endif
#if FT_ENABLED(FT_ANALYTICS)
    root["analytics"] = true;
#else
    root["analytics"] = false;
#endif

#if FT_ENABLED(EVENT_USE_JSON)
    root["event_use_json"] = true;
#else
    root["event_use_json"] = false;
#endif

    root["firmware_version"] = APP_VERSION;
    root["firmware_name"] = APP_NAME;
    root["firmware_built_target"] = BUILD_TARGET;
}

esp_err_t getFeatures(PsychicRequest *request) {
    PsychicJsonResponse response = PsychicJsonResponse(request, false);
    JsonObject root = response.getRoot();
    features(root);
    return response.send();
}

} // namespace feature_service
