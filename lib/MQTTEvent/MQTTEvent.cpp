
#include <MQTTEvent.h>
#include <led.h>
#include <AutoConnect.h>
#include "PIR.h"

String MQTTEvent::ip_address;
PubSubClient *MQTTEvent::mqttClient;
WiFiClass *MQTTEvent::WiFi;

void MQTTEvent::callback(char *topic, byte *payload, unsigned int length)
{
    Serial.println("MQTTEvent::Message arrived on topic: " + String(topic));
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);
    JsonObject payload_json = doc.as<JsonObject>();

    if (payload_json["mac_address"] != MQTTEvent::WiFi->macAddress() && payload_json["mac_address"] != "all")
        return;

    Serial.println("MQTTEvent::topic is " + String(topic));
    if (String(topic) == "device/led")
    {
        MQTTEvent::onCallBackDeviceLED(payload_json, length);
    }
    if (String(topic) == "device/camera/take-image")
    {
        MQTTEvent::oncallBackTakeImage(payload_json, length);
    }
    if (String(topic) == "device/camera/stream")
    {
        MQTTEvent::onCallBackStream(payload_json, length);
    }
    if (String(topic) == "device/scan-connect")
    {
        MQTTEvent::onDeviceConnected();
    }
    if (String(topic) == "device/restart")
    {
        MQTTEvent::onCallBackDeviceRestart();
    }
    if (String(topic) == "device/pir/config")
    {
        MQTTEvent::onCallBackSetConfig(payload_json, length);
    }

    if (String(topic) == "device/camera/config")
    {
        MQTTEvent::onCallBackDeviceControl(payload_json, length);
    }
}
void MQTTEvent::onCallBackDeviceControl(JsonObject payload_json, unsigned int length)
{
    // int timoutTimer = payload_json["timoutTimer"].as<int>();
    // if (timoutTimer >= 0)
    // {
    //     ESP32Camera::timoutTimer = timoutTimer;
    //     Serial.println("MQTTEvent:onCallBackDeviceControl");
    //     return;
    // }

    sensor_t *s = esp_camera_sensor_get();
    const char *variable = payload_json["sensor"]["var"].as<char *>();
    const int val = payload_json["sensor"]["val"].as<int>();
    int res = 0;

    Serial.println("onCallBackDeviceControl:val" + val);
    if (!strcmp(variable, "framesize"))
    {
        Serial.println(variable);
        Serial.println("onCallBackDeviceControl:val" + val);
        if (s->pixformat == PIXFORMAT_JPEG)

            res = s->set_framesize(s, (framesize_t)val);
    }
    else if (!strcmp(variable, "quality"))
        res = s->set_quality(s, val);
    else if (!strcmp(variable, "contrast"))
        res = s->set_contrast(s, val);
    else if (!strcmp(variable, "brightness"))
        res = s->set_brightness(s, val);
    else if (!strcmp(variable, "saturation"))
        res = s->set_saturation(s, val);
    else if (!strcmp(variable, "gainceiling"))
        res = s->set_gainceiling(s, (gainceiling_t)val);
    else if (!strcmp(variable, "colorbar"))
        res = s->set_colorbar(s, val);
    else if (!strcmp(variable, "awb"))
        res = s->set_whitebal(s, val);
    else if (!strcmp(variable, "agc"))
        res = s->set_gain_ctrl(s, val);
    else if (!strcmp(variable, "aec"))
        res = s->set_exposure_ctrl(s, val);
    else if (!strcmp(variable, "hmirror"))
        res = s->set_hmirror(s, val);
    else if (!strcmp(variable, "vflip"))
        res = s->set_vflip(s, val);
    else if (!strcmp(variable, "awb_gain"))
        res = s->set_awb_gain(s, val);
    else if (!strcmp(variable, "agc_gain"))
        res = s->set_agc_gain(s, val);
    else if (!strcmp(variable, "aec_value"))
        res = s->set_aec_value(s, val);
    else if (!strcmp(variable, "aec2"))
        res = s->set_aec2(s, val);
    else if (!strcmp(variable, "dcw"))
        res = s->set_dcw(s, val);
    else if (!strcmp(variable, "bpc"))
        res = s->set_bpc(s, val);
    else if (!strcmp(variable, "wpc"))
        res = s->set_wpc(s, val);
    else if (!strcmp(variable, "raw_gma"))
        res = s->set_raw_gma(s, val);
    else if (!strcmp(variable, "lenc"))
        res = s->set_lenc(s, val);
    else if (!strcmp(variable, "special_effect"))
        res = s->set_special_effect(s, val);
    else if (!strcmp(variable, "wb_mode"))
        res = s->set_wb_mode(s, val);
    else if (!strcmp(variable, "ae_level"))
        res = s->set_ae_level(s, val);
}

void MQTTEvent::setSubScribe()
{
    MQTTEvent::mqttClient->subscribe("device/camera/config");
    MQTTEvent::mqttClient->subscribe("device/led");
    MQTTEvent::mqttClient->subscribe("device/camera/take-image");
    MQTTEvent::mqttClient->subscribe("device/camera/take-image-mqtt");
    MQTTEvent::mqttClient->subscribe("device/camera/stream");
    MQTTEvent::mqttClient->subscribe("device/pir/config");
    MQTTEvent::mqttClient->subscribe("device/scan-connect");
    MQTTEvent::mqttClient->subscribe("device/restart");
    MQTTEvent::mqttClient->subscribe("device/control");
}
void MQTTEvent::onCallBackDeviceLED(JsonObject payload_json, unsigned int length)
{
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["RSSI"] = MQTTEvent::WiFi->RSSI();
    doc["ip_address"] = MQTTEvent::ip_address;
    char payload[256];

    if (payload_json["task"]["led"] == "on")
    {
        doc["led"] = "on";
        LED::turnON(payload_json["task"]["pin"]);

        // MQTTEvent::ledState = true;
    }
    else if (payload_json["task"]["led"] == "off")
    { // LED off
        // digitalWrite(LED_PIN, LOW);
        doc["led"] = "off";
        LED::turnOff(payload_json["task"]["pin"]);
        // MQTTEvent::ledState = false;
    }
    serializeJson(doc, payload);
    MQTTEvent::mqttClient->publish("device/led/status", payload);
}

void MQTTEvent::onDeviceConnected()
{
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["online"] = "true";

    MQTTEvent::ip_address = MQTTEvent::WiFi->localIP().toString();
    doc["ip_address"] = MQTTEvent::ip_address;
    // doc["led_state"] = MQTTEvent::ledState;
    doc["RSSI"] = MQTTEvent::WiFi->RSSI();
    doc["PIR_Work"] = PIR::work;
    doc["LED_State"] = LED::ledState;
    // JsonArray data = doc.createNestedArray("data");
    // data.add(48.756080);
    // data.add(2.302038);
    char payload[256];
    serializeJson(doc, payload);

    Serial.println(payload);

    Serial.println("MQTTEvent:onDeviceConnected");
    MQTTEvent::mqttClient->publish("device/connected", payload);
}

void MQTTEvent::onCallBackSetConfig(JsonObject payload_json, unsigned int length)
{
    if (payload_json["pir"]["work"] == "true")
    {
        PIR::work = true;
        Serial.println("MQTTEvent:onCallBackSetConfig pir::work=true");
    }

    if (payload_json["pir"]["work"] == "false")
    {
        PIR::work = false;
        Serial.println("MQTTEvent:onCallBackSetConfig pir::work=false");
    }

    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["ip_address"] = MQTTEvent::ip_address;
    doc["message"] = "onCallBackSetConfig";
    char payload[256];
    serializeJson(doc, payload);

    Serial.println("MQTTEvent:onCallBackSetConfig");
    MQTTEvent::mqttClient->publish("device/config/complete", payload);
}

void MQTTEvent::oncallBackTakeImage(JsonObject payload_json, unsigned int length)
{
    if (payload_json["task"]["take-image"] == "on" && payload_json["task"]["method"] == "http")
    {
        ESP32Camera::serverIP = payload_json["task"]["upload-server"].as<String>();
        ESP32Camera::serverPath = payload_json["task"]["upload-path"].as<String>();
        ESP32Camera::serverPort = payload_json["task"]["upload-port"].as<int>();
        ESP32Camera::takeImage();
    }
    else
    {
        Serial.println("MQTTEvent:oncallBackTakeImage takeImageMQtt ");
    }
}

void MQTTEvent::onCallBackStream(JsonObject payload_json, unsigned int length)
{
}

void MQTTEvent::onCallBackPIRDetected()
{
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

void MQTTEvent::onCallBackPIRNotDetected()
{
    StaticJsonDocument<256> doc;
    doc["mac_address"] = MQTTEvent::WiFi->macAddress();
    doc["ip_address"] = MQTTEvent::ip_address;
    // doc["message"] = "onCallBackPIRNotDetected";
    char payload[256];
    serializeJson(doc, payload);

    // Serial.println(payload);
    Serial.println("MQTTEvent:onCallBack Not-detected");
    MQTTEvent::mqttClient->publish("device/not-detected", payload);
}

void MQTTEvent::onCallBackDeviceRestart()
{
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
