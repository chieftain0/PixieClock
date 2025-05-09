#pragma once

#include "config.h"

#include <time.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <GyverDS18.h>
GyverDS18Single DS18(DS18_PIN);

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

bool CallOpenWeatherMap(const char *city, const char *apiKey, double &temp, int &timezoneOffset)
{ // TODO replace spaces in city name with %20
    char URL[256];
    snprintf(URL,
             sizeof(URL),
             "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric",
             city, apiKey);

    HTTPClient http;
    http.begin(URL);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, http.getStream());

        http.end();

        if (error)
        {
            return false;
        }
        timezoneOffset = doc["timezone"];
        temp = doc["main"]["temp"];
        return true;
    }
    else
    {
        http.end();
        return false;
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
bool GetCity(char *city, size_t sizeArr)
{
    HTTPClient http;
    http.begin("http://ip-api.com/json/");
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, http.getStream());

        http.end();

        if (error)
        {
            return false;
        }
        
        const char *cityName = doc["city"];
        snprintf(city, sizeArr, "%s", cityName);
        city[sizeArr - 1] = '\0';
        return true;
    }
    else
    {
        http.end();
        return false;
    }
}
