# ESP32 Telegram Temperature Bot
C source code for programming an ESP32 board to read the temperature and humidity using the DHT11 sensor whenever a ‘/status’ command is sent to a Telegram Bot linked to the ESP32. The code from [ESP-IDF WIFI Examples](https://github.com/espressif/esp-idf/blob/v5.2.5/examples/wifi/getting_started/station/main/station_example_main.c) and [ESP-IDF HTTP Examples](https://github.com/espressif/esp-idf/blob/v5.0/examples/protocols/esp_http_client/main/esp_http_client_example.c) were used to connect to the Wifi and use the HTTP API on this project.

## Dependencies
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html) installed
- [DHT11 C Library](https://github.com/kuldeepaher01/esp32-DHT11) developed by [kuldeepaher01](https://github.com/kuldeepaher01) (cloned in components/)

## Example
[![Watch the video](https://img.youtube.com/vi/zLXlb8f3CNQ/maxresdefault.jpg)](https://www.youtube.com/watch?v=zLXlb8f3CNQ)

## Author
- @[EmilioRivera0](https://github.com/EmilioRivera0)
