#pragma once

#include <vector>
#include <ArduinoJson.h>
#include <stateful_result.h>

struct SessionSlot {
    long start;
    long end;
    int steps;
    std::vector<float> times;

    void serialize(JsonObject &json) const {
        json["start"] = start;
        json["end"] = end;
        json["steps"] = steps;
        JsonArray timesArray = json["times"].to<JsonArray>();
        for (auto &time : times) {
            timesArray.add(time);
        }
    }
    bool deserialize(const JsonObject &json) {
        start = json["start"];
        end = json["end"];
        steps = json["steps"];

        if (json["times"].is<JsonArray>()) {
            times.clear();
            JsonArray timesArray = json["times"];
            for (JsonVariant time : timesArray) {
                times.push_back(time);
            }
        }
        return true;
    }
};

class PedoMeterData {
    std::vector<SessionSlot> sessions;
    float diameterOfHamsterWheel = 0.19;
    float numOfMagnets = 1;

  public:
    static void read(PedoMeterData &settings, JsonObject &root) {
        root["magnets"] = settings.numOfMagnets;
        root["diameter"] = settings.diameterOfHamsterWheel;
        JsonArray sessionsArray = root["sessions"].to<JsonArray>();

        for (auto &session : settings.sessions) {
            JsonObject newSession = sessionsArray.add<JsonObject>();
            session.serialize(newSession);
        }
    }

    static StateUpdateResult update(JsonObject &root, PedoMeterData &settings) {
        settings.numOfMagnets = root["magnets"] || settings.numOfMagnets;
        settings.diameterOfHamsterWheel = root["diameter"] || settings.diameterOfHamsterWheel;
        settings.sessions.clear();

        if (root["sessions"].is<JsonArray>()) {
            JsonArray sessionsArray = root["sessions"];

            for (auto jsonSession : sessionsArray) {
                JsonObject sessionJson = jsonSession.as<JsonObject>();

                SessionSlot newSession;
                if (newSession.deserialize(sessionJson)) {
                    settings.sessions.push_back(newSession);
                }
            }
        }

        return StateUpdateResult::CHANGED;
    }
    void updateSession(float timeElapsed) {
        SessionSlot &lastSession = sessions.back();
        lastSession.steps += 1;
        lastSession.times.push_back(timeElapsed);
    }

    void startSession() { sessions.push_back(SessionSlot {.start = time(nullptr)}); }

    void endSession() {
        if (sessions.empty()) return;
        SessionSlot &lastSession = sessions.back();
        lastSession.end = time(nullptr);
    }

    void reset() { sessions.clear(); }
};