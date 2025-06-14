# PixieClock

## Installation

### Prerequisites

- #### Hardware

  - [PCBs](hardware/gerber)
  - [ESP32-S3-WROOM-1U](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf)

- #### Software

  - [Arduino IDE](https://www.arduino.cc/en/software)
  - [Arduino Core for ESP32](https://github.com/espressif/arduino-esp32.git)
  - [Git](https://git-scm.com/downloads) (optional)
  
### Steps

1. Install Arduino IDE and configure Arduino Core for ESP32 by adding the following URL to the "Additional Board Manager URLs" in the IDE preferences:

    ```bash
    https://espressif.github.io/arduino-esp32/package_esp32_index.json
    ```

    or follow the instructions by [Espressif](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
2. Clone or download the PixieClock repository:

    ```bash
    git clone https://github.com/chieftain0/PixieClock.git
    ```

3. Install the required libraries using Arduino IDE's library manager.

4. Configure the board parameters according to your ESP32-S3-WROOM-1U. See the [datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf) for details.

5. Configure the confidential information (WiFi credentials, OpenWeatherMap API key, NTP servers) either in [config.h](include/config.h) or by creating a new file called [Secrets.h](include/) in the [include](include/) folder.

6. Upload the code and enjoy!

## Credits

- [FastLED](https://github.com/FastLED/FastLED.git)
- [GyverLibs](https://github.com/GyverLibs)
- [ArduinoJson](https://arduinojson.org/)
