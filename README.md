# esp_hardware_and_comms for a Face Mask Verifier   

## README sections  

* (Overiew)[#overview]
* (Setting up prerequisites)[#setting-up-the-prerequisites]
* (Build and Run)[#build-and-run]
* (Project explanation)(#project-explanation) : Explanation of the embedded systems, communication using BLE and wifi, and the code

## Overview  

This repository contains the code for the face Mask Verifier. It works together with the API and image processing app in (this repository)[https://github.com/pkjennings999/CS7NS2-API/tree/main/CS7NS2-API]

`face_mask_verifier/src/face_mask_verifier_esp32` contains the code to be run and flashed onto the esp32.   

`face_mask_verifier/src/face_mask_verifier_esp_eye` contains the code to be run and flashed onto the esp32.      

These two run together and communicate via bluetooth as explained at the end of this README

## Setting up prerequisites  

To build this project, you first need to setup esp-idf.  
Please refer to the (official site)[https://docs.espressif.com/projects/esp-idf/en/v4.0/get-started/index.html] by Espressif for this setup.

## Build and Run

1. Enter the esp32 repository
```
cd face_mask_verifier/src/face_mask_verifier_esp32
```

2. Run the `export.sh` code as explained in the getting start link by Espressif above   
For example:
```
. ~/esp/esp-idf/export.sh
```   

3. Clean any previous builds, if it exists
```
idf.py fullclean
```

4. Build the app
```
idf.py build
```

5. Flash it onto the device using the relevant port. In this case the port is /dev/cu.usbserial-1110
```
idf.py -p /dev/cu.usbserial-1110 flash monitor
```

6. Open a new terminal and navigate to the esp-eye repository
```
cd face_mask_verifier/src/face_mask_verifier_esp_eye
```

7. Follow steps 2 to 6 in this folder

## Project explanation  



### Communication
#### Device to Cloud
The communication between the ESP-EYE and the cloud was accomplished using HTTPS. This added a layer of security.
The binary image captured by the camera was sent using a POST request.

#### Local Communication
Bluetooth Low Energy (BLE) was used to enable commmunication between the ESP-EYE
and ESP32. The ESP-EYE acted as a GATT client, while the ESP32 was a GATT server.
The ESP32 sent a notification to the ESP-EYE to take a picture when the
motion sensor was triggered. After receiving a response from the cloud, the ESP-EYE
sent the response as a message to the ESP32 so that it could take the appropriate action in regards to opening the door/switching on LEDs.
