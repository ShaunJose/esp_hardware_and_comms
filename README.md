# esp32
esp32 code with Micropython    

Currently contains code to blink the led every one second   


### Dependencies   
1. pyserial    
```
python3 -m pip install pyserial
```   

2. picocom     

Linux:   
```
sudo apt-get install picocom
```   

Mac: (with homebrew)   
```
arch -x86_64 brew install picocom
```    

3. urequests
```
python3 -m pip install urequests
```   

4. network
```
python3 -m pip install network
```   


### Run

Make sure your esp32 is connected via a usb port   

1. Enter this directory    
```
cd esp32
```   

2. Erase Flash  
```
esptool.py --chip esp32 --port <YOUR USB PORT connected to esp32> erase_flash
```    

Your usb port could be, for example, ```/dev/ttyUSB0```    

3. Write Flash    
```
esptool.py --chip esp32 --port <YOUR USB PORT> --baud 460800 write_flash -z 0x1000 esp32-idf3-20190125-v1.10.bin
```     

4. Run REPLace.py
```
python3 REPLace.py
```   

5. Enter the picocom terminal  
```
picocom -b 115200 <YOUR USB PORT>
```   

6. Import blink_led   
```
import blink_led
```   

7. Run the pin that you want to power on and off   
```
blink_led.run(<PIN_NUMBER>)
```   

Note: PIN_NUMBER param is optional. Defaults to pin 2. Use the pin which needs to be powered, on the esp32 in your circuit.
