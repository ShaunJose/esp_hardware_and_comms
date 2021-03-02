print('blink_led.py is loaded!')

import time
from machine import Pin

led_pin = 0

def run(pin=led_pin):

    p = Pin(pin,Pin.OUT)

    while True:
        print("powering pin on")
        p.value(1)
        time.sleep(1)
        print("powering pin off")
        p.value(0)
        time.sleep(1)
