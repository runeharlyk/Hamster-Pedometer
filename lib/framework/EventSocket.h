#ifndef Socket_h
#define Socket_h

#include <Features.h>
#include <PsychicHttp.h>
#include <StatefulService.h>
#include <list>
#include <map>
#include <vector>

#define EVENT_SERVICE_PATH "/ws/events"

typedef std::function<void(JsonObject &root, int originId)> EventCallback;
typedef std::function<void(const String &originId)> SubscribeCallback;

class EventSocket {
  public:
    EventSocket(PsychicHttpServer *server);

    PsychicWebSocketHandler *getHandler() { return &_socket; }

    void begin();

    void registerEvent(String event);

    void onEvent(String event, EventCallback callback);

    void onSubscribe(String event, SubscribeCallback callback);

    void emitEvent(String event, JsonObject &jsonObject, const char *originId = "", bool onlyToSameOrigin = false);
    // if onlyToSameOrigin == true, the message will be sent to the originId only,
    // otherwise it will be broadcasted to all clients except the originId

  private:
    PsychicHttpServer *_server;
    PsychicWebSocketHandler _socket;

    std::vector<String> events;
    std::map<String, std::list<int>> client_subscriptions;
    std::map<String, std::list<EventCallback>> event_callbacks;
    std::map<String, std::list<SubscribeCallback>> subscribe_callbacks;
    void handleEventCallbacks(String event, JsonObject &jsonObject, int originId);
    void handleSubscribeCallbacks(String event, const String &originId);

    bool isEventValid(String event);

    void onWSOpen(PsychicWebSocketClient *client);
    void onWSClose(PsychicWebSocketClient *client);
    esp_err_t onFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame);
};

#endif
