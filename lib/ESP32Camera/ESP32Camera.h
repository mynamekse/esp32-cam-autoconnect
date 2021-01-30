#ifndef ESP32Camera_H
#define ESP32Camera_H
#include <Arduino.h>
#include <WiFi.h>
#include "esp_http_server.h"

class ESP32Camera {
   private:
   public:
    static String serverIP;
    static String serverPath;

    static String _STREAM_CONTENT_TYPE;
    static String _STREAM_BOUNDARY;
    static String _STREAM_PART;
    
    static httpd_handle_t stream_httpd ;
    static WiFiClient wifiClient;
    static String takeImage();
    static void startCameraServer();
    static void stopCameraServer();
    static esp_err_t stream_handler(httpd_req_t *req);
    static void setup();
 
    static bool initCamera();
    static void initCameraStream();
};

#endif
