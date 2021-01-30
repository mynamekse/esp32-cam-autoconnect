
#include "TCPServer.h"

#include <Arduino.h>
// #include <MQTTEvent.h>
// #include <Timer.h>
// #include <EEPROM.h>
// #include <EEPROMAdd.h>
#include <Timer.h>
#define MQTT_SERVER_03 "192.168.0.3"
#define MQTT_SERVER_05 "192.168.0.5"
WiFiServer TCPserver::server(80);
String converter(uint8_t *str) {
    return String((char *)str);
}
void TCPserver::loop() {
    /* listen for client */
    WiFiClient client = TCPserver::server.available();
    uint8_t data[50];
    if (client) {
        Serial.println("new client");
        /* check client is connected */
        while (client.connected()) {
            if (client.available()) {
                int len = client.read(data, 100);
                // StaticJsonBuffer<500> JSONBuffer;
                if (len < 50) {
                    data[len] = '\0';
                } else {
                    data[49] = '\0';
                }
                Serial.print("cliend send :");
                Serial.println(converter(data));
                StaticJsonDocument<256> doc;
                const char *cover_data = converter(data).c_str();

                deserializeJson(doc, cover_data, len);
                //  JsonObject payload_json = doc.as<JsonObject>();
                //  const char* wx_description = payload_json["config"];

                Timer::timeLoop(millis(), 10);
                TCPserver::onHandleMessage(doc);
                client.stop();
                Serial.println("client disonnected");
            }
        }
    }
}

void TCPserver::setup() {
    server.begin();
    Serial.println("TCPserver::setup");
}

void TCPserver::onHandleMessage(StaticJsonDocument<256> message) {
    const char *mqtt_server = message["config"]["mqtt_server"];
    uint8_t mqtt_server_uint8 = *mqtt_server;
    // const char* mqtt_port = message["config"]["mqtt_port"];
    Serial.print("TCPServer::onHandleMessage write ");
    Serial.print(mqtt_server);
    // uint16_t mqtt_port_u =(uint16_t) mqtt_port;
    // str_to_uint16(mqtt_server,mqtt_server_uint16);
    // if (strcmp(mqtt_server, "192.168.0.3")) {
    //     MQTTEvent::client->setServer(MQTT_SERVER_03, 1883);
    //     Serial.println(mqtt_server);
    // } else if (strcmp(mqtt_server, "192.168.0.5")) {
    //     MQTTEvent::client->setServer(MQTT_SERVER_05, 1883);
    //     Serial.println(mqtt_server);
    // }

    // writeString(10,mqtt_server);
    // ESP.restart();

    // Timer::timeLoop(millis(), 1500);
    //MQTTEvent::reconnect();
}
