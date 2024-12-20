#pragma once

#include <Arduino.h>

#include <ArduinoJson.h>
#include <EventSocket.h>
#include <PsychicHttp.h>
#include <WiFi.h>

#include <HTTPClient.h>
#include <HTTPUpdate.h>
// #include <SSLCertBundle.h>

#define EVENT_DOWNLOAD_OTA "otastatus"
#define OTA_TASK_STACK_SIZE 9216

class DownloadFirmwareService {
  public:
    DownloadFirmwareService();
    esp_err_t handleDownloadUpdate(PsychicRequest *request, JsonVariant &json);
};
