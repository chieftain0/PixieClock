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
long timezoneOffset = 0;

#include "include/Display.h"
CRGB PIXELS[NUM_SEGS][NUM_LEDS_PER_SEG];
uint8_t brightness = 5;

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
    gpio_set_direction(BUTTON1_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON2_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON3_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON4_PIN, GPIO_MODE_INPUT);

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
    static bool autoMode = true;
    static bool mode = false;
    static long manualModeTimer = 0;

    static double InTemp = 99;
    static double OutTemp = 99;

    if (autoMode)
    {
        /*
            AUTO Mode
            50 seconds: Show time
            55 seconds: Show outdoor temperature
            60 seconds: Show room temperature
        */
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
            if (DS18.readTemp())
            {
                InTemp = round(DS18.getTemp());
            }
            DisplayTemperature(InTemp, brightness, CRGB::Green, PIXELS);
            hasRequestedTemp = false;
        }
    }
    else
    {
        /*
            MANUAL Mode
            Responds to button presses.
            Each mode is displayed for 5 seconds.
            Then switches to AUTO mode.
        */
        if (mode)
        {
            DisplayTemperature(InTemp, brightness, CRGB::Green, PIXELS);
        }
        else
        {
            DisplayTemperature(OutTemp, brightness, CRGB::Blue, PIXELS);
        }

        if (millis() - manualModeTimer >= 5000)
        {
            autoMode = true;
            mode = 0;
        }
    }

    // Set first indicator LEDs
    CRGB color0 = !gpio_get_level(SENSE_PIN_0) ? CRGB::Green : CRGB::Red;
    CRGB color1 = !gpio_get_level(SENSE_PIN_1) ? CRGB::Green : CRGB::Red;
    CRGB color2 = !gpio_get_level(SENSE_PIN_2) ? CRGB::Green : CRGB::Red;
    CRGB color3 = !gpio_get_level(SENSE_PIN_3) ? CRGB::Green : CRGB::Red;
    SetFirstPixels(color0, color1, color2, color3, brightness, PIXELS);

    // Check for button presses
    // TODO: Assign functions to buttons
    static bool button1Flag = false;
    static bool button2Flag = false;
    static bool button3Flag = false;
    static bool button4Flag = false;
    if (!gpio_get_level(BUTTON1_PIN) && !button1Flag) // Increase brightness
    {
        button1Flag = true;
        if (brightness <= 250)
        {
            brightness += 5;
        }
        else
        {
            brightness = 255;
        }
    }
    else if (gpio_get_level(BUTTON1_PIN) && button1Flag)
    {
        button1Flag = false;
    }
    if (!gpio_get_level(BUTTON2_PIN) && !button2Flag) // Decrease brightness
    {
        button2Flag = true;
        if (brightness >= 5)
        {
            brightness -= 5;
        }
        else
        {
            brightness = 0;
        }
    }
    else if (gpio_get_level(BUTTON2_PIN) && button2Flag)
    {
        button2Flag = false;
    }
    if (!gpio_get_level(BUTTON3_PIN) && !button3Flag) // Switch to manual mode and cycle through modes
    {
        button3Flag = true;

        autoMode = false;
        mode = !mode;
        manualModeTimer = millis();
    }
    else if (gpio_get_level(BUTTON3_PIN) && button3Flag)
    {
        button3Flag = false;
    }
    if (!gpio_get_level(BUTTON4_PIN) && !button4Flag)
    {
        button4Flag = true;
    }
    else if (gpio_get_level(BUTTON4_PIN) && button4Flag)
    {
        button4Flag = false;
    }
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

/**
 * @brief Generates a tone on the specified buzzer pin.
 * @param buzzerPin The GPIO pin to output the tone on.
 * @param frequency The frequency of the tone in Hz.
 * @param durationMS The duration of the tone in milliseconds.
 * @details
 *  This function uses the analogWrite() function to generate a tone.
 *  The resolution is set to 8 bits, and the frequency is set to the
 *  specified frequency. The function then waits for the specified
 *  duration and sets the output to 0 to stop the tone.
 *  @warning This function is currently blocking. Use with caution.
 */
void buzz(gpio_num_t buzzerPin, long frequency, long durationMS)
{
    analogWriteResolution(buzzerPin, 8);
    analogWriteFrequency(buzzerPin, frequency);

    static long start = millis();
    while (millis() - start < durationMS)
    {
        analogWrite(buzzerPin, 128);
    }
    analogWrite(buzzerPin, 0);
}