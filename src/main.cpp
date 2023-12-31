#include <Arduino.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "ThingsBoardOTA.h"

const char ssid[] = "eFisheryPlus";
const char password[] = "123123123";
const char fw_ver[] = "1.0.7";

ThingsboardOTA ota;
const char* ThingsboardOTA::_current_fw_version = fw_ver;

void InitWiFi();
bool reconnect();

const uint8_t pin = 34;

void setup()
{
    Serial.begin(115200);
    pinMode(pin, INPUT_PULLDOWN);
    delay(1000);
    InitWiFi();
}

void loop()
{
    delay(1000);
    Serial.println("ini firmware TEST " + String(fw_ver));
    bool pinState = digitalRead(pin);
    // Serial.printf("pinState: %d\n", pinState);

    if (pinState) ota.do_update();

    if (!reconnect()) return;

    ota.reconnect();
    ota.loop();
}

void InitWiFi() {
#if THINGSBOARD_ENABLE_PROGMEM
  Serial.println(F("Connecting to AP ..."));
#else
  Serial.println("Connecting to AP ...");
#endif
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been succesfully established
    delay(500);
#if THINGSBOARD_ENABLE_PROGMEM
    Serial.print(F("."));
#else
    Serial.print(".");
#endif
  }
#if THINGSBOARD_ENABLE_PROGMEM
  Serial.println(F("Connected to AP"));
#else
  Serial.println("Connected to AP");
#endif
#if ENCRYPTED
  espClient.setCACert(ROOT_CERT);
#endif
}

bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}