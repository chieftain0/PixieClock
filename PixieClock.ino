/*
    .------------------------------------------------.
    | ____  _      _       ____ _               _    |
    ||  _ \(_)_  _(_) ___ / ___| |    ___   ___| | __|
    || |_) | \ \/ / |/ _ \ |   | |   / _ \ / __| |/ /|
    ||  __/| |>  <| |  __/ |___| |__| (_) | (__|   < |
    ||_|   |_/_/\_\_|\___|\____|_____\___/ \___|_|\_\|
    '------------------------------------------------'

    PixieClock
    Copyright (C) 2026 Behruz Erkinov

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

#include <WiFi.h>
#include "include/config.h"
#include "include/display.h"
#include "include/wtl_utils.h"

struct tm timeinfo;
uint8_t lastSyncHour = 255;
char city[64] = "London";
char countryCode[4] = "GB";
int32_t timezoneOffset = 0;
bool isSunUp = false;

CRGB PIXELS[NUM_SEGS][NUM_LEDS_PER_SEG];
uint8_t brightness = 5;

uint8_t buzzer_duty_cycle = 10;
uint8_t buzzer_duration_ms = 100;

void setup()
{
    // Serial.begin(115200);

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

    // Initialize buzzers
    buzz_setup(BUZZER_FREQ_HZ);

    // Initialize LEDs
    DisplayInit(PIXELS);

    // Connect to WiFi
    SetFirstPixels(CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, PIXELS);
    Display('W', 'I', 'F', 'I', Colors::MessageChar0, Colors::MessageChar1, Colors::MessageChar2, Colors::MessageChar3, PIXELS);
    UpdateDisplay(brightness, PIXELS);
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
    Display('C', 'I', 'T', 'Y', Colors::MessageChar0, Colors::MessageChar1, Colors::MessageChar2, Colors::MessageChar3, PIXELS);
    UpdateDisplay(brightness, PIXELS);
    timezoneOffset = GetTzOffsetAndCity(city, sizeof(city), countryCode, sizeof(countryCode));
    Serial.print(city);
    Serial.print(", ");
    Serial.print(countryCode);
    Serial.print(", Offset: ");
    Serial.println(timezoneOffset);

    // Sync time for the first time
    Serial.print("Syncing time: ");
    Display('T', 'I', 'M', 'E', Colors::MessageChar0, Colors::MessageChar1, Colors::MessageChar2, Colors::MessageChar3, PIXELS);
    UpdateDisplay(brightness, PIXELS);
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
    // Reboot every 24 hours
    if (millis() >= 24 * 3600 * 1000)
    {
        ESP.restart();
    }

    static bool autoMode = true;
    static bool mode = false;
    static long manualModeTimer = 0;

    static double InTemp = 99;
    static double OutTemp = 99;
    static uint32_t SunriseTime = 0;
    static uint32_t SunsetTime = 0;

    if (autoMode)
    {
        /*
            AUTO Mode
            [0, 50) seconds: Show time
            [50, 55) seconds: Show outdoor temperature
            [55, 60) seconds: Show room temperature
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
            Display('T', 'I', 'M', 'E', Colors::MessageChar0, Colors::MessageChar1, Colors::MessageChar2, Colors::MessageChar3, PIXELS);
            UpdateDisplay(brightness, PIXELS);
            configTime(timezoneOffset, 0, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
            if (!GetTimeFromRTC(&timeinfo, 10))
            {
                Serial.println("\nFailed to sync time. Rebooting...");
                ESP.restart();
            }
            lastSyncHour = timeinfo.tm_hour;
            Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        }

        static bool hasRequestedInAndOutData = false;
        // Show time for first 55 seconds
        if (timeinfo.tm_sec < 50)
        {
            DisplayTime(timeinfo, Colors::TimeHourTens, Colors::TimeHourOnes, Colors::TimeMinTens, Colors::TimeMinOnes, PIXELS);

            // Set the flag for the next mode
            hasRequestedInAndOutData = false;
        }

        // Fetch outdoor and indoor data
        else if (timeinfo.tm_sec >= 50 && timeinfo.tm_sec < 55)
        {
            if (!hasRequestedInAndOutData)
            {
                // Request temperature for both modes
                hasRequestedInAndOutData = true;
                DS18.requestTemp();
                FetchOutdoorData(city, countryCode, OPENWEATHERMAP_API_KEY, OutTemp, SunriseTime, SunsetTime);

                // Sunset and Sunrise times are in current tz epoch format. Convert them to seconds since midnight for easier comparison with current time.
                SunriseTime = SunriseTime % 86400; // Seconds since midnight
                SunsetTime = SunsetTime % 86400;   // Seconds since midnight
                uint32_t currentTimeInSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

                if (currentTimeInSeconds >= SunriseTime && currentTimeInSeconds < SunsetTime && isSunUp == false)
                {
                    isSunUp = true;
                    if (brightness < 128)
                    {
                        brightness = brightness * 2; // Double the brightness when the sun is up
                    }
                    else
                    {
                        brightness = 255; // Set to max brightness if it was already high
                    }
                }
                if ((currentTimeInSeconds < SunriseTime || currentTimeInSeconds >= SunsetTime) && isSunUp == true)
                {
                    isSunUp = false;
                    if (brightness >= 4)
                    {
                        brightness = brightness / 2; // Halve the brightness when the sun is down
                    }
                    else
                    {
                        brightness = 2; // Set to minimum brightness if it was already low
                    }
                }
            }

            // Show outdoor temperature for 5 seconds
            DisplayTemperature(OutTemp, Colors::OutdoorTempTens, Colors::OutdoorTempOnes, Colors::OutdoorTempDeg, Colors::OutdoorTempCelsius, PIXELS);
        }

        else if (timeinfo.tm_sec >= 55)
        {
            // Read and show indoor temperature for 5 seconds
            if (DS18.readTemp())
            {
                InTemp = round(DS18.getTemp());
            }
            DisplayTemperature(InTemp, Colors::IndoorTempTens, Colors::IndoorTempOnes, Colors::IndoorTempDeg, Colors::IndoorTempCelsius, PIXELS);
            hasRequestedInAndOutData = false;
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
            DisplayTemperature(InTemp, Colors::IndoorTempTens, Colors::IndoorTempOnes, Colors::IndoorTempDeg, Colors::IndoorTempCelsius, PIXELS);
        }
        else
        {
            DisplayTemperature(OutTemp, Colors::OutdoorTempTens, Colors::OutdoorTempOnes, Colors::OutdoorTempDeg, Colors::OutdoorTempCelsius, PIXELS);
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
    SetFirstPixels(color0, color1, color2, color3, PIXELS);

    // Check for button presses
    // TODO: Assign functions to buttons
    static bool button1Flag = false;
    static bool button2Flag = false;
    static bool button3Flag = false;
    static bool button4Flag = false;

    // Button 1
    if (!gpio_get_level(BUTTON1_PIN) && !button1Flag) // Increase brightness
    {
        buzz(buzzer_duty_cycle, buzzer_duration_ms);
        button1Flag = true;
        if (brightness >= 251 && brightness < 255)
        {
            brightness = 255;
        }
        else if (brightness == 255)
        {
        }
        else if (brightness >= 5)
        {
            brightness += 5;
        }
        else
        {
            brightness += 1;
        }
    }
    else if (gpio_get_level(BUTTON1_PIN) && button1Flag)
    {
        button1Flag = false;
    }

    // Button 2
    if (!gpio_get_level(BUTTON2_PIN) && !button2Flag) // Decrease brightness
    {
        buzz(buzzer_duty_cycle, buzzer_duration_ms);
        button2Flag = true;

        if (brightness <= 2)
        {
        }
        else if (brightness <= 5 && brightness > 2)
        {
            brightness -= 1;
        }
        else
        {
            brightness -= 5;
        }
    }
    else if (gpio_get_level(BUTTON2_PIN) && button2Flag)
    {
        button2Flag = false;
    }

    // Button 3
    if (!gpio_get_level(BUTTON3_PIN) && !button3Flag) // Switch to manual mode and cycle through modes
    {
        buzz(buzzer_duty_cycle, buzzer_duration_ms);
        button3Flag = true;

        autoMode = false;
        mode = !mode;
        manualModeTimer = millis();
    }
    else if (gpio_get_level(BUTTON3_PIN) && button3Flag)
    {
        button3Flag = false;
    }

    // Button 4
    if (!gpio_get_level(BUTTON4_PIN) && !button4Flag)
    {
        buzz(buzzer_duty_cycle, buzzer_duration_ms);
        button4Flag = true;

        autoMode = false;
        mode = !mode;
        manualModeTimer = millis();
    }
    else if (gpio_get_level(BUTTON4_PIN) && button4Flag)
    {
        button4Flag = false;
    }

    UpdateDisplay(brightness, PIXELS);
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
 * @brief Sets up the buzzer pins for tone generation.
 * @param frequency The frequency of buzzer the tone in Hz.
 * @details
 * This function initializes the buzzer pins for tone generation by setting
 * the PWM frequency and resolution. It prepares the buzzer pins to generate
 * tones at the specified frequency.
 *  @warning This function should be called before using the buzz() function.
 */
void buzz_setup(uint32_t frequency)
{
    analogWriteResolution(BUZZER1_PIN, 8);
    analogWriteResolution(BUZZER2_PIN, 8);
    analogWriteFrequency(BUZZER1_PIN, frequency);
    analogWriteFrequency(BUZZER2_PIN, frequency);
}

/**
 * @brief Generates a tone on the specified buzzer pin.
 * @param duty_cycle The duty cycle of the PWM signal (0-255).
 * @param durationMS The duration of the tone in milliseconds.
 * @details
 * This function generates a tone on the buzzer for the specified duration.
 * It uses the analogWrite function to set the duty cycle of the PWM signal,
 * which controls the volume of the tone. After the specified duration,
 * the buzzer is turned off.
 *  @warning This function is currently blocking. Use with caution.
 */
void buzz(uint8_t duty_cycle, long durationMS)
{
    analogWrite(BUZZER1_PIN, duty_cycle);
    analogWrite(BUZZER2_PIN, duty_cycle);
    SafeDelay(durationMS);
    analogWrite(BUZZER1_PIN, 0);
    analogWrite(BUZZER2_PIN, 0);
}
