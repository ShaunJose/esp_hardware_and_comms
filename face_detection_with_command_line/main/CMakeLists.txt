set(COMPONENT_SRCS
    app_main.cpp
    app_facenet.c
    app_camera.cpp
    bluetooth_door.c
    servo.c
    blink.c
    motion_sensor.c
    )

set(COMPONENT_ADD_INCLUDEDIRS
    include
    )

set(COMPONENT_REQUIRES
    esp32-camera
    esp-face
    nvs_flash
    esp_http_client
    bt
    )

register_component()
