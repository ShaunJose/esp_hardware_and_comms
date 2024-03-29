// Source: official esp-who repository: https://github.com/espressif/esp-who

#ifndef _APP_CAMERA_H_
#define _APP_CAMERA_H_

#include "esp_log.h"
#include "esp_system.h"
#include "esp_camera.h"

#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG

#define CAMERA_FRAME_SIZE FRAMESIZE_QVGA

#if CONFIG_CAMERA_MODEL_WROVER_KIT
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

#elif CONFIG_CAMERA_MODEL_ESP_EYE
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    4
#define SIOD_GPIO_NUM    18
#define SIOC_GPIO_NUM    23

#define Y9_GPIO_NUM      36
#define Y8_GPIO_NUM      37
#define Y7_GPIO_NUM      38
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      35
#define Y4_GPIO_NUM      14
#define Y3_GPIO_NUM      13
#define Y2_GPIO_NUM      34
#define VSYNC_GPIO_NUM   5
#define HREF_GPIO_NUM    27
#define PCLK_GPIO_NUM    25

#elif CONFIG_CAMERA_MODEL_M5STACK_PSRAM
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    15
#define XCLK_GPIO_NUM     27
#define SIOD_GPIO_NUM     25
#define SIOC_GPIO_NUM     23

#define Y9_GPIO_NUM       19
#define Y8_GPIO_NUM       36
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       39
#define Y5_GPIO_NUM        5
#define Y4_GPIO_NUM       34
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       32
#define VSYNC_GPIO_NUM    22
#define HREF_GPIO_NUM     26
#define PCLK_GPIO_NUM     21

#elif CONFIG_CAMERA_MODEL_M5STACK_WIDE
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    15
#define XCLK_GPIO_NUM     27
#define SIOD_GPIO_NUM     22
#define SIOC_GPIO_NUM     23

#define Y9_GPIO_NUM       19
#define Y8_GPIO_NUM       36
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       39
#define Y5_GPIO_NUM        5
#define Y4_GPIO_NUM       34
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       32
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     26
#define PCLK_GPIO_NUM     21

#elif CONFIG_CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


#elif CONFIG_CAMERA_MODEL_CUSTOM
#define PWDN_GPIO_NUM    CONFIG_CAMERA_PIN_PWDN
#define RESET_GPIO_NUM   CONFIG_CAMERA_PIN_RESET
#define XCLK_GPIO_NUM    CONFIG_CAMERA_PIN_XCLK
#define SIOD_GPIO_NUM    CONFIG_CAMERA_PIN_SIOD
#define SIOC_GPIO_NUM    CONFIG_CAMERA_PIN_SIOC

#define Y9_GPIO_NUM      CONFIG_CAMERA_PIN_Y9
#define Y8_GPIO_NUM      CONFIG_CAMERA_PIN_Y8
#define Y7_GPIO_NUM      CONFIG_CAMERA_PIN_Y7
#define Y6_GPIO_NUM      CONFIG_CAMERA_PIN_Y6
#define Y5_GPIO_NUM      CONFIG_CAMERA_PIN_Y5
#define Y4_GPIO_NUM      CONFIG_CAMERA_PIN_Y4
#define Y3_GPIO_NUM      CONFIG_CAMERA_PIN_Y3
#define Y2_GPIO_NUM      CONFIG_CAMERA_PIN_Y2
#define VSYNC_GPIO_NUM   CONFIG_CAMERA_PIN_VSYNC
#define HREF_GPIO_NUM    CONFIG_CAMERA_PIN_HREF
#define PCLK_GPIO_NUM    CONFIG_CAMERA_PIN_PCLK
#endif

#define XCLK_FREQ       20000000

void app_camera_init();

#endif
