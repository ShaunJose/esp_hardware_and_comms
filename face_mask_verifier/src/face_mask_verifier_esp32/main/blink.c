#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "blink";

static uint8_t s_led_state = 0;


static void configure_led(int pin_num)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(pin_num);
    gpio_set_direction(pin_num, GPIO_MODE_OUTPUT);
}

void app_led_main(int led_pin_num)
{

    configure_led(led_pin_num);

    // on and off
    gpio_set_level(led_pin_num, 1);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    gpio_set_level(led_pin_num, 0);

}
