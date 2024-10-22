#include <ESP32SvelteKit.h>
#include <PsychicHttpServer.h>

#define SERIAL_BAUD_RATE 115200

DRAM_ATTR PsychicHttpServer server;

DRAM_ATTR ESP32SvelteKit esp32sveltekit(&server);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    esp32sveltekit.begin();
}

void loop() { vTaskDelete(nullptr); }
