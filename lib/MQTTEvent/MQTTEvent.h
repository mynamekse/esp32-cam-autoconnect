
#ifndef MQTTEvent_H
#define MQTTEvent_H
#include <ArduinoJson.h>
#include <ESP32Camera.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "esp_camera.h"
class MQTTEvent {
   public:
    static String mac_address;
    static PubSubClient *mqttClient;
    static StaticJsonDocument<256> configs;
    static String mqtt_server;
    static WiFiClass *WiFi;
    static bool enableLED;
    static bool enableCamera;
    static bool enablePIR;
    static bool ledState;
    
    static String ip_address;
    static void callback(char *topic, byte *payload, unsigned int length);
    static void onCallBackDeviceLED(JsonObject payload_json, unsigned int length);
    static void addSubscribe(std::function<void(int)> callback);
    static void onDeviceConnected();
    static void onCallBackSetConfig(JsonObject payload_json, unsigned int length);
    static void oncallBackTakeImage(JsonObject payload_json, unsigned int length);
    static void onCallBackStream(JsonObject payload_json, unsigned int length);
    static void onCallBackPIRDetected();
    static void onCallBackPIRNotDetected();
    static void onCallBackDeviceRestart();
    static void onCallBackDeviceControl(JsonObject payload_json, unsigned int length);
    static void setSubScribe();
};

#endif
