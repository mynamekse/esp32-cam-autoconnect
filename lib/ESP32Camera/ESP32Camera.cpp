#include "ESP32Camera.h"

#include <WiFi.h>
#include <base64.h>

#include "Arduino.h"
#include "PIR.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "img_converters.h"
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include "soc/soc.h"          //disable brownout problems
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define PART_BOUNDARY "123456789000000000000987654321"

#include <MQTTEvent.h>
#include <WiFi.h>

String ESP32Camera::serverIP;
String ESP32Camera::serverPath;
int ESP32Camera::serverPort ;
WiFiClient ESP32Camera::wifiClient;

String ESP32Camera::takeImage()
{
    String getAll;
    String getBody;
    PIR::enable = false;
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        delay(1000);
        ESP.restart();
    }
    Serial.println("ESP32Camera:before takeImage");
    if (ESP32Camera::wifiClient.connect(serverIP.c_str(), ESP32Camera::serverPort))
    {
        String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String tail = "\r\n--RandomNerdTutorials--\r\n";

        uint16_t imageLen = fb->len;
        uint16_t extraLen = head.length() + tail.length();
        uint16_t totalLen = imageLen + extraLen;

        ESP32Camera::wifiClient.println("POST " + ESP32Camera::serverPath + " HTTP/1.1");
        ESP32Camera::wifiClient.println("Host: " + ESP32Camera::serverIP);
        ESP32Camera::wifiClient.println("Content-Length: " + String(totalLen));
        ESP32Camera::wifiClient.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
        ESP32Camera::wifiClient.println();
        ESP32Camera::wifiClient.print(head);

        uint8_t *fbBuf = fb->buf;
        size_t fbLen = fb->len;
        for (size_t n = 0; n < fbLen; n = n + 1024)
        {
            if (n + 1024 < fbLen)
            {
                ESP32Camera::wifiClient.write(fbBuf, 1024);
                fbBuf += 1024;
            }
            else if (fbLen % 1024 > 0)
            {
                size_t remainder = fbLen % 1024;
                ESP32Camera::wifiClient.write(fbBuf, remainder);
            }
        }
        ESP32Camera::wifiClient.print(tail);

        esp_camera_fb_return(fb);

        // int timoutTimer = 10000;
        // long startTimer = millis();
        // boolean state = false;

        // while ((startTimer + timoutTimer) > millis()) {
        //     Serial.print(".");
        //     delay(100);
        //     while (ESP32Camera::wifiClient.available()) {
        //         char c = ESP32Camera::wifiClient.read();
        //         if (c == '\n') {
        //             if (getAll.length() == 0) {
        //                 state = true;
        //             }
        //             getAll = "";
        //         } else if (c != '\r') {
        //             getAll += String(c);
        //         }
        //         if (state == true) {
        //             getBody += String(c);
        //         }
        //         startTimer = millis();
        //     }
        //     if (getBody.length() > 0) {
        //         break;
        //     }
        // }
        Serial.println();
        ESP32Camera::wifiClient.stop();

        PIR::enable = true;
        StaticJsonDocument<256> doc;
        doc["mac_address"] = MQTTEvent::WiFi->macAddress();
        doc["ip_address"] = MQTTEvent::ip_address;
        // doc["message"] = "onCallBackPIRNotDetected";
        char payload[256];
        serializeJson(doc, payload);

        Serial.println("ESP32Camera takeImage:Connection successful!");
        MQTTEvent::mqttClient->publish("device/upload_image_to", payload);
        // Serial.println(getBody);โ
    }
    else
    {
        getBody = "Connection to " + serverIP + " failed.";
        Serial.println(getBody);
    }
    return getBody;
}

void ESP32Camera::takeImageMQtt()
{
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        return;
    }

    const char *pic_buf = (const char *)(fb->buf);

    // if (MQTT_MAX_PACKET_SIZE == 128) {
    //     //SLOW MODE (increase MQTT_MAX_PACKET_SIZE)
    //     if (MQTTEvent::mqttClient->publish_P("device/image", fb->buf, fb->len, false)) {
    //         Serial.println("publish_P succcess ");
    //     }

    // } else {
    //     //FAST MODE (increase MQTT_MAX_PACKET_SIZE)
    //     if (MQTTEvent::mqttClient->publish("device/image", fb->buf, fb->len, false)) {
    //         Serial.println("publish succcess ");
    //     }
    // }
    // MQTTEvent::mqttClient->publish("device/image", pic_buf, fb->len);
    Serial.println("publish device/image ");
     // String imgDataB64 = base64::encode(fb->buf, fb->len);
    uint16_t packetIdPubTemp = MQTTEvent::mqttClient->publish("device/image",  fb->buf, fb->len, false);

    if (!packetIdPubTemp)
    {
        Serial.println("Sending Failed! err: " + String(packetIdPubTemp));
    }
    else
    {
        Serial.println("MQTT Publish succesful");
    }

    Serial.println("CLIC");
    esp_camera_fb_return(fb);
}

String ESP32Camera::_STREAM_CONTENT_TYPE;
String ESP32Camera::_STREAM_BOUNDARY;
String ESP32Camera::_STREAM_PART;

bool ESP32Camera::initCamera()
{
    camera_config_t config;
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // ESP32Camera::_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
    // ESP32Camera::_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
    // ESP32Camera::_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
    if (psramFound())
    {
        config.frame_size = FRAMESIZE_XGA;  //8-13  VGA SVGA(9 800x600)   XGA(1080)
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_XGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t result = esp_camera_init(&config);

    if (result != ESP_OK)
    {
        return false;
    }
    sensor_t *s = esp_camera_sensor_get();
    s->set_brightness(s, 2);                 // -2 to 2
    s->set_contrast(s, 2);                   // -2 to 2
    s->set_saturation(s, 2);                 // -2 to 2
    s->set_special_effect(s, 0);             // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);                   // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);                   // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);                    // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 1);              // 0 = disable , 1 = enable
    s->set_aec2(s, 0);                       // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);                   // -2 to 2
    s->set_aec_value(s, 300);                // 0 to 1200
    s->set_gain_ctrl(s, 1);                  // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);                   // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0); // 0 to 6
    s->set_bpc(s, 0);                        // 0 = disable , 1 = enable
    s->set_wpc(s, 1);                        // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);                    // 0 = disable , 1 = enable
    s->set_lenc(s, 1);                       // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);                    // 0 = disable , 1 = enable
    s->set_vflip(s, 0);
    // 0 = disable , 1 = enable
    //s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_dcw(s, 1);      // 0 = disable , 1 = enable
    s->set_colorbar(s, 0); // 0 = disable , 1 = enable
    if (s->id.PID == OV3660_PID)
    {
       
        s->set_vflip(s, 1);       //flip it back
        s->set_brightness(s, 1);  //up the blightness just a bit
        s->set_saturation(s, -2); //lower the saturation
    }
    return true;
}
