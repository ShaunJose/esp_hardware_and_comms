#include "app_camera.h"
#include "bluetooth_camera.h"

extern "C" void app_main()
{
    app_camera_init(); // Configure camera
    app_camera_bt_main(); // Setup bt communication with the esp32
}
