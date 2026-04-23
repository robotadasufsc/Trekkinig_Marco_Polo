#include <Arduino.h>

#define ESP_LOG(level, tag, message) { Serial.print(level " "); Serial.print(tag); Serial.print(" "); Serial.println(message); }
#define ESP_LOGI_(tag, message) ESP_LOG("INFO", tag, message)
