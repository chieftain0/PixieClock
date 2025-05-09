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

#include "config.h"

#include <WiFi.h>

#include "Time_Temp.h"
struct tm timeinfo;
uint8_t lastSyncHour = 255;
char city[64] = "London";
int timezoneOffset = 0;

#include "Display.h"
#include <FastLED.h>
CRGB PIXELS[NUM_SEGS][NUM_LEDS_PER_SEG];
uint8_t brightness = 25;

void setup()
{
    Serial.begin(115200);

    // Initialize GPIO
    pinMode(SEG0_PIN, OUTPUT);
    pinMode(SEG1_PIN, OUTPUT);
    pinMode(SEG2_PIN, OUTPUT);
    pinMode(SEG3_PIN, OUTPUT);
    pinMode(SENSE_PIN_0, INPUT);
    pinMode(SENSE_PIN_1, INPUT);
    pinMode(SENSE_PIN_2, INPUT);
    pinMode(SENSE_PIN_3, INPUT);

    // Initialize LEDs
    FastLED.addLeds<WS2812B, SEG0_PIN, GRB>(PIXELS[0], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG1_PIN, GRB>(PIXELS[1], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG2_PIN, GRB>(PIXELS[2], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG3_PIN, GRB>(PIXELS[3], NUM_LEDS_PER_SEG);

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
    double temporary;
    GetCity(city, sizeof(city));
    CallOpenWeatherMap(city, OPENWEATHERMAP_API_KEY, temporary, timezoneOffset);
    Serial.println(city);

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
        Serial.println("Syncing time");
        if (!GetTimeFromRTC(&timeinfo, 10))
        {
            Serial.println("Failed to sync time. Rebooting...");
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
            CallOpenWeatherMap(city, OPENWEATHERMAP_API_KEY, OutTemp, timezoneOffset);
        }
        DisplayTemperature(round(OutTemp), brightness, CRGB::Blue, PIXELS);
    }

    // Show room temperature
    else
    {
        if (DS18.readTemp())
        {
            int temp = round(DS18.getTemp());
            DisplayTemperature(temp, brightness, CRGB::Green, PIXELS);
        }
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