#include "config.h"

const int PWM_freq = 4000; // 4kHz tone

void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER1_PIN, OUTPUT);
    pinMode(BUZZER2_PIN, OUTPUT);

    analogWriteFrequency(BUZZER1_PIN, PWM_freq);
    analogWriteFrequency(BUZZER2_PIN, PWM_freq);
}

void loop()
{
    analogWrite(BUZZER1_PIN, 128);
    analogWrite(BUZZER2_PIN, 0);
    delay(2000);

    analogWrite(BUZZER1_PIN, 0);
    analogWrite(BUZZER2_PIN, 0);
    delay(2000);

    analogWrite(BUZZER1_PIN, 0);
    analogWrite(BUZZER2_PIN, 128);
    delay(2000);

    analogWrite(BUZZER1_PIN, 0);
    analogWrite(BUZZER2_PIN, 0);
    delay(2000);
}
