#include "esp32-telegram-bot-api-interface.h"
#include "keys.h"

// Wifi Functions Implementation ---------------------------------------------------------------------------
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
// debugging variable
//static const char *TAG = "---------------------------------- wifi station";

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSWD,
        },
    };

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    // debugging code
    /*EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to ap SSID:%s", SSID);
    }*/
}

void connect_wifi(void)
{
    // Disable Brownout Detector
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // temp ---------------------------------
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ret = nvs_flash_init();
    }
    wifi_init_sta();
}

// HTTP Functions Implementation ---------------------------------------------------------------------------
// static variables that will be used multiple times during the programs execution
static char endpoint[ENDPOINT_LENGTH];
static char r_buffer[RESPONSE_BUFFER] = "\0";
static unsigned long long int ci = 0, ui = 0;
static char command[COMMAND_MAX_LENGTH];
static esp_http_client_handle_t client = NULL;
// debugging variable
//static const char *TAG_HTTP = "---------------------------------- HTTP";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;
    static int output_len;
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                    // r_buffer contains response body
                    memcpy(r_buffer, output_buffer, RESPONSE_BUFFER);
                }
                output_len += evt->data_len;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            break;
    }
    return ESP_OK;
}

/*
    Initialize client handle
    It will be used for GET and POST requests in the entire lifetime of the program
*/
void init_http_client(void)
{
    esp_http_client_config_t config = {
        .url = URL,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    client = esp_http_client_init(&config);
}

/*
    Make a GET request to Telegram API asking for any pending updates
*/
void get_telegram_command(void)
{
    // initialize endpoint
    endpoint[0] = '\0';
    strcat(endpoint, URL);
    strcat(endpoint, GET_COMMANDS_ENDPOINT);

    // modify client handle
    esp_http_client_set_url(client, endpoint);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    
    // perform GET request
    esp_err_t err = esp_http_client_perform(client);

    /*if (err == ESP_OK) {
        // debugging code
        ESP_LOGI(TAG_HTTP, "HTTPS Status = %d, content_length = %lld", esp_http_client_get_status_code(client), esp_http_client_get_content_length(client));
        printf("\n\n%s\n\n", r_buffer);
    } else {
        esp_restart();
    }*/
    if (err != ESP_OK)
        esp_restart();
}

/*
    Process response data to obtain update_id, chat_id and command/text
    Return true if request body contains an update
    Return false if request body is empty
*/
bool get_response_data(void){
    // function local methods
    short str_len = 0;
    char temp_id[ID_MAX_LENGTH];
    char* ps = NULL, *pe = NULL;

    // get update_id
    ps = strstr(r_buffer, "update_id");
    if (ps == NULL)
        return false;
    ps = strchr(ps, ':');
    pe = strchr(ps, ',');
    str_len = ((int)pe - (int)ps) - 1;
    ps++;
    strncpy(temp_id, ps, str_len);
    ui = strtoull(temp_id, NULL, 10);

    // get chat_id
    ps = strstr(r_buffer, "chat");
    ps = strstr(ps, "id");
    ps = strchr(ps, ':');
    pe = strchr(ps, ',');
    str_len = ((int)pe - (int)ps) - 1;
    ps++;
    strncpy(temp_id, ps, str_len);
    ci = strtoull(temp_id, NULL, 10);

    // get text/command
    ps = strstr(r_buffer, "text");
    ps = strchr(ps, ':');
    ps = strchr(ps, '"');
    pe = strchr(ps + 1, '"');
    str_len = ((int)pe - (int)ps) - 1;
    ps++;
    strncpy(command, ps, str_len);

    return true;
}

/*
    Answer the /status command by sending to the user the temperature and humidity, specified in the parameters
*/
void answer_command(int temperature, int humidity)
{
    // function local methods
    char query[QUERY_LENGTH];
    char post_data[POST_DATA_BUFFER];
    esp_err_t err;
    
    // answer /status command only
    if (!strcmp(command, "/status")){
        endpoint[0] = '\0';
        strcat(endpoint, URL);
        strcat(endpoint, ANSWER_COMMANDS_ENDPOINT);

        sprintf(post_data, "{\"text\": \"Temperature: %d°C Humidity: %d%%\", \"chat_id\": %llu}", temperature, humidity, ci);
    
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, post_data, strlen(post_data));
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_url(client, endpoint);
    
        err = esp_http_client_perform(client);
    
        esp_http_client_delete_header(client, "Content-Type");

        if (err != ESP_OK)
            esp_restart();
    }

    // remove the latet command from the server
    endpoint[0] = '\0';
    strcat(endpoint, URL);
    strcat(endpoint, GET_COMMANDS_ENDPOINT);
    sprintf(query, "?offset=%llu", ui + 1);
    strcat(endpoint, query);

    esp_http_client_set_url(client, endpoint);
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    err = esp_http_client_perform(client);

    if (err != ESP_OK)
        esp_restart();
}
