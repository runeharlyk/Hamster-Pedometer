#include <ESP32SvelteKit.h>

static const char *TAG = "ESP32SvelteKit";

ESP32SvelteKit::ESP32SvelteKit(PsychicHttpServer *server)
    : _server(server), _wifiSettingsService(server, &ESPFS, &_socket),
      _apSettingsService(server, &ESPFS), _socket(server),
#if FT_ENABLED(FT_NTP)
      _ntpSettingsService(server, &ESPFS),
#endif
#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
      _uploadFirmwareService(server),
#endif
#if FT_ENABLED(FT_DOWNLOAD_FIRMWARE)
      _downloadFirmwareService(server, &_socket),
#endif
#if FT_ENABLED(FT_MQTT)
      _mqttSettingsService(server, &ESPFS, &_securitySettingsService),
      _mqttStatus(server, &_mqttSettingsService, &_securitySettingsService),
#endif
#if FT_ENABLED(FT_ANALYTICS)
      _analyticsService(&_socket),
#endif
      _pedoMeter(server, &ESPFS, &_socket) {
}

void ESP32SvelteKit::begin() {
  ESP_LOGV("ESP32SvelteKit", "Loading settings from files system");
  ESPFS.begin(true);

  _wifiSettingsService.initWiFi();

  ESP_LOGI(TAG, "Running Firmware Version: %s\n", APP_VERSION);

  setupServer();
  setupMDNS();
  startServices();
}

void ESP32SvelteKit::setupServer() {
  _server->config.max_uri_handlers = _numberEndpoints;
  _server->maxUploadSize = _maxFileUpload;
  _server->listen(_port);

  // Serve static resources from PROGMEM
  ESP_LOGV(TAG, "Registering routes from PROGMEM static resources");
  WWWData::registerRoutes([&](const String &uri, const String &contentType,
                              const uint8_t *content, size_t len) {
    PsychicHttpRequestCallback requestHandler = [contentType, content,
                                                 len](PsychicRequest *request) {
      PsychicResponse response(request);
      response.setCode(200);
      response.setContentType(contentType.c_str());
      response.addHeader("Content-Encoding", "gzip");
      response.addHeader("Cache-Control",
                         "public, immutable, max-age=31536000");
      response.setContent(content, len);
      return response.send();
    };
    PsychicWebHandler *handler = new PsychicWebHandler();
    handler->onRequest(requestHandler);
    _server->on(uri.c_str(), HTTP_GET, handler);

    // Set default end-point for all non matching requests
    // this is easier than using webServer.onNotFound()
    if (uri.equals("/index.html")) {
      _server->defaultEndpoint->setHandler(handler);
    }
  });

  // SYSTEM
  _server->on("/api/v1/system/reset", HTTP_POST, system_service::handleReset);
  _server->on("/api/v1/system/restart", HTTP_POST,
              system_service::handleRestart);
  _server->on("/api/v1/system/sleep", HTTP_POST, system_service::handleSleep);
  _server->on("/api/v1/system/status", HTTP_GET, system_service::getStatus);
  _server->on("/api/v1/system/metrics", HTTP_GET, system_service::getMetrics);

  // WIFI
  _server->on("/api/v1/wifi/scan", HTTP_POST, wifi_sta::handleScan);
  _server->on("/api/v1/wifi/networks", HTTP_GET, wifi_sta::getNetworks);
  _server->on("/api/v1/wifi/sta/status", HTTP_GET, wifi_sta::getNetworkStatus);

  // AP
  _server->on("/api/v1/wifi/ap/status", HTTP_GET, [this](PsychicRequest *r) {
    return _apSettingsService.getStatus(r);
  });

// NTP
#if FT_ENABLED(FT_NTP)
  _server->on("/api/v1/ntp/status", HTTP_GET, [this](PsychicRequest *r) {
    return _ntpSettingsService.getStatus(r);
  });
  _server->on("/api/v1/ntp/time", HTTP_POST,
              [this](PsychicRequest *r, JsonVariant &json) {
                return _ntpSettingsService.handleTime(r, json);
              });
#endif

  // MISC
  _server->on("/api/v1/features", HTTP_GET, feature_service::getFeatures);
  _server->on("/ws/events", _socket.getHandler());

  // STATIC CONFIG
#if SERVE_CONFIG_FILES
  _server->serveStatic("/api/config/", ESPFS, "/config/");
#endif

#if defined(ENABLE_CORS)
  ESP_LOGV(TAG, "Enabling CORS headers");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin",
                                       CORS_ORIGIN);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
                                       "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials",
                                       "true");
#endif
}

void ESP32SvelteKit::setupMDNS() {
  ESP_LOGV("ESP32SvelteKit", "Starting MDNS");
  MDNS.begin(_wifiSettingsService.getHostname().c_str());
  MDNS.setInstanceName(_appName);
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 80);
  MDNS.addServiceTxt("http", "tcp", "Firmware Version", APP_VERSION);
}

void ESP32SvelteKit::startServices() {
  _socket.begin();
  _apSettingsService.begin();
  _wifiSettingsService.begin();

#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
  _uploadFirmwareService.begin();
#endif
#if FT_ENABLED(FT_DOWNLOAD_FIRMWARE)
  _downloadFirmwareService.begin();
#endif
#if FT_ENABLED(FT_NTP)
  _ntpSettingsService.begin();
#endif
#if FT_ENABLED(FT_MQTT)
  _mqttSettingsService.begin();
  _mqttStatus.begin();
#endif
#if FT_ENABLED(FT_ANALYTICS)
  _analyticsService.begin();
#endif
  _pedoMeter.begin();

  // Start the loop task
  ESP_LOGV(TAG, "Starting loop task");
  xTaskCreatePinnedToCore(
      this->_loopImpl,        // Function that should be called
      "ESP32 SvelteKit Loop", // Name of the task (for debugging)
      4096,                   // Stack size (bytes)
      this,                   // Pass reference to this class instance
      (tskIDLE_PRIORITY + 1), // task priority
      NULL,                   // Task handle
      0                       // Pin to application core
  );
}

void ESP32SvelteKit::_loop() {
  while (1) {
    _wifiSettingsService.loop(); // 30 seconds
    _apSettingsService.loop();   // 10 seconds
#if FT_ENABLED(FT_MQTT)
    _mqttSettingsService.loop(); // 5 seconds
#endif
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
