#pragma once

#include "config.h"

#include <time.h>

#include <Arduino.h>
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
 * @brief Fetch the current outdoor temperature, sunrise, and sunset times
 * @param city the city name
 * @param countryCode the country code of the city
 * @param apiKey the API key for OpenWeatherMap
 * @param OutdoorTemp[out] the current outdoor temperature in degrees Celsius, or 99 if there was an error
 * @param SunriseEpoch[out] sunrise time as a Unix epoch timestamp in current timezone, or 0 if there was an error
 * @param SunsetEpoch[out] sunset time as a Unix epoch timestamp in current timezone, or 0 if there was an error
 * @return 0 on success, 1 on error
 * @details
 *  This function sends a GET request to OpenWeatherMap's weather API and
 *  extracts the current temperature, sunrise, and sunset from the JSON
 *  response. If there was an error with the request or if the JSON could
 *  not be parsed, OutdoorTemp is set to 99 and the epochs to 0.
 */
uint8_t FetchOutdoorData(const char *city, const char *countryCode, const char *apiKey, double &OutdoorTemp, uint32_t &SunriseEpoch, uint32_t &SunsetEpoch)
{
    if (city == NULL || apiKey == NULL)
    {
        OutdoorTemp = 99;
        SunriseEpoch = 0;
        SunsetEpoch = 0;
        return 1;
    }

    char *cityEncoded = URLencodeSpaces(city);
    if (cityEncoded == NULL)
    {
        OutdoorTemp = 99;
        SunriseEpoch = 0;
        SunsetEpoch = 0;
        return 1;
    }

    char URL[256];
    snprintf(URL,
             sizeof(URL),
             "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&appid=%s&units=metric",
             cityEncoded, countryCode, apiKey);

    free(cityEncoded);

    HTTPClient http;
    http.begin(URL);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        DynamicJsonDocument doc(JSON_BUFFER_SIZE);
        DeserializationError error = deserializeJson(doc, http.getStream());

        http.end();

        if (error)
        {
            OutdoorTemp = 99;
            SunriseEpoch = 0;
            SunsetEpoch = 0;
            return 1;
        }
        OutdoorTemp = (double)doc["main"]["temp"];
        SunriseEpoch = (uint32_t)doc["sys"]["sunrise"] + (int32_t)doc["timezone"];
        SunsetEpoch = (uint32_t)doc["sys"]["sunset"] + (int32_t)doc["timezone"];
        return 0;
    }
    else
    {
        http.end();
        OutdoorTemp = 99;
        SunriseEpoch = 0;
        SunsetEpoch = 0;
        return 1;
    }
}

/**
 * @brief Retrieve the timezone offset and city information based on IP address
 * @param city buffer to store the name of the city
 * @param cityArrSize size of the city buffer
 * @param countryCode buffer to store the country code
 * @param countryCodeArrSize size of the country code buffer
 * @return the timezone offset in seconds or 0 if there was an error
 * @details
 *  This function makes a request to ip-api.com to obtain the country code,
 *  city name, and timezone offset of the user based on their IP address.
 *  If the request is successful, the city and country code are copied
 *  into the provided buffers, and the timezone offset is returned.
 *  If there is an error, the function returns 0.
 */

int32_t GetTzOffsetAndCity(char *city, size_t cityArrSize, char *countryCode, size_t countryCodeArrSize)
{
    HTTPClient http;
    http.begin("http://ip-api.com/json/?fields=countryCode,city,offset");
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        DynamicJsonDocument doc(JSON_BUFFER_SIZE);
        DeserializationError error = deserializeJson(doc, http.getStream());

        http.end();

        if (error)
        {
            return 0;
        }

        const char *countryCodeName = doc["countryCode"];
        const char *cityName = doc["city"];

        snprintf(city, cityArrSize, "%s", cityName);
        snprintf(countryCode, countryCodeArrSize, "%s", countryCodeName);

        int TzOffset = doc["offset"];
        return TzOffset;
    }
    else
    {
        http.end();
        return 0;
    }
}
