idf_component_register(SRCS "esp32-telegram-bot-api-interface.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_wifi" "nvs_flash" "esp-tls" "esp_http_client")
