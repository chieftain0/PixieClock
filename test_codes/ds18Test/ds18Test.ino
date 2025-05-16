#include "config.h"

#include <GyverDS18.h>
GyverDS18Single DS18(DS18_PIN);

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    DS18.requestTemp();
    delay(1000);
    DS18.readTemp();
    Serial.println(DS18.getTemp());
}