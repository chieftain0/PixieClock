#include "config.h"

#include <FastLED.h>

// Character maps
static const bool patterns[37][NUM_LEDS_PER_SEG] = {
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
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // G
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // H
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // I
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // J
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // K
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // L
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // M
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // N
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // O
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // P
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Q
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // R
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // S
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // T
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // U
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // V
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // W
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // X
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Y
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Z
    {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0}, // °
};

/**
 * @brief Look up the index from patterns array
 * @param c character to look up
 * @return index in patterns array, or 36 (°) if not found
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
    else if (c == '°')
    {
        return 36;
    }
    else
    {
        return 36;
    }
}

/**
 * @brief Display 4 characters on 4 segments of LED segments
 * @param c0 character to display on first segment
 * @param c1 character to display on second segment
 * @param c2 character to display on third segment
 * @param c3 character to display on fourth segment
 * @param brightness overall brightness of the LEDs
 * @param color0 color of first segment
 * @param color1 color of second segment
 * @param color2 color of third segment
 * @param color3 color of fourth segment
 * @param pixels the 2D array of CRGBs to store the LED data in
 * @details
 *  The characters are looked up in the patterns array, and the segment
 *  is filled with the corresponding LEDs. The brightness and color
 *  are set accordingly.
 */
void Display(char c0, char c1, char c2, char c3, uint8_t brightness, CRGB color0, CRGB color1, CRGB color2, CRGB color3, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    FastLED.setBrightness(brightness);
    CRGB colors[NUM_SEGS] = {color0, color1, color2, color3};
    char chars[NUM_SEGS] = {c0, c1, c2, c3};

    for (int i = 0; i < NUM_SEGS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_SEG; j++)
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
    FastLED.show();
}

/**
 * @brief Display the time on the 4 segment display
 * @param time_struct a tm struct with the current time
 * @param brightness overall brightness of the LEDs
 * @param color color of the LEDs
 * @param pixels the 2D array of CRGBs to store the LED data in
 * @details
 *  The time is displayed in the format HHMM, with the first digit
 *  of the hour on the first segment, the second digit of the hour
 *  on the second segment, the first digit of the minute on the third
 *  segment, and the second digit of the minute on the fourth segment.
 *  The display is updated every 500 milliseconds to blink the last digit.
 */
void DisplayTime(tm &time_struct, uint8_t brightness, CRGB color, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    static unsigned long timePast = 0;

    uint8_t hour[2], minute[2];
    hour[0] = time_struct.tm_hour / 10;
    hour[1] = time_struct.tm_hour % 10;
    minute[0] = time_struct.tm_min / 10;
    minute[1] = time_struct.tm_min % 10;

    if (millis() - timePast > 500)
    {
        Display(hour[0] + '0', hour[1] + '0', minute[0] + '0', minute[1] + '0', brightness, color, color, color, CRGB::Black, pixels);
        timePast = millis();
    }
    else
    {
        Display(hour[0] + '0', hour[1] + '0', minute[0] + '0', minute[1] + '0', brightness, color, color, color, color, pixels);
    }
}

/**
 * @brief Display the temperature on the 4 segment display
 * @param temp the temperature as an integer (e.g. 25 for 25 degrees Celsius)
 * @param brightness overall brightness of the LEDs
 * @param color color of the LEDs
 * @param pixels the 2D array of CRGBs to store the LED data in
 * @details
 *  The temperature is displayed in the format XX°C, with the first digit
 *  of the temperature on the first segment, the second digit of the
 *  temperature on the second segment, the degree symbol on the third
 *  segment, and the letter 'C' on the fourth segment.
 *  The display is updated every 500 milliseconds to blink the third (°) digit.
 */
void DisplayTemperature(int temp, uint8_t brightness, CRGB color, CRGB (&pixels)[NUM_SEGS][NUM_LEDS_PER_SEG])
{
    static unsigned long timePast = 0;

    int first_digit = temp / 10;
    int second_digit = temp % 10;

    if (millis() - timePast > 500)
    {
        Display(first_digit + '0', second_digit + '0', '°', 'C', brightness, color, color, CRGB::Black, color, pixels);
        timePast = millis();
    }
    else
    {
        Display(first_digit + '0', second_digit + '0', '°', 'C', brightness, color, color, color, color, pixels);
    }
}
