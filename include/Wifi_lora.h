/*
    configuramos la actualización de firmware mediante OTA
*/

#include <Arduino.h>
#include <ArduinoOTA.h>


const char* ssid = "Isma";
const char* password = "12345678";

String hostname = "ESP32_E32";


void wifi_config(void);
