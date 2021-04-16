/* MAIN FILE */

#include "app_camera.h"
#include "bluetooth_camera.h"

// Entire app's main function
extern "C" void app_main()
{
    app_camera_init(); // Configure camera pins and components
    app_camera_bt_main(); // Setup bluetooth (BLE) communication with the esp32
}
