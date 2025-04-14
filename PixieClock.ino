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

// modify Secrets.h to manage your credentials
#include <Secrets.h>

#include <WiFi.h>

#include <time.h>
struct tm timeinfo;
uint8_t hour[2], minute[2], seconds[2];

#include <FastLED.h>
#define NUM_LEDS 80
#define LED_PIN 10
uint8_t brightness = 150;
CRGB LED[NUM_LEDS];
bool onDisplay[NUM_LEDS] = {0};

static const bool patterns[17][20] = {
    {0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0}, // 0
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 1
    {1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1}, // 2
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1}, // 3
    {1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1}, // 4
    {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1}, // 5
    {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1}, // 9
    {0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // A
    {1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // B
    {0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0}, // C
    {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // D
    {1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // E
    {1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // F
    {0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}, // °
};

unsigned long start = 0;

void setup()
{
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(LED, NUM_LEDS);

    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        Delay(100);
    }

    configTime(GMT_TIMEZONE * 3600, 0, NTP_SERVER);
}

void loop()
{
    FastLED.setBrightness(brightness);

    if (!getLocalTime(&timeinfo))
    {
        return;
    }
    hour[0] = timeinfo.tm_hour / 10;
    hour[1] = timeinfo.tm_hour % 10;
    minute[0] = timeinfo.tm_min / 10;
    minute[1] = timeinfo.tm_min % 10;
    seconds[0] = timeinfo.tm_sec / 10;
    seconds[1] = timeinfo.tm_sec % 10;

    for (int i = 0; i < NUM_LEDS / 4; i++)
    {
        onDisplay[i] = patterns[hour[0]][i];
        onDisplay[i + NUM_LEDS / 4] = patterns[hour[1]][i];
        onDisplay[i + NUM_LEDS / 2] = patterns[minute[0]][i];
        onDisplay[i + NUM_LEDS / 2 + NUM_LEDS / 4] = patterns[minute[1]][i];
    }

    start = millis();
    while (millis() - start < 60000 - seconds[0] * 10000 - seconds[1] * 1000)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            if (onDisplay[i])
            {
                LED[i] = CRGB(0xFF0000);
            }
            else
            {
                LED[i] = CRGB::Black;
            }
        }
        FastLED.show();
        Delay(500);

        for (int i = 0; i < NUM_LEDS / 4; i++)
        {
            LED[NUM_LEDS - i - 1] = CRGB::Black;
        }
        FastLED.show();
        Delay(500);
    }
}

void Delay(unsigned long ms)
{
    unsigned long start = millis();
    while (millis() - start < ms)
    {
        yield();
    }
}
