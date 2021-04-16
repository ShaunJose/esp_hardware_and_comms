# Hardware and Comms for a Face Mask Verifier   

## README sections  

* [Overview](#overview)
* [Setting up prerequisites](#setting-up-prerequisites)
* [Build and Run](#build-and-run)
* [Project explanation](#project-explanation): Explanation of the code for the embedded systems, and communication using BLE and wifi

## Overview  

This repository contains the code for the face Mask Verifier. It works together with the API and image processing app in [this repository](https://github.com/pkjennings999/CS7NS2-API/tree/main/CS7NS2-API)

`face_mask_verifier/src/face_mask_verifier_esp32` contains the code to be run and flashed onto the esp32.   

`face_mask_verifier/src/face_mask_verifier_esp_eye` contains the code to be run and flashed onto the esp32.      

These two run together and communicate via bluetooth as explained at the end of this README

## Setting up prerequisites  

To build this project, you first need to setup esp-idf.  
Please refer to the [official site](https://docs.espressif.com/projects/esp-idf/en/v4.0/get-started/index.html) by Espressif for this setup.

## Build and Run

1. Enter the esp32 repository
```
cd face_mask_verifier/src/face_mask_verifier_esp32
```

2. Run the `export.sh` code as explained in the getting started link by Espressif above   
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

5. Flash it onto the device using the relevant port. In this case the port is `/dev/cu.usbserial-1110`
```
idf.py -p /dev/cu.usbserial-1110 flash monitor
```

6. Open a new terminal and navigate to the esp-eye repository
```
cd face_mask_verifier/src/face_mask_verifier_esp_eye
```

7. Change the ssid and password to your wifi's details in the file `main/app_facenet.c`

8. Repeat steps 2 to 6

## Project explanation  

### Embedded Systems     

We use the ESP32 microprocessor to connect and control all the hardware components, except for the ESP-EYE. The ESP-EYE has no GPIO pins, and is connected to a power source separately via a usb-cable. For this reason, there are two separate programs, `face_mask_verifier/src/face_mask_verifier_esp32` for the esp32, and `face_mask_verifier/src/face_mask_verifier_esp_eye` for the esp-eye.  

#### ESP-32:
After running `face_mask_verifier_esp32` on the ESP-32, it sets up the configuration for communication via BLE, and waits for motion to be sensed by the motion sensor. Once the sensor gets triggered by some motion, it calls the `send_message` function in the bluetooth file in order to call the ESP-EYE. The ESP-EYE then works to get a take a picture and send back a response to the ESP-32 as explained below. The response is received as an integer:
1. 5 denotes that the face mask has been worn properly. In this case, we flash the green LED to indicate success and trigger the servo motor to open the door, wait and then close the door.
2. 6 denotes that a face has been detected but there's no face mask on. In this case, we flash the red LED
3. 7 denotes the unlikely scenario that no face has been found. Here, we flash the yellow LED to indicate that the person must position him/herself correctly and try again.   

#### ESP-EYE:
Running `face_mask_verifier_esp_eye` on the ESP-EYE causes it to setup the bluetooth configuration, where it hits the ESP32 once so the ESP32 can save it's details for communication throughout the running of the project. It then waits for the ESP32 to call it when motion has been sensed. Once called, it takes a picture, converts it to RGB and sends it to the API. The API works with an image processing app to figure out whether a mask is worn properly or not, or whether there is a human face at all. After receiving the appropriate response from the API ('true', 'false' or 'Face not found'), this program sets the response code to 5, 6 or 7 as explained above. It then sends this response to the ESP32 via BLE, which then decides how it should react to the result.


### Communication
#### Device to Cloud:
The communication between the ESP-EYE and the cloud was accomplished using HTTPS. This added a layer of security.
The binary image captured by the camera was sent using a POST request.

#### Local Communication:
Bluetooth Low Energy (BLE) was used to enable communication between the ESP-EYE
and ESP32. The ESP-EYE acted as a GATT client, while the ESP32 was a GATT server.
The ESP32 sent a notification to the ESP-EYE to take a picture when the
motion sensor was triggered. After receiving a response from the cloud, the ESP-EYE
sent the response as a message to the ESP32 so that it could take the appropriate action in regards to opening the door/switching on LEDs.
