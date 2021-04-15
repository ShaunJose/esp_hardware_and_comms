#include "motion_sensor.h"
#include "bluetooth_door.h"

extern "C" void app_main()
{
    app_door_bt_main(); // setup bt communication with esp-eye
    app_motion_sensor_main(); // motion sensor waiting to be triggered
}
