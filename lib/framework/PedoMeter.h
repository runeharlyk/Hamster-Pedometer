#pragma once
/*
Per hour
- Time spend in wheel
- Max speed
- Average speed
- Distance traveled


Top stats:
- Total time tracking
- Max overall speed
- Total distance
- Time spend in wheel
*/
#include <ArduinoJson.h>
#include <ESPFS.h>
#include <EventSocket.h>
#include <FSPersistence.h>
#include <WiFi.h>
#include <stateful_endpoint.h>
#include <timing.h>
#include <vector>
#include <domain/pedometer_data.h>

#define EVENT_STEP "step"
#define STEP_INTERVAL 150

#define HALL_SENSOR_PIN 32
#define DEBOUNCE_DELAY 150
#define SESSION_INACTIVITY_DELAY 10000

class PedoMeter : public StatefulService<PedoMeterData> {
  public:
    PedoMeter()
        : endpoint(PedoMeterData::read, PedoMeterData::update, this),
          _fsPersistence(PedoMeterData::read, PedoMeterData::update, this, STEPS_FILE) {};

    void begin();

    HttpEndpoint<PedoMeterData> endpoint;

  protected:
    FSPersistence<PedoMeterData> _fsPersistence;

    static void _loopImpl(void *_this) { static_cast<PedoMeter *>(_this)->_loop(); }
    void _loop();

    float totalDistance = 0.0;
    const float diameterOfHamsterWheel = 0.19; // cm
    const float pi = 3.14159;
    const float numOfMagnets = 1;
    const float circumference = pi * diameterOfHamsterWheel / numOfMagnets;
};
