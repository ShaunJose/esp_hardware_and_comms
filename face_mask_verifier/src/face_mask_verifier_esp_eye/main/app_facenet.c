#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_facenet.h"
#include "sdkconfig.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "bluetooth_camera.h"

#include "esp_crt_bundle.h"

// set these values by updating Example Configuration after running idf.py menuconfig
#define EXAMPLE_WIFI_SSID CONFIG_EXAMPLE_WIFI_SSID
#define EXAMPLE_EAP_METHOD CONFIG_EXAMPLE_EAP_METHOD

#define EXAMPLE_EAP_ID CONFIG_EXAMPLE_EAP_ID
#define EXAMPLE_EAP_USERNAME CONFIG_EXAMPLE_EAP_USERNAME
#define EXAMPLE_EAP_PASSWORD CONFIG_EXAMPLE_EAP_PASSWORD

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* esp netif object representing the WIFI station */
static esp_netif_t *sta_netif = NULL;

static const char *WIFI_TAG = "wifi";
static const char *HTTP_TAG = "http";
bool wifi_connected = false;

// response code to symbolise what i get from the API
uint8_t responseCode = -1;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;


static const char *TAG = "app_process";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        wifi_connected = true;
    }
}

static void initialise_wifi(void)
{
   ESP_ERROR_CHECK(esp_netif_init());
   wifi_event_group = xEventGroupCreate();
   ESP_ERROR_CHECK(esp_event_loop_create_default());
   sta_netif = esp_netif_create_default_wifi_sta();
   assert(sta_netif);

   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
   ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
   ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL) );
   ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL) );
   ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
   wifi_config_t wifi_config = {
       .sta = {
           .ssid = "Oneplus 8 pro",
           .password = "josejose",
       },
   };
   ESP_LOGI(WIFI_TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
   ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
   ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
   ESP_ERROR_CHECK( esp_wifi_start() );
   ESP_LOGI(HTTP_TAG, "Wifi established");
}

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(HTTP_TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(HTTP_TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(HTTP_TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(HTTP_TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(HTTP_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
              //set response code based on data received
              char *response = (char*)evt->data;
              char *notFound = "Face not";
              char *noMask = "false";
              char *mask = "true";
              ESP_LOGI(HTTP_TAG, "HERE NOW: %s", response);
              if(strstr(response, mask) != NULL)
                responseCode = 5;
              else if(strstr(response, noMask) != NULL)
                responseCode = 6;
              else if(strstr(response, notFound) != NULL)
                responseCode = 7;
            }
            send_message(responseCode);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(HTTP_TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(HTTP_TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

static void hit_api(char* url, unsigned char *data_ptr, int len)
{

    ESP_LOGI(HTTP_TAG, "Starting api call");
    esp_http_client_config_t config = {
   .url = url,
   .event_handler = _http_event_handle,
   .skip_cert_common_name_check = false,
   .method = HTTP_METHOD_POST,
   .timeout_ms = 100000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_post_field(client, ((char *)data_ptr), len);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
       ESP_LOGI(HTTP_TAG, "Status = %d, content_length = %d",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
    }
    esp_http_client_cleanup(client);
}

void wifi_app(unsigned char *image_ptr, int len)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    while(!wifi_connected)
    {
      vTaskDelay( xDelay );
    }
    hit_api("https://facemaskcheck.azurewebsites.net/api/Facemask", image_ptr, len);

}

mtmn_config_t init_config()
{
    mtmn_config_t mtmn_config = {0};
    mtmn_config.type = FAST;
    mtmn_config.min_face = 80;
    mtmn_config.pyramid = 0.707;
    mtmn_config.pyramid_times = 4;
    mtmn_config.p_threshold.score = 0.6;
    mtmn_config.p_threshold.nms = 0.7;
    mtmn_config.p_threshold.candidate_number = 20;
    mtmn_config.r_threshold.score = 0.7;
    mtmn_config.r_threshold.nms = 0.7;
    mtmn_config.r_threshold.candidate_number = 10;
    mtmn_config.o_threshold.score = 0.7;
    mtmn_config.o_threshold.nms = 0.7;
    mtmn_config.o_threshold.candidate_number = 1;

    return mtmn_config;
}

void error(const char *msg) { perror(msg); exit(0); }


void task_process (void *arg)
{
    size_t frame_num = 0;
    dl_matrix3du_t *image_matrix = NULL;
    camera_fb_t *fb = NULL;

    /* 1. Load configuration for detection */
    mtmn_config_t mtmn_config = init_config();

    int64_t start_time = esp_timer_get_time();
    /* 2. Get one image with camera */
    fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera capture failed");
    }
    int64_t fb_get_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Get one frame in %lld ms.", (fb_get_time - start_time) / 1000);

    /* 3. Allocate image matrix to store RGB data */
    image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

    /* 4. Transform image to RGB */
    uint32_t res = fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);
    if (true != res)
    {
        ESP_LOGE(TAG, "fmt2rgb888 failed, fb: %d", fb->len);
        dl_matrix3du_free(image_matrix);
    }

    esp_camera_fb_return(fb);

    /* 5. Do face detection */ //->TODO: REMOVE NOT NEEDED
    box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);
    ESP_LOGI(TAG, "Detection time consumption: %lldms", (esp_timer_get_time() - fb_get_time) / 1000);

    if (net_boxes) //wow face detected -> REMOVE
    {
        frame_num++;
        dl_lib_free(net_boxes->score);
        dl_lib_free(net_boxes->box);
        dl_lib_free(net_boxes->landmark);
        dl_lib_free(net_boxes);
    }

    ESP_LOGI(TAG, "Sending image to api function");
    wifi_app(fb->buf, fb->len);

    dl_matrix3du_free(image_matrix);


}

void app_facenet_main()
{
    xTaskCreatePinnedToCore(task_process, "process", 4 * 1024, NULL, 5, NULL, 1);
}
