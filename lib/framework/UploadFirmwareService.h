#ifndef UploadFirmwareService_h
#define UploadFirmwareService_h

#include <Arduino.h>

#include <Update.h>
#include <WiFi.h>

#include <PsychicHttp.h>
#include <system_service.h>

#define UPLOAD_FIRMWARE_PATH "/api/v1/uploadFirmware"

enum FileType { ft_none = 0, ft_firmware = 1, ft_md5 = 2 };

class UploadFirmwareService {
  public:
    UploadFirmwareService(PsychicHttpServer *server);

    void begin();

    PsychicUploadHandler *getHandler() { return &uploadHandler; }

  private:
    PsychicHttpServer *_server;
    PsychicUploadHandler uploadHandler;

    esp_err_t handleUpload(PsychicRequest *request, const String &filename, uint64_t index, uint8_t *data, size_t len,
                           bool final);
    esp_err_t uploadComplete(PsychicRequest *request);
    esp_err_t handleError(PsychicRequest *request, int code);
    esp_err_t handleEarlyDisconnect();
};

#endif // end UploadFirmwareService_h
