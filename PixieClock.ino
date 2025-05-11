/*
    .------------------------------------------------.
    | ____  _      _       ____ _               _    |
    ||  _ \(_)_  _(_) ___ / ___| |    ___   ___| | __|
    || |_) | \ \/ / |/ _ \ |   | |   / _ \ / __| |/ /|
    ||  __/| |>  <| |  __/ |___| |__| (_) | (__|   < |
    ||_|   |_/_/\_\_|\___|\____|_____\___/ \___|_|\_\|
    '------------------------------------------------'

    PixieClock
    Copyright (C) 2025  Behruz Erkinov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>
*/

#include "include/config.h"

#include <WiFi.h>

#include "include/WeatherTempLocationUtils.h"
struct tm timeinfo;
uint8_t lastSyncHour = 255;
char city[64] = "London";
char countryCode[4] = "GB";
int timezoneOffset = 0;

#include "include/Display.h"
#include <FastLED.h>
CRGB PIXELS[NUM_SEGS][NUM_LEDS_PER_SEG];
uint8_t brightness = 25;

void setup()
{
    Serial.begin(115200);

    // Initialize GPIO
    gpio_set_direction(SEG0_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SEG1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SEG2_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SEG3_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SENSE_PIN_0, GPIO_MODE_INPUT);
    gpio_set_direction(SENSE_PIN_1, GPIO_MODE_INPUT);
    gpio_set_direction(SENSE_PIN_2, GPIO_MODE_INPUT);
    gpio_set_direction(SENSE_PIN_3, GPIO_MODE_INPUT);

    // Initialize LEDs
    DisplayInit(PIXELS);

    // Connect to WiFi
    SetFirstPixels(CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, brightness, PIXELS);
    Display('W', 'I', 'F', 'I', brightness, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, PIXELS);
    Serial.print("Connecting to WiFi: ");
    Serial.println(SSID);
    if (isEAP)
    {
        WiFi.begin(SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, PASSWORD);
    }
    else
    {
        WiFi.begin(SSID, PASSWORD);
    }
    while (WiFi.status() != WL_CONNECTED)
    {
        SafeDelay(100);
    }

    // Get Geolocation
    Serial.print("Getting Geolocation: ");
    Display('C', 'I', 'T', 'Y', brightness, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, PIXELS);
    timezoneOffset = GetTzOffsetAndCity(city, sizeof(city), countryCode, sizeof(countryCode));
    Serial.print(city);
    Serial.print(", ");
    Serial.print(countryCode);
    Serial.print(", Offset: ");
    Serial.println(timezoneOffset);

    // Sync time for the first time
    Serial.print("Syncing time: ");
    Display('T', 'I', 'M', 'E', brightness, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, PIXELS);
    configTime(timezoneOffset, 0, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    if (!GetTimeFromRTC(&timeinfo, 10))
    {
        Serial.println("\nFailed to sync time. Rebooting...");
        ESP.restart();
    }
    lastSyncHour = timeinfo.tm_hour;
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void loop()
{
    // Get time for this loop
    if (!GetTimeFromRTC(&timeinfo, 10))
    {
        Serial.println("Failed to sync time. Rebooting...");
        ESP.restart();
    }

    // Check if time needs to be synced
    if (lastSyncHour != timeinfo.tm_hour || timeinfo.tm_year < 125) // 125 is 2025
    {
        Serial.print("Syncing time: ");
        Display('T', 'I', 'M', 'E', brightness, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, PIXELS);
        configTime(timezoneOffset, 0, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
        if (!GetTimeFromRTC(&timeinfo, 10))
        {
            Serial.println("\nFailed to sync time. Rebooting...");
            ESP.restart();
        }
        lastSyncHour = timeinfo.tm_hour;
        Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    }

    static bool hasRequestedTemp = false;
    // Show time for first 55 seconds
    if (timeinfo.tm_sec < 50)
    {
        DisplayTime(timeinfo, brightness, CRGB::Red, PIXELS);

        // Set the flag for the next mode
        hasRequestedTemp = false;
    }

    // Show outdoor temperature
    else if (timeinfo.tm_sec >= 50 && timeinfo.tm_sec < 55)
    {
        static double OutTemp = 99;
        if (!hasRequestedTemp)
        {
            // Request temperature for both modes
            hasRequestedTemp = true;
            DS18.requestTemp();
            OutTemp = GetOutdoorTemp(city, countryCode, OPENWEATHERMAP_API_KEY);
        }
        DisplayTemperature(round(OutTemp), brightness, CRGB::Blue, PIXELS);
    }

    // Show room temperature
    else if (timeinfo.tm_sec >= 55)
    {
        static double InTemp = 99;
        if (DS18.readTemp())
        {
            InTemp = round(DS18.getTemp());
        }
        DisplayTemperature(InTemp, brightness, CRGB::Green, PIXELS);
        hasRequestedTemp = false;
    }

    // Reboot at midnight every day
    // if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 0 && lastSyncHour != 0)
    // {
    //     Serial.println("Long day! Rebooting...");
    //     ESP.restart();
    // }

    CRGB color0 = !gpio_get_level(SENSE_PIN_0) ? CRGB::Green : CRGB::Red;
    CRGB color1 = !gpio_get_level(SENSE_PIN_1) ? CRGB::Green : CRGB::Red;
    CRGB color2 = !gpio_get_level(SENSE_PIN_2) ? CRGB::Green : CRGB::Red;
    CRGB color3 = !gpio_get_level(SENSE_PIN_3) ? CRGB::Green : CRGB::Red;
    SetFirstPixels(color0, color1, color2, color3, brightness, PIXELS);
}

/**
 * @brief      A safe version of delay() that uses yield() to keep the watchdog happy.
 *
 * @param  ms    The time to wait in milliseconds.
 */
void SafeDelay(unsigned long ms)
{
    unsigned long start = millis();
    while (millis() - start < ms)
    {
        yield();
    }
}

/**
 * @brief Synchronizes the system time using the Real Time Clock (RTC)
 * @param timeinfo A pointer to a tm struct where the synced time will be stored
 * @param num_tries The maximum number of attempts to retrieve the time
 * @return True if the time was successfully synchronized, false otherwise
 * @details
 *  This function attempts to get the local time from the RTC and stores
 *  it in the provided tm struct. If the initial attempt fails, it retries
 *  up to the specified number of attempts, with a delay between each try.
 *  If the function exhausts all attempts without success, it returns false.
 */
bool GetTimeFromRTC(tm *timeinfo, int num_tries)
{
    if (!getLocalTime(timeinfo))
    {
        int retries = 0;
        while (!getLocalTime(timeinfo) && retries < num_tries)
        {
            SafeDelay(500);
            retries++;
        }
        if (retries >= num_tries)
        {
            return false;
        }
    }
    return true;
}