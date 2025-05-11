#pragma once

#include "config.h"

#include <time.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <GyverDS18.h>
GyverDS18Single DS18(DS18_PIN);

#define JSON_BUFFER_SIZE 2048

/**
 * @brief      Encodes spaces in a given string according to the URL
 *             encoding standard, replacing them with %20.
 * @param str  The string to encode.
 * @return     The encoded string.
 * @warning    This function allocates memory
 *             that must be freed when finished with the string. Failure
 *             to do so will result in a memory leak.
 */
char *URLencodeSpaces(const char *str)
{
    size_t len = strlen(str);
    size_t spaceCount = 0;

    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == ' ')
        {
            spaceCount++;
        }
    }

    size_t newLen = len + spaceCount * 2;
    char *encoded = (char *)malloc(newLen + 1);

    if (!encoded)
    {
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == ' ')
        {
            encoded[j++] = '%';
            encoded[j++] = '2';
            encoded[j++] = '0';
        }
        else
        {
            encoded[j++] = str[i];
        }
    }
    encoded[j] = '\0';
    return encoded;
}

/**
 * @brief Gets the current outdoor temperature in the given city
 * @param city the city name
 * @param apiKey the OpenWeatherMap API key
 * @return the current temperature in degrees Celsius, or 99 if there was an error
 * @details
 *  This function sends a GET request to the OpenWeatherMap API, parsing the
 *  JSON response for the current temperature in the given city. If the
 *  request fails or the JSON could not be parsed, the function returns 99.
 */
double GetOutdoorTemp(const char *city, const char *apiKey)
{
    if (city == NULL || apiKey == NULL)
    {
        return 99;
    }

    char *cityEncoded = URLencodeSpaces(city);
    if (cityEncoded == NULL)
    {
        return 99;
    }

    char URL[256];
    snprintf(URL,
             sizeof(URL),
             "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric",
             cityEncoded, apiKey);

    free(cityEncoded);

    HTTPClient http;
    http.begin(URL);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        DynamicJsonDocument doc(JSON_BUFFER_SIZE);
        DeserializationError error = deserializeJson(doc, http.getStream());

        http.end();

        if (error)
        {
            return 99;
        }
        double temp = doc["main"]["temp"];
        return temp;
    }
    else
    {
        http.end();
        return 99;
    }
}

/**
 * @brief Get the city name of the user's location
 * @return the city name as a const char*
 * @details
 *  This function sends a GET request to the IP-API.com API and
 *  extracts the city name from the JSON response. If there was an
 *  error with the request or if the JSON could not be parsed, the
 *  function returns "London".
 */
int GetTzOffsetAndCity(char *city, size_t sizeArr)
{
    HTTPClient http;
    http.begin("http://ip-api.com/json/?fields=country,city,offset");
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        DynamicJsonDocument doc(JSON_BUFFER_SIZE);
        DeserializationError error = deserializeJson(doc, http.getStream());

        http.end();

        if (error)
        {
            return 0;
        }

        const char *cityName = doc["city"];
        snprintf(city, sizeArr, "%s", cityName);
        city[sizeArr - 1] = '\0';

        int TzOffset = doc["offset"];
        return TzOffset;
    }
    else
    {
        http.end();
        return 0;
    }
}
