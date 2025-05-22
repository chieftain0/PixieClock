#pragma once

// modify Secrets.h to manage your credentials:
#include "Secrets.h"
// Otherwise:
// const bool isEAP = false;
// const char *SSID = "";
// const char *PASSWORD = "";
// const char *EAP_IDENTITY = ""; // Optional
// const char *EAP_USERNAME = ""; // Optional
// const char *NTP_SERVER1 = "";
// const char *NTP_SERVER2 = ""; // Optional
// const char *NTP_SERVER3 = ""; // Optional
// const char *OPENWEATHERMAP_API_KEY = "";

#define NUM_SEGS 4
#define NUM_LEDS_PER_SEG 23

#define SEG0_PIN GPIO_NUM_6
#define SEG1_PIN GPIO_NUM_7
#define SEG2_PIN GPIO_NUM_15
#define SEG3_PIN GPIO_NUM_16

#define SENSE_PIN_0 GPIO_NUM_18
#define SENSE_PIN_1 GPIO_NUM_8
#define SENSE_PIN_2 GPIO_NUM_9
#define SENSE_PIN_3 GPIO_NUM_10

#define DS18_PIN GPIO_NUM_11

#define BUZZER1_PIN GPIO_NUM_47
#define BUZZER2_PIN GPIO_NUM_48

#define BUTTON1_PIN GPIO_NUM_12
#define BUTTON2_PIN GPIO_NUM_13
#define BUTTON3_PIN GPIO_NUM_14
#define BUTTON4_PIN GPIO_NUM_21