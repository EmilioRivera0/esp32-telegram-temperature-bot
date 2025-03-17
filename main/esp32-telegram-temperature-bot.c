#include <stdio.h>
#include "esp_system.h"
#include "dht11.h"
#include "esp32-telegram-bot-api-interface.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    static struct dht11_reading stDht11Reading;
    DHT11_init(GPIO_NUM_25);

    connect_wifi();
    init_http_client();

    while (1){
        get_telegram_command();
        if (get_response_data()){
            while (true){
                stDht11Reading = DHT11_read();
                if(DHT11_OK == stDht11Reading.status)
                {
                    answer_command(stDht11Reading.temperature, stDht11Reading.humidity);
                    break;
                } 
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
