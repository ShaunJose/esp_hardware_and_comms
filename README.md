# Communication
## Device to Cloud
The communication between the ESP-EYE and the cloud was accomplished using HTTPS. This added a layer of security.
The binary image captured by the camera was sent using a POST request.

## Local Communication
Bluetooth Low Energy (BLE) was used to enable commmunication between the ESP-EYE
and ESP32. The ESP-EYE acted as a GATT client, while the ESP32 was a GATT server.
The ESP32 sent a notification to the ESP-EYE to take a picture when the
motion sensor was triggered. After receiving a response from the cloud, the ESP-EYE
sent the response as a message to the ESP32 so that it could take the appropriate action in regards to opening the door/switching on LEDs.
