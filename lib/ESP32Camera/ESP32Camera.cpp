#include "ESP32Camera.h"

#include <WiFi.h>

#include "Arduino.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "img_converters.h"
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "soc/soc.h"           //disable brownout problems

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

#include <WiFi.h>
String ESP32Camera::serverIP;
String ESP32Camera::serverPath = "/upload";
const int serverPort = 1880;
WiFiClient ESP32Camera::wifiClient;

String ESP32Camera::takeImage() {
    String getAll;
    String getBody;

    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        delay(1000);
        ESP.restart();
    }
    Serial.println("ESP32Camera:before takeImage");
    if (ESP32Camera::wifiClient.connect(serverIP.c_str(), serverPort)) {
        Serial.println("takeImage:Connection successful!");

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
        for (size_t n = 0; n < fbLen; n = n + 1024) {
            if (n + 1024 < fbLen) {
                ESP32Camera::wifiClient.write(fbBuf, 1024);
                fbBuf += 1024;
            } else if (fbLen % 1024 > 0) {
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

        // Serial.println(getBody);
    } else {
        getBody = "Connection to " + serverIP + " failed.";
        Serial.println(getBody);
    }
    return getBody;
}

esp_err_t ESP32Camera::stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[64];

    res = httpd_resp_set_type(req, ESP32Camera::_STREAM_CONTENT_TYPE.c_str());
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            if (fb->width > 400) {
                if (fb->format != PIXFORMAT_JPEG) {
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if (!jpeg_converted) {
                        Serial.println("JPEG compression failed");
                        res = ESP_FAIL;
                    }
                } else {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
            }
        }
        if (res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 64, ESP32Camera::_STREAM_PART.c_str(), _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, ESP32Camera::_STREAM_BOUNDARY.c_str(), strlen(ESP32Camera::_STREAM_BOUNDARY.c_str()));
        }
        if (fb) {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK) {
            break;
        }
        //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
    }
    return res;
}
httpd_handle_t ESP32Camera::stream_httpd=NULL;

void ESP32Camera::startCameraServer() {
    Serial.println("ES32Camera::startCameraServer");
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = ESP32Camera::stream_handler,
        .user_ctx = NULL};

    //Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&ESP32Camera::stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(ESP32Camera::stream_httpd, &index_uri);
        Serial.println("ES32Camera:: start stream  ok");
    } else {
        Serial.println("ES32Camera:: start stream fail");
    }
}

void ESP32Camera::stopCameraServer() {
    if (ESP32Camera::stream_httpd != NULL) {
        httpd_stop(&ESP32Camera::stream_httpd);
    }

}


String ESP32Camera::_STREAM_CONTENT_TYPE;
String ESP32Camera::_STREAM_BOUNDARY;
String ESP32Camera::_STREAM_PART;

bool ESP32Camera::initCamera() {
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
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;

   
    ESP32Camera::_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
    ESP32Camera::_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
    ESP32Camera::_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

    if (psramFound()) {
        config.frame_size = FRAMESIZE_SVGA;  // FRAMESIZE_SVGA
        config.jpeg_quality = 10;            //0-63 lower number means higher quality 10
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_CIF;
        config.jpeg_quality = 12;  //0-63 lower number means higher quality 1
        config.fb_count = 1;
    }

    esp_err_t result = esp_camera_init(&config);

    if (result != ESP_OK) {
        return false;
    }

    return true;
}

void ESP32Camera::initCameraStream() {
   
}
