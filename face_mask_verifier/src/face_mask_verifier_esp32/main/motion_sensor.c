#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_main.h"

#include "bluetooth_door.h"

static const char *TAG = "blink";

#define SENSOR_GPIO 27

static void configure_sensor(void)
{
    gpio_reset_pin(SENSOR_GPIO);
    gpio_set_direction(SENSOR_GPIO, GPIO_MODE_INPUT);
}

void trigger_camera(uint8_t val)
{
  send_message(val);
}

void app_motion_sensor_main(void)
{
    uint8_t state = 0;

    configure_sensor();

    while (1) {
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
