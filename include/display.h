#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "config.h"

// Character maps
static const bool patterns[38][NUM_LEDS_PER_SEG] = {
    {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // 0
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, // 1
    {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0}, // 2
    {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0}, // 3
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0}, // 4
    {0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0}, // 5
    {0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // 6
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 7
    {0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0}, // 8
    {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0}, // 9
    {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // A - 10
    {0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // B - 11
    {0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // C - 12
    {0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // D - 13
    {0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // E - 14
    {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // F - 15
    {0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0}, // G - 16
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // H - 17
    {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1}, // I - 18
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0}, // J - 19
    {0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1}, // K - 20
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // L - 21
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0}, // M - 22
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0}, // N - 23
    {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // O - 24
    {0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, // P - 25
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0}, // Q - 26
    {0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1}, // R - 27
    {0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0}, // S - 28
    {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}, // T - 29
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0}, // U - 30
    {0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1}, // V - 31
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // W - 32
    {0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0}, // X - 33
    {0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1}, // Y - 34
    {0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0}, // Z - 35
    {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0}, // ° - 36
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // SPACE - 37
};
//    {0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1}, // K - 20 (Alternative pattern for K)

/**
 * @brief Initialize the LED segments for display
 * @param pixels a 2D array of CRGB to store the LED data for each segment
 * @details
 *  This function sets up the LED segments by associating each segment
 *  with its corresponding GPIO pin and initializing the LED data using
 *  the FastLED library.
 */
void DisplayInit(CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    FastLED.addLeds<WS2812B, SEG0_PIN, GRB>(pixels[0], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG1_PIN, GRB>(pixels[1], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG2_PIN, GRB>(pixels[2], NUM_LEDS_PER_SEG);
    FastLED.addLeds<WS2812B, SEG3_PIN, GRB>(pixels[3], NUM_LEDS_PER_SEG);
    FastLED.setCorrection(TypicalLEDStrip);
}

/**
 * @brief Look up the index from patterns array
 * @param c character to look up
 * @return index in patterns array, or 15 (F) if not found
 * @details
 * 0-9 is 0-9 and A-Z is 10-35, '°' is 36
 */
int CharToIndex(char c) // look up the index from patterns array
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 55;
    }
    else if (c >= '0' && c <= '9')
    {
        return c - 48;
    }
    else if (c == 96) // replace ` in ASCII with °
    {
        return 36;
    }
    else if (c == ' ') // SPACE
    {
        return 37;
    }
    else
    {
        return 15; // F
    }
}

/**
 * @brief Display 4 characters on 4 segments of LED segments
 * @param c0 character to display on first segment
 * @param c1 character to display on second segment
 * @param c2 character to display on third segment
 * @param c3 character to display on fourth segment
 * @param color0 color of first segment
 * @param color1 color of second segment
 * @param color2 color of third segment
 * @param color3 color of fourth segment
 * @param pixels the 2D array of CRGBs to store the LED data in
 * @details
 *  The characters are looked up in the patterns array, and the segment
 *  is filled with the corresponding LEDs. If the character and color
 *  are the same as the previous call, the segment is not updated to
 *  save processing time.
 */
void Display(char c0, char c1, char c2, char c3, CRGB color0, CRGB color1, CRGB color2, CRGB color3, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    CRGB colors[NUM_SEGS] = {color0, color1, color2, color3};
    char chars[NUM_SEGS] = {c0, c1, c2, c3};
    static char prev_chars[NUM_SEGS] = {' ', ' ', ' ', ' '};
    static CRGB prev_colors[NUM_SEGS] = {CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black};

    for (int i = 0; i < NUM_SEGS; i++)
    {
        if (chars[i] == prev_chars[i] && colors[i] == prev_colors[i])
        {
            continue;
        }
        else
        {
            prev_chars[i] = chars[i];
            prev_colors[i] = colors[i];
            for (int j = 1; j < NUM_LEDS_PER_SEG; j++)
            {
                if (patterns[CharToIndex(chars[i])][j])
                {
                    pixels[i][j] = colors[i];
                }
                else
                {
                    pixels[i][j] = CRGB::Black;
                }
            }
        }
    }
}

/**
 * @brief Display the time on the 4 segment display
 * @param time_struct a tm struct with the current time
 * @param color0 color of the first segment
 * @param color1 color of the second segment
 * @param color2 color of the third segment
 * @param color3 color of the fourth segment
 * @param pixels the 2D array of CRGBs to store the LED data in
 * @details
 *  The time is displayed in the format HHMM, with the first digit
 *  of the hour on the first segment, the second digit of the hour
 *  on the second segment, the first digit of the minute on the third
 *  segment, and the second digit of the minute on the fourth segment.
 *  The display is updated every 1 second to blink the last digit.
 */
void DisplayTime(tm &time_struct, CRGB color0, CRGB color1, CRGB color2, CRGB color3, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    static unsigned long timePast = 0;
    static bool blink = false;

    uint8_t hour[2], minute[2];
    hour[0] = time_struct.tm_hour / 10;
    hour[1] = time_struct.tm_hour % 10;
    minute[0] = time_struct.tm_min / 10;
    minute[1] = time_struct.tm_min % 10;

    if (millis() - timePast >= 1000)
    {
        blink = !blink;
        timePast = millis();
    }

    Display(hour[0] + '0', hour[1] + '0', minute[0] + '0', blink ? (minute[1] + '0') : ' ', color0, color1, color2, color3, pixels);
}

/**
 * @brief Display the temperature on the 4 segment display
 * @param temp the temperature as an integer (e.g. 25 for 25 degrees Celsius)
 * @param color0 color of the first LED segment
 * @param color1 color of the second LED segment
 * @param color2 color of the third LED segment
 * @param color3 color of the fourth LED segment
 * @param pixels the 2D array of CRGBs to store the LED data in
 * @details
 *  The temperature is displayed in the format XX°C, with the first digit
 *  of the temperature on the first segment, the second digit of the
 *  temperature on the second segment, the degree symbol on the third
 *  segment, and the letter 'C' on the fourth segment.
 *  The display is updated every 1 second to blink the third (°) digit.
 */
void DisplayTemperature(int temp, CRGB color0, CRGB color1, CRGB color2, CRGB color3, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    static unsigned long timePast = 0;
    static bool blink = false;

    int first_digit = temp / 10;
    int second_digit = temp % 10;

    if (millis() - timePast >= 1000)
    {
        blink = !blink;
        timePast = millis();
    }

    Display(first_digit + '0', second_digit + '0', blink ? '`' : ' ', 'C', color0, color1, color2, color3, pixels);
}

/**
 * @brief Set the first LED of each segment to the given colors
 * @param color0 color of the first LED of the first segment
 * @param color1 color of the first LED of the second segment
 * @param color2 color of the first LED of the third segment
 * @param color3 color of the first LED of the fourth segment
 * @param pixels the 2D array of CRGBs to store the LED data in
 * @details
 *  This function is used to set the first LED of each segment to
 *  a different color. This is useful for debugging purposes.
 */
void SetFirstPixels(CRGB color0, CRGB color1, CRGB color2, CRGB color3, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    pixels[0][0] = color0;
    pixels[1][0] = color1;
    pixels[2][0] = color2;
    pixels[3][0] = color3;
}

void UpdateDisplay(uint8_t brightness, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    static uint8_t prev_brightness = 0;
    if (brightness != prev_brightness)
    {
        prev_brightness = brightness;
        FastLED.setBrightness(brightness);
    }
    FastLED.show();
}
