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

// modify Secrets.h to manage your credentials:
#include <Secrets.h>
// Otherwise:
// const bool isEAP = false;
// const char *SSID = "";
// const char *PASSWORD = "";
// const char *EAP_IDENTITY = ""; // Optional
// const char *EAP_USERNAME = ""; // Optional
// const char *NTP_SERVER1 = "";
// const char *NTP_SERVER2 = ""; // Optional
// const char *NTP_SERVER3 = ""; // Optional
// const int *GMT_TIMEZONE = 0;
// const char *OPENWEATHERMAP_API_KEY = "";
// const char *OPENWEATHERMAP_CITY = "";

#include <WiFi.h>

#include <time.h>
struct tm timeinfo;
uint8_t lastSyncHour = 255;

#include <FastLED.h>
#define NUM_SEGS 4
#define NUM_LEDS_PER_SEG 23
#define SEG0_PIN GPIO_NUM_7
#define SEG1_PIN GPIO_NUM_8
#define SEG2_PIN GPIO_NUM_9
#define SEG3_PIN GPIO_NUM_10
#define SENSE_PIN_0 GPIO_NUM_18
#define SENSE_PIN_1 GPIO_NUM_8
#define SENSE_PIN_2 GPIO_NUM_9
#define SENSE_PIN_3 GPIO_NUM_10

CRGB PIXELS[NUM_SEGS][NUM_LEDS_PER_SEG];
uint8_t brightness = 150;

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GyverDS18.h>
#define DS18_PIN GPIO_NUM_11
GyverDS18Single DS18(DS18_PIN);
bool hasRequestedTemp = false;
double temp = 99;

// Character maps
static const bool patterns[17][NUM_LEDS_PER_SEG] = {
    {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // 0
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 1
    {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0}, // 2
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0}, // 3
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0}, // 4
    {0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0}, // 5
    {0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // 6
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 7
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // 8
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0}, // 9
    {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // A
    {0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // B
    {0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // C
    {0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // D
    {0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // E
    {0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // F
    {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0}, // °
};

unsigned long timePast = 0;

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
    Serial.println("Connected to WiFi");

    // Sync time for the first time
    configTime(GMT_TIMEZONE * 3600, 0, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    Serial.println("Syncing time");
    if (!GetTimeFromRTC(&timeinfo, 10))
    {
        Serial.println("Failed to sync time. Rebooting...");
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
    if (lastSyncHour != timeinfo.tm_hour || timeinfo.tm_year < 125)
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

    // Show time for first 55 seconds
    if (timeinfo.tm_sec < 50)
    {
        DisplayTime(timeinfo, (millis() - timePast >= 500), CRGB::Red, brightness, PIXELS);
        timePast = millis();

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
            temp = GetOutdoorTemperature(OPENWEATHERMAP_CITY, OPENWEATHERMAP_API_KEY);
        }
        DisplayTemperature(round(temp), CRGB::Blue, brightness, (millis() - timePast >= 500), PIXELS);
        timePast = millis();

    }

    // Show room temperature
    else
    {
        if (DS18.readTemp())
        {
            int temp = round(DS18.getTemp());
            DisplayTemperature(temp, CRGB::Green, brightness, (millis() - timePast >= 500), PIXELS);
            timePast = millis();
        }
        hasRequestedTemp = false;
    }

    // Reboot at midnight every day
    // if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 0 && lastSyncHour != 0)
    // {
    //     Serial.println("Long day! Rebooting...");
    //     ESP.restart();
    // }
}

void DisplayTime(tm &time_struct, bool lastDigitOn, CRGB color, double brightness, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    FastLED.setBrightness(brightness);

    bool onDisplay[NUM_SEGS][NUM_LEDS_PER_SEG] = {0};
    uint8_t hour[2], minute[2], seconds[2];
    hour[0] = time_struct.tm_hour / 10;
    hour[1] = time_struct.tm_hour % 10;
    minute[0] = time_struct.tm_min / 10;
    minute[1] = time_struct.tm_min % 10;
    seconds[0] = time_struct.tm_sec / 10;
    seconds[1] = time_struct.tm_sec % 10;

    for (int i = 0; i < NUM_LEDS_PER_SEG; i++)
    {
        onDisplay[0][i] = patterns[hour[0]][i];
        onDisplay[1][i] = patterns[hour[1]][i];
        onDisplay[2][i] = patterns[minute[0]][i];
        onDisplay[3][i] = patterns[minute[1]][i];
    }

    for (int i = 0; i < NUM_SEGS - 1; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_SEG; j++)
        {
            if (onDisplay[i][j])
            {
                pixels[i][j] = color;
            }
            else
            {
                pixels[i][j] = CRGB::Black;
            }
        }
    }

    if (lastDigitOn)
    {
        for (int i = 0; i < NUM_LEDS_PER_SEG; i++)
        {
            if (onDisplay[4][i])
            {
                pixels[4][i] = color;
            }
            else
            {
                pixels[4][i] = CRGB::Black;
            }
        }
    }
    else
    {
        for (int i = 0; i < NUM_LEDS_PER_SEG; i++)
        {
            pixels[4][i] = CRGB::Black;
        }
    }

    FastLED.show();
}

void DisplayTemperature(int temp, CRGB color, double brightness, bool secondsDisplayOn, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    bool onDisplay[NUM_SEGS][NUM_LEDS_PER_SEG] = {0};

    int first_digit = temp / 10;
    int second_digit = temp % 10;

    for (int i = 0; i < NUM_LEDS_PER_SEG; i++)
    {
        onDisplay[0][i] = patterns[first_digit][i];
        onDisplay[1][i] = patterns[second_digit][i];
        onDisplay[2][i] = patterns[16][i]; // Display degree symbol
        onDisplay[3][i] = patterns[13][i]; // Display C
    }

    for (int i = 0; i < NUM_SEGS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_SEG; j++)
        {
            if (onDisplay[i][j])
            {
                pixels[i][j] = color;
            }
            else
            {
                pixels[i][j] = CRGB::Black;
            }
        }
    }

    if (secondsDisplayOn)
    {
        for (int i = 0; i < NUM_LEDS_PER_SEG; i++)
        {
            if (onDisplay[3][i])
            {
                pixels[3][i] = color;
            }
            else
            {
                pixels[3][i] = CRGB::Black;
            }
        }
    }
    else
    {
        for (int i = 0; i < NUM_LEDS_PER_SEG; i++)
        {
            pixels[4][i] = CRGB::Black;
        }
    }

    FastLED.show();
}

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

double GetOutdoorTemperature(const char *city, const char *apiKey)
{
    char URL[256];
    snprintf(URL,
             sizeof(URL),
             "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric",
             city, apiKey);

    WiFiClient client;
    HTTPClient http;
    http.begin(client, URL);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        int len = http.getSize();
        char json[len + 1];

        int bytesRead = http.getStream().readBytes(json, len);
        json[bytesRead] = '\0';

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, json);

        client.stop();
        http.end();

        if (error)
        {
            client.stop();
            http.end();
            return 99;
        }
        return double(doc["main"]["temp"]);
    }
    else
    {
        client.stop();
        http.end();
        return 99;
    }
}

void SafeDelay(unsigned long ms)
{
    unsigned long start = millis();
    while (millis() - start < ms)
    {
        yield();
    }
}