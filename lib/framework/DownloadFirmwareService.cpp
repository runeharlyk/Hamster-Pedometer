/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32
 *platforms with responsive Sveltekit front-end built with TailwindCSS and
 *DaisyUI. https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <DownloadFirmwareService.h>

static const char *TAG = "Download OAT";

extern const uint8_t rootca_crt_bundle_start[] asm("_binary_src_certs_x509_crt_bundle_bin_start");

static EventSocket *_socket = nullptr;
static int previousProgress = 0;
JsonDocument doc;

void update_started() {
    String output;
    doc["status"] = "preparing";
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);
}

void update_progress(int currentBytes, int totalBytes) {
    doc["status"] = "progress";
    int progress = ((currentBytes * 100) / totalBytes);
    if (progress > previousProgress) {
        doc["progress"] = progress;
        JsonObject jsonObject = doc.as<JsonObject>();
        _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);
        ESP_LOGV(TAG, "HTTP update process at %d of %d bytes... (%d %%)", currentBytes, totalBytes, progress);
    }
    previousProgress = progress;
}

void update_finished() {
    doc["status"] = "finished";
    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);

    // delay to allow the event to be sent out
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void updateTask(void *param) {
    WiFiClientSecure client;
    client.setCACertBundle(rootca_crt_bundle_start);
    client.setTimeout(10);

    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpUpdate.rebootOnUpdate(true);

    String url = *((String *)param);
    String output;
    // httpUpdate.onStart(update_started);
    // httpUpdate.onProgress(update_progress);
    // httpUpdate.onEnd(update_finished);

    t_httpUpdate_return ret = httpUpdate.update(client, url.c_str());
    JsonObject jsonObject;

    switch (ret) {
        case HTTP_UPDATE_FAILED:

            doc["status"] = "error";
            doc["error"] = httpUpdate.getLastErrorString().c_str();
            jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);

            ESP_LOGE(TAG, "HTTP Update failed with error (%d): %s", httpUpdate.getLastError(),
                     httpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:

            doc["status"] = "error";
            doc["error"] = "Update failed, has same firmware version";
            jsonObject = doc.as<JsonObject>();
            _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);

            ESP_LOGE(TAG, "HTTP Update failed, has same firmware version");
            break;
        case HTTP_UPDATE_OK: ESP_LOGI(TAG, "HTTP Update successful - Restarting"); break;
    }
    vTaskDelete(NULL);
}

DownloadFirmwareService::DownloadFirmwareService(PsychicHttpServer *server, EventSocket *socket)
    : _server(server), _socket(socket) {}

void DownloadFirmwareService::begin() {
    _server->on(
        GITHUB_FIRMWARE_PATH, HTTP_POST,
        std::bind(&DownloadFirmwareService::downloadUpdate, this, std::placeholders::_1, std::placeholders::_2));

    ESP_LOGV(TAG, "Registered POST endpoint: %s", GITHUB_FIRMWARE_PATH);
}

esp_err_t DownloadFirmwareService::downloadUpdate(PsychicRequest *request, JsonVariant &json) {
    if (!json.is<JsonObject>()) {
        return request->reply(400);
    }

    String downloadURL = json["download_url"];
    ESP_LOGI(TAG, "Starting OTA from: %s", downloadURL.c_str());

    doc["status"] = "preparing";
    doc["progress"] = 0;
    doc["error"] = "";

    JsonObject jsonObject = doc.as<JsonObject>();
    _socket->emitEvent(EVENT_DOWNLOAD_OTA, jsonObject);

    if (xTaskCreatePinnedToCore(&updateTask,                // Function that should be called
                                "Update",                   // Name of the task (for debugging)
                                OTA_TASK_STACK_SIZE,        // Stack size (bytes)
                                &downloadURL,               // Pass reference to this class instance
                                (configMAX_PRIORITIES - 1), // Pretty high task priority
                                NULL,                       // Task handle
                                1                           // Have it on application core
                                ) != pdPASS) {
        ESP_LOGE(TAG, "Couldn't create download OTA task");
        return request->reply(500);
    }
    return request->reply(200);
}
