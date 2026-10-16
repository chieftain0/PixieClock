#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "config.h"

static bool displayDirty = false;

/**
 * @brief Namespace for color constants used in the display
 * @details
 *  This namespace contains color constants for different types of
 *  messages and information displayed on the LED segments. Each
 *  constant is defined as a CRGB color value.
 */
namespace Colors
{
    // Message
    const uint32_t MessageChar0 = CRGB::Fuchsia;
    const uint32_t MessageChar1 = CRGB::Crimson;
    const uint32_t MessageChar2 = CRGB::Fuchsia;
    const uint32_t MessageChar3 = CRGB::Crimson;

    // Time
    const uint32_t TimeHourTens = CRGB::Red;
    const uint32_t TimeHourOnes = CRGB::Orange;
    const uint32_t TimeMinTens = CRGB::Red;
    const uint32_t TimeMinOnes = CRGB::Orange;

    // Outdoor Temperature
    const uint32_t OutdoorTempTens = CRGB::Blue;
    const uint32_t OutdoorTempOnes = CRGB::Gold;
    const uint32_t OutdoorTempDeg = CRGB::Blue;
    const uint32_t OutdoorTempCelsius = CRGB::Gold;

    // Indoor
    const uint32_t IndoorTempTens = CRGB::Green;
    const uint32_t IndoorTempOnes = CRGB::White;
    const uint32_t IndoorTempDeg = CRGB::Green;
    const uint32_t IndoorTempCelsius = CRGB::White;
}

// Character maps
const uint32_t patterns[39] = {
    0b00001111101101111101100, // 0
    0b01000000000011111110000, // 1
    0b00111001101110001101100, // 2
    0b00111000101101111101100, // 3
    0b00111110000011111110010, // 4
    0b00111110011101100011110, // 5
    0b00111111101101100011100, // 6
    0b00000000000011111111110, // 7
    0b00111101101101101101100, // 8
    0b00111100101101111101100, // 9
    0b00111111110011111101100, // A - 10
    0b00111111111101101101110, // B - 11
    0b00001111101101000101100, // C - 12
    0b00001111111101111101110, // D - 13
    0b00111111111110010011110, // E - 14
    0b00111111110000000011110, // F - 15
    0b00101111101101110011100, // G - 16
    0b00111111110011111110010, // H - 17
    0b11110000011110000011110, // I - 18
    0b00000001101101111111110, // J - 19
    0b11011111110110000011010, // K - 20
    0b00001111111110000000010, // L - 21
    0b01001111110011111110010, // M - 22
    0b01101111110011111110010, // N - 23
    0b00001111101101111101100, // O - 24
    0b00111111110000001101110, // P - 25
    0b00111100000011111111100, // Q - 26
    0b10111111110110001101110, // R - 27
    0b00111100011101100011100, // S - 28
    0b11110000000000000011110, // T - 29
    0b00001111101101111110010, // U - 30
    0b10001110000000011110010, // V - 31
    0b11111111101101111110010, // W - 32
    0b00111101110011101110010, // X - 33
    0b10111100001100001110010, // Y - 34
    0b00110001111110001111110, // Z - 35
    0b00111100000000001101100, // ° - 36
    0b00110010000000010000000, // - - 37
    0b00000000000000000000000, // SPACE - 38
};

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
    else if (c == '-') // -
    {
        return 37;
    }
    else if (c == ' ') // SPACE
    {
        return 38;
    }
    else
    {
        return 15; // F
    }
}

/**
 * @brief Get the current blink state
 * @return The current blink state (true or false)
 * @details
 *  This function toggles the blink state every BLINK_PERIOD milliseconds.
 *  The blink state is shared by DisplayTime() and DisplayTemperature().
 *  The function is non-blocking and uses millis() to track elapsed time.
 */
bool GetBlinkState()
{
    static uint32_t previousMillis = 0;
    static bool blink = false;

    uint32_t now = millis();
    if (now - previousMillis >= BLINK_PERIOD)
    {
        previousMillis = now;
        blink = !blink;
    }

    return blink;
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
            displayDirty = true;
            prev_chars[i] = chars[i];
            prev_colors[i] = colors[i];
            for (int j = 1; j < NUM_LEDS_PER_SEG; j++) // Start at 1 to skip the first LED
            {
                uint32_t bit_select = 1UL << j;
                if (patterns[CharToIndex(chars[i])] & bit_select != 0)
                {
                    pixels[i][j] = colors[i];
                }
                else
                {
                    pixels[i][j] = CRGB::Black; // Set the LED to black
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
    bool blink = GetBlinkState();

    uint8_t hour[2], minute[2];
    hour[0] = time_struct.tm_hour / 10;
    hour[1] = time_struct.tm_hour % 10;
    minute[0] = time_struct.tm_min / 10;
    minute[1] = time_struct.tm_min % 10;

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
    bool blink = GetBlinkState();
    bool negative_temp = false;

    if (temp > 99 || temp < -99)
    {
        Display('E', 'R', 'R', '`', color0, color1, color2, color3, pixels);
        return; // Are you okay?
    }

    if (temp < 0)
    {
        temp = -temp;
        negative_temp = true;
    }

    int first_digit = temp / 10;
    int second_digit = temp % 10;

    if (negative_temp == false)
    {
        Display(first_digit + '0', second_digit + '0', blink ? '`' : ' ', 'C', color0, color1, color2, color3, pixels);
    }
    else
    {
        Display('-', first_digit + '0', second_digit + '0', blink ? '`' : 'C', color0, color1, color2, color3, pixels);
    }
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
    displayDirty = true;
    pixels[0][0] = color0;
    pixels[1][0] = color1;
    pixels[2][0] = color2;
    pixels[3][0] = color3;
}

void UpdateDisplay(uint8_t brightness, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    static uint8_t prev_brightness = 0;
    if (brightness != prev_brightness || displayDirty == true)
    {
        displayDirty = false;
        prev_brightness = brightness;
        FastLED.setBrightness(brightness);
        FastLED.show();
    }
}
