#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "soc/soc.h" // temp ---------------------------------
#include "soc/rtc_cntl_reg.h" // temp ---------------------------------

#define SSID "MEGACABLE-2.4G-50AB"
#define PASSWD "pj8uJnwRYF"
#define ESP_MAXIMUM_RETRY  10

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

void connect_wifi(void);
