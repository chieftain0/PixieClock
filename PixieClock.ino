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

#define FASTLED_RMT_BUILTIN_DRIVER 0
#include <FastLED.h>
#define NUM_SEGS 4
#define NUM_LEDS_PER_SEG 23
#define SEG0_PIN 7
#define SEG1_PIN 8
#define SEG2_PIN 9
#define SEG3_PIN 10
uint8_t brightness = 150;
CRGB PIXELS[NUM_SEGS][NUM_LEDS_PER_SEG];

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

bool onDisplay[NUM_SEGS][NUM_LEDS_PER_SEG] = {0};
unsigned long start = 0;

void setup()
{
    FastLED.addLeds<WS2812B, SEG0_PIN, GRB>(PIXELS[0], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG1_PIN, GRB>(PIXELS[1], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG2_PIN, GRB>(PIXELS[2], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG3_PIN, GRB>(PIXELS[3], NUM_LEDS_PER_SEG);

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

    for (int i = 0; i < NUM_LEDS_PER_SEG; i++)
    {
        onDisplay[0][i] = patterns[hour[0]][i];
        onDisplay[1][i] = patterns[hour[1]][i];
        onDisplay[2][i] = patterns[minute[0]][i];
        onDisplay[3][i] = patterns[minute[1]][i];
    }

    start = millis();
    while (millis() - start < 60000 - seconds[0] * 10000 - seconds[1] * 1000)
    {
        for (int i = 0; i < NUM_SEGS; i++)
        {
            for (int j = 0; j < NUM_LEDS_PER_SEG; j++)
            {
                if (onDisplay[i][j])
                {
                    PIXELS[i][j] = CRGB::Red;
                }
                else
                {
                    PIXELS[i][j] = CRGB::Black;
                }
            }
        }
        FastLED.show();
        Delay(500);

        for (int i = 0; i < NUM_LEDS_PER_SEG / 4; i++)
        {
            PIXELS[3][i] = CRGB::Black;
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
