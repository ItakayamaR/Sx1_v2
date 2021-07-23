/*
    configuramos la actualización de firmware mediante OTA
*/

#include <Arduino.h>
#include <ArduinoOTA.h>

extern const char* ssid;
extern const char* password;

extern const String hostname;

void wifi_config(void);
