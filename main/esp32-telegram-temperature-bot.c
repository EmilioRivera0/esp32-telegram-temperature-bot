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
    while (1){
        stDht11Reading = DHT11_read();
    
        if(DHT11_OK == stDht11Reading.status)
        {
            printf("Temperature: %d\n\n\n", stDht11Reading.temperature);
        } 
        else
        {
            printf("Error\n\n\n");
        }
        //fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
