// C Library
#include <string.h>
#include <stdlib.h>

// General ESP IDF headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h" // temp ---------------------------------

#include "nvs_flash.h"

// Wifi headers
#include "freertos/event_groups.h"
#include "esp_wifi.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// HTTP client headers
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"

#include "soc/soc.h" // temp ---------------------------------
#include "soc/rtc_cntl_reg.h" // temp ---------------------------------

// Wifi macros and function prototypes
#define ESP_MAXIMUM_RETRY  10
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

void connect_wifi(void);

// HTTP macros and function prototypes
#define ENDPOINT_LENGTH 500
#define RESPONSE_BUFFER 1024

void get_telegram_command(void);
