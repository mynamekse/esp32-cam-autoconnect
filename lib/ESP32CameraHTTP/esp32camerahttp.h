#ifndef __ESP32CAMERAHTTP_H__
#define __ESP32CAMERAHTTP_H__

#include <Arduino.h>
#include <WiFi.h>
#include <AutoConnect.h>
#include "esp_camera.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include "esp_http_server.h"
#include "esp_timer.h"

#include "img_converters.h"


#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"

class ESP32CameraHTTP {
   private:
   public:
    static String serverIP;
    static String serverPath;

    static void setup();
    static void startServer();
    static void handleCapture();

};


#endif  // __ESP32CAMERAHTTP_H__
