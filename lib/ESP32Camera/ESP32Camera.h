#ifndef ESP32Camera_H
#define ESP32Camera_H
#include <Arduino.h>
#include <WiFi.h>

class ESP32Camera {
   private:
   public:
    static String serverIP;
    static String serverPath;
    static int serverPort;
    static String _STREAM_CONTENT_TYPE;
    static String _STREAM_BOUNDARY;
    static String _STREAM_PART;
    static int timoutTimer;
 
    static WiFiClient wifiClient;
    static String takeImage();
    
    static void startCameraServer();
    static void stopCameraServer();
     
    static void setup();
  
    static bool initCamera();
  
};

#endif
