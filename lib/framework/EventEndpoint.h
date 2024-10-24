#ifndef EventEndpoint_h
#define EventEndpoint_h

#include <EventSocket.h>
#include <PsychicHttp.h>
#include <StatefulService.h>

template <class T>
class EventEndpoint {
  public:
    EventEndpoint(JsonStateReader<T> stateReader, JsonStateUpdater<T> stateUpdater, StatefulService<T> *statefulService,
                  const char *event)
        : _stateReader(stateReader), _stateUpdater(stateUpdater), _statefulService(statefulService), _event(event) {
        _statefulService->addUpdateHandler([&](const String &originId) { syncState(originId); }, false);
    }

    void begin() {
        socket.onEvent(_event,
                       std::bind(&EventEndpoint::updateState, this, std::placeholders::_1, std::placeholders::_2));
        socket.onSubscribe(_event, [&](const String &originId) { syncState(originId, true); });
    }

  private:
    JsonStateReader<T> _stateReader;
    JsonStateUpdater<T> _stateUpdater;
    StatefulService<T> *_statefulService;
    const char *_event;

    void updateState(JsonObject &root, int originId) {
        _statefulService->update(root, _stateUpdater, String(originId));
    }

    void syncState(const String &originId, bool sync = false) {
        JsonDocument jsonDocument;
        JsonObject root = jsonDocument.to<JsonObject>();
        _statefulService->read(root, _stateReader);
        JsonObject jsonObject = jsonDocument.as<JsonObject>();
        socket.emitEvent(_event, jsonObject, originId.c_str(), sync);
    }
};

#endif
