#ifndef Features_h
#define Features_h

#define FT_ENABLED(feature) feature

// mqtt feature on by default
#ifndef FT_MQTT
#define FT_MQTT 1
#endif

// ntp feature on by default
#ifndef FT_NTP
#define FT_NTP 1
#endif

// upload firmware feature off by default
#ifndef FT_UPLOAD_FIRMWARE
#define FT_UPLOAD_FIRMWARE 0
#endif

// download firmware feature off by default
#ifndef FT_DOWNLOAD_FIRMWARE
#define FT_DOWNLOAD_FIRMWARE 0
#endif

// ESP32 sleep states off by default
#ifndef FT_SLEEP
#define FT_SLEEP 0
#endif

// ESP32 analytics on by default
#ifndef FT_ANALYTICS
#define FT_ANALYTICS 1
#endif

// Use JSON for events. Default, use MessagePack for events
#ifndef EVENT_USE_JSON
#define EVENT_USE_JSON 0
#endif

#endif
