#pragma once

#include <Arduino.h>

#include <ArduinoJson.h>
#include <EventSocket.h>
#include <PsychicHttp.h>
#include <WiFi.h>

#include <HTTPClient.h>
#include <HTTPUpdate.h>
// #include <SSLCertBundle.h>

#define GITHUB_FIRMWARE_PATH "/api/v1/downloadUpdate"
#define EVENT_DOWNLOAD_OTA "otastatus"
#define OTA_TASK_STACK_SIZE 9216

class DownloadFirmwareService {
  public:
    DownloadFirmwareService(PsychicHttpServer *server, EventSocket *socket);

    void begin();

  private:
    PsychicHttpServer *_server;
    EventSocket *_socket;
    esp_err_t downloadUpdate(PsychicRequest *request, JsonVariant &json);
};
