#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
// #include "led_strip.h"
#include "sdkconfig.h"

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_bt.h"
// #include "bta_api.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_main.h"

#include "bluetooth_door.h"

static const char *TAG = "blink";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define SENSOR_GPIO 27

// static uint8_t s_led_state = 0;

// #ifdef CONFIG_BLINK_LED_RMT
// static led_strip_t *pStrip_a;
//
// static void blink_led(void)
// {
//     /* If the addressable LED is enabled */
//     if (s_led_state) {
//         /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
//         pStrip_a->set_pixel(pStrip_a, 0, 16, 16, 16);
//         /* Refresh the strip to send data */
//         pStrip_a->refresh(pStrip_a, 100);
//     } else {
//         /* Set all LED off to clear all pixels */
//         pStrip_a->clear(pStrip_a, 50);
//     }
// }
//
// static void configure_led(void)
// {
//     ESP_LOGI(TAG, "Example configured to blink addressable LED!");
//     /* LED strip initialization with the GPIO and pixels number*/
//     pStrip_a = led_strip_init(CONFIG_BLINK_LED_RMT_CHANNEL, SENSOR_GPIO, 1);
//     /* Set all LED off to clear all pixels */
//     pStrip_a->clear(pStrip_a, 50);
// }

// #elif CONFIG_BLINK_LED_GPIO

// static void blink_sensor(void)
// {
//     /* Set the GPIO level according to the state (LOW or HIGH)*/
//     gpio_set_level(SENSOR_GPIO, s_led_state);
// }

static void configure_sensor(void)
{
    ESP_LOGI(TAG, "Example configured to motion sensor!");
    gpio_reset_pin(SENSOR_GPIO);
    gpio_set_direction(SENSOR_GPIO, GPIO_MODE_INPUT);
}

// #endif

void trigger_camera(uint8_t val)
{
  // const TickType_t xDelay = 30000 / portTICK_PERIOD_MS;
  // vTaskDelay( xDelay );
  // uint8_t temp_val = 2;
  // ESP_LOGI(GATTS_TABLE_TAG, "DELAY DONE");
  // esp_ble_gatts_send_indicate(gatts_if_global, conn_id_global, handle_global, 1, &temp_val, false);
  send_message(val);
}

void app_motion_sensor_main(void)
{
    uint8_t state = 0;

    /* Configure the peripheral according to the LED type */
    configure_sensor();

    while (1) {
        // ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");

        if(gpio_get_level(SENSOR_GPIO) == 1 && state == 0)
        {
          ESP_LOGI(TAG, "Motion detected");
          trigger_camera(2);
          state = 1;
        }
        if(gpio_get_level(SENSOR_GPIO) == 0 && state == 1)
        {
          ESP_LOGI(TAG, "Motion stopped");
          state = 0;
        }
    }
}
