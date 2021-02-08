#ifndef __ESP32CAMERAHTTP_H__
#define __ESP32CAMERAHTTP_H__

#include <Arduino.h>
#include <WiFi.h>

#include "esp_camera.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"

class ESP32CameraHTTP {
   private:
   public:
    static String serverIP;
    static String serverPath;
    static void setup();
};

#endif  // __ESP32CAMERAHTTP_H__
