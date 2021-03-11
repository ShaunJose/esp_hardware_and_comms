print('blink_led.py is loaded!')

import time
from machine import Pin


def run():

    led_pin = 2
    pir_pin = 0
    power_pin = 5
    curr_state = 0

    time.sleep(4)

    # Define pins and set init values
    led = Pin(led_pin, Pin.OUT)
    pir = Pin(pir_pin, Pin.IN)
    power = Pin(power_pin, Pin.OUT)
    led.value(0)
    pir.value(0)
    power.value(1)

    time.sleep(4)

    while True:
        if pir.value() == 1 and curr_state == 0:
            print("Motion detected!")
            led.value(1)
            curr_state = 1
        if pir.value() == 0 and curr_state == 1:
            print("Motion stopped!")
            led.value(0)
            curr_state = 0
            time.sleep(5)
