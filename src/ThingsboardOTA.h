#pragma once

// #ifndef THINGS_BOARDOTA_H
// #define THINGS_BOARDOTA_H

#include <ThingsBoard.h>
#include <WiFiClientSecure.h>

constexpr char CURRENT_FIRMWARE_TITLE[] PROGMEM = "TEST";
constexpr char FW_STATE_UPDATED[] PROGMEM = "UPDATED";
constexpr uint8_t FIRMWARE_FAILURE_RETRIES PROGMEM = 5U;
constexpr uint16_t FIRMWARE_PACKET_SIZE PROGMEM = 4096U;
constexpr char TOKEN[] PROGMEM = "mrtI3ZbGiOelz26xxS47";
constexpr char THINGSBOARD_SERVER[] PROGMEM = "thingsboard.cloud"; //"demo.thingsboard.io";
constexpr uint16_t THINGSBOARD_PORT PROGMEM = 1883U;
constexpr uint32_t MAX_MESSAGE_SIZE PROGMEM = 512U;

class ThingsboardOTA
{
private:
    WiFiClient espClient;
    ThingsBoard tb;

    // Statuses for updating
    bool currentFWSent = false;
    bool updateRequestSent = false;

public:
    ThingsboardOTA();
    ~ThingsboardOTA();

    static const char* _current_fw_version;

    // @brief Updated callback that will be called as soon as the firmware update finishes
    /// @param success Either true (update succesfull) or false (update failed)
    static void updatedCallback(const bool &success);

    /// @brief Progress callback that will be called every time we downloaded a new chunk successfully
    /// @param currentChunk
    /// @param totalChuncks
    static void progressCallback(const uint32_t &currentChunk, const uint32_t &totalChuncks);

    bool reconnect();
    void loop();
};

const OTA_Update_Callback callback(&ThingsboardOTA::progressCallback, &ThingsboardOTA::updatedCallback, CURRENT_FIRMWARE_TITLE, ThingsboardOTA::_current_fw_version, FIRMWARE_FAILURE_RETRIES, FIRMWARE_PACKET_SIZE);

// #endif

ThingsboardOTA::ThingsboardOTA() : tb(espClient, MAX_MESSAGE_SIZE)
{
    // Initialize ThingsBoard instance with the maximum needed buffer size
}

ThingsboardOTA::~ThingsboardOTA()
{
}

void ThingsboardOTA::updatedCallback(const bool &success)
{
    if (success)
    {
#if THINGSBOARD_ENABLE_PROGMEM
        Serial.println(F("Done, Reboot now"));
#else
        Serial.println("Done, Reboot now");
#endif

#ifdef ESP8266
        ESP.restart();
#else
#ifdef ESP32
        esp_restart();
#endif // ESP32
#endif // ESP8266
        return;
    }
#if THINGSBOARD_ENABLE_PROGMEM
    Serial.println(F("Downloading firmware failed"));
#else
    Serial.println("Downloading firmware failed");
#endif
}

void ThingsboardOTA::progressCallback(const uint32_t &currentChunk, const uint32_t &totalChuncks)
{
    Serial.printf("Progress %.2f%%\n", static_cast<float>(currentChunk * 100U) / totalChuncks);
}

bool ThingsboardOTA::reconnect()
{
    if (!tb.connected())
    {
        // Reconnect to the ThingsBoard server,
        // if a connection was disrupted or has not yet been established
        Serial.printf("Connecting to: (%s) with token (%s)\n", THINGSBOARD_SERVER, TOKEN);
        if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT))
        {
#if THINGSBOARD_ENABLE_PROGMEM
            Serial.println(F("Failed to connect"));
#else
            Serial.println("Failed to connect");
#endif
            return false;
        }
    }
    return true;
}

void ThingsboardOTA::loop()
{
    if (!currentFWSent)
    {
        currentFWSent = tb.Firmware_Send_Info(CURRENT_FIRMWARE_TITLE, ThingsboardOTA::_current_fw_version) && tb.Firmware_Send_State(FW_STATE_UPDATED);
    }

    if (!updateRequestSent)
    {
#if THINGSBOARD_ENABLE_PROGMEM
        Serial.println(F("Firwmare Update..."));
#else
        Serial.println("Firwmare Update...");
#endif
        // See https://thingsboard.io/docs/user-guide/ota-uxpdates/
        // to understand how to create a new OTA pacakge and assign it to a device so it can download it.
        updateRequestSent = tb.Start_Firmware_Update(callback);
    }

    tb.loop();
}