
#include <MQTTEvent.h>
#include <led.h>

#include "PIR.h"
String MQTTEvent::ip_address;
PubSubClient *MQTTEvent::mqttClient;
WiFiClass *MQTTEvent::WiFi;

void MQTTEvent::callback(char *topic, byte *payload, unsigned int length) {
    Serial.println("MQTTEvent::Message arrived on topic: ");
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);
    JsonObject payload_json = doc.as<JsonObject>();

    if (payload_json["mac_address"] != MQTTEvent::WiFi->macAddress() && payload_json["mac_address"] != "all") return;

    Serial.println("MQTTEvent::topic is " + String(topic));
    if (String(topic) == "device/led") {
        MQTTEvent::onCallBackDeviceLED(payload_json, length);
    }
    if (String(topic) == "device/camera/take-image") {
        MQTTEvent::oncallBackTakeImage(payload_json, length);
    }
    if (String(topic) == "device/camera/stream") {
        MQTTEvent::onCallBackStream(payload_json, length);
    }
    if (String(topic) == "device/scan-connect") {
        MQTTEvent::onDeviceConnected();
    }
    if (String(topic) == "device/restart") {
        MQTTEvent::onCallBackDeviceRestart();
    }
    if (String(topic) == "device/config") {
        MQTTEvent::onCallBackSetConfig(payload_json, length);
    }
}

void MQTTEvent::setSubScribe() {
    MQTTEvent::mqttClient->subscribe("device/led");
    MQTTEvent::mqttClient->subscribe("device/camera/take-image");
    MQTTEvent::mqttClient->subscribe("device/camera/stream");
    MQTTEvent::mqttClient->subscribe("device/config");
    MQTTEvent::mqttClient->subscribe("device/scan-connect");
    MQTTEvent::mqttClient->subscribe("device/restart");
}
void MQTTEvent::onCallBackDeviceLED(JsonObject payload_json, unsigned int length) {
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["RSSI"] = MQTTEvent::WiFi->RSSI();
    doc["ip_address"] = MQTTEvent::ip_address;
    char payload[256];

    if (payload_json["task"]["led"] == "on") {
        doc["led"] = "on";
        LED::turnON(payload_json["task"]["pin"]);

        // MQTTEvent::ledState = true;
    } else if (payload_json["task"]["led"] == "off") {  // LED off
        // digitalWrite(LED_PIN, LOW);
        doc["led"] = "off";
        LED::turnOff(payload_json["task"]["pin"]);
        // MQTTEvent::ledState = false;
    }
    serializeJson(doc, payload);
    MQTTEvent::mqttClient->publish("device/led/status", payload);
}

void MQTTEvent::onDeviceConnected() {
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["online"] = "true";

    MQTTEvent::ip_address = MQTTEvent::WiFi->localIP().toString();
    doc["ip_address"] = MQTTEvent::ip_address;
    // doc["led_state"] = MQTTEvent::ledState;
    doc["RSSI"] = MQTTEvent::WiFi->RSSI();
    doc["PIR:Work"]=PIR::work;
    doc["LED:State"]=LED::ledState;
    // JsonArray data = doc.createNestedArray("data");
    // data.add(48.756080);
    // data.add(2.302038);
    char payload[256];
    serializeJson(doc, payload);

    Serial.println(payload);

    Serial.println("MQTTEvent:onDeviceConnected");
    MQTTEvent::mqttClient->publish("device/connected", payload);
}

void MQTTEvent::onCallBackSetConfig(JsonObject payload_json, unsigned int length) {
    if (payload_json["pir"]["work"] == "true") {
        PIR::work = true;
        Serial.println("MQTTEvent:onCallBackSetConfig pir::work=true");
    }

    if (payload_json["pir"]["work"] == "false") {
        PIR::work = false;
        Serial.println("MQTTEvent:onCallBackSetConfig pir::work=false");
    }

    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["mac_address"] = MQTTEvent::ip_address;
    doc["message"] = "onCallBackSetConfig";
    char payload[256];
    serializeJson(doc, payload);

    Serial.println("MQTTEvent:onCallBackSetConfig");
    MQTTEvent::mqttClient->publish("device/config", payload);
}

void MQTTEvent::oncallBackTakeImage(JsonObject payload_json, unsigned int length) {
    if (payload_json["task"]["take-image"] == "on" && payload_json["task"]["method"] == "http") {
        ESP32Camera::serverIP = payload_json["task"]["upload-server"].as<String>();
        ESP32Camera::takeImage();

    } else {
        Serial.println("MQTTEvent:oncallBackTakeImage takeImageMQtt ");
        ESP32Camera::takeImageMQtt();
    }
}

void MQTTEvent::onCallBackStream(JsonObject payload_json, unsigned int length) {
}

void MQTTEvent::onCallBackPIRDetected() {
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["ip_address"] = MQTTEvent::ip_address;
    // doc["message"] = "onCallBackPIRNotDetected";

    char payload[256];
    serializeJson(doc, payload);

    // Serial.println(payload);
    Serial.println("MQTTEvent:onCallBackPIRDetected");
    MQTTEvent::mqttClient->publish("device/detected", payload);
}

void MQTTEvent::onCallBackPIRNotDetected() {
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["ip_address"] = MQTTEvent::ip_address;
    // doc["message"] = "onCallBackPIRNotDetected";
    char payload[256];
    serializeJson(doc, payload);

    // Serial.println(payload);
    Serial.println("MQTTEvent:onCallBackPIRDetected");
    MQTTEvent::mqttClient->publish("device/not-detected", payload);
}

void MQTTEvent::onCallBackDeviceRestart() {
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["ip_address"] = MQTTEvent::ip_address;
    // doc["message"] = "onCallBackPIRNotDetected";
    char payload[256];
    serializeJson(doc, payload);

    Serial.println("MQTTEvent:device/restart");
    MQTTEvent::mqttClient->publish("device/restart", payload);
    delay(100);
    ESP.restart();
}
