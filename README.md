# esp32
esp32 code with Micropython    

Currently contains code to blink the led every one second   


### Dependencies   
1. pyserial    
```
python3 -m pip install pyserial
```   

2. picocom   
Linux   
```
sudo apt-get install picocom
```   

### Run

Make sure your esp32 is connected   

1. Enter this directory    
```
cd esp32
```   

2. Run REPLace.py
```
python3 REPLace.py
```   

3. Enter the picocom terminal  
```
picocom -b 115200 <YOUR USB PORT connected to esp32>
```   

for example  
```
picocom -b 115200 /dev/ttyUSB0
```   

4. Import blink_led   
```
import blink_led
```   

5. Run the pin that you want to power on and off   
```
blink_led.run(<PIN_NUMBER>)
```   

Note: PIN_NUMBER param is optional. Defaults to pin 0
