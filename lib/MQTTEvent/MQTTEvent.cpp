
#include <MQTTEvent.h>
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
}
void MQTTEvent::onCallPIRDetected() {
}

void MQTTEvent::setSubScribe() {
    MQTTEvent::mqttClient->subscribe("device/led");
    MQTTEvent::mqttClient->subscribe("device/camera/take-image");
    MQTTEvent::mqttClient->subscribe("device/camera/stream");
    MQTTEvent::mqttClient->subscribe("device/config");
    MQTTEvent::mqttClient->subscribe("device/scan-connect");
}
void MQTTEvent::onCallBackDeviceLED(JsonObject payload_json, unsigned int length) {
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["RSSI"] = MQTTEvent::WiFi->RSSI();
    char payload[256];
    serializeJson(doc, payload);
    if (payload_json["task"]["led"] == "on") {
        // digitalWrite(LED_PIN, HIGH);
        doc["led"] = "on";
        // MQTTEvent::ledState = true;
    } else if (payload_json["task"]["led"] == "off") {  // LED off
        // digitalWrite(LED_PIN, LOW);
        doc["led"] = "off";
        // MQTTEvent::ledState = false;
    }

    MQTTEvent::mqttClient->publish("device/led/status", payload);
}

void MQTTEvent::onDeviceConnected() {
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["online"] = "true";
    doc["ip_address"] = MQTTEvent::WiFi->localIP().toString();
    // doc["led_state"] = MQTTEvent::ledState;
    doc["RSSI"] = MQTTEvent::WiFi->RSSI();

    // JsonArray data = doc.createNestedArray("data");
    // data.add(48.756080);
    // data.add(2.302038);
    char payload[256];
    serializeJson(doc, payload);

    Serial.println(payload);

    Serial.println("MQTTEvent:onDeviceConnected");
    MQTTEvent::mqttClient->publish("device/connected", payload);
}

void MQTTEvent::oncallBackTakeImage(JsonObject payload_json, unsigned int length) {
}

void MQTTEvent::onCallBackStream(JsonObject payload_json, unsigned int length) {
}
