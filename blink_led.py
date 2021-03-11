print('blink_led.py is loaded!')

import time
from machine import Pin


def run():

    led_pin = 2
    pir_pin = 0
    power_pin = 5
    curr_state = 0

    time.sleep(4) # Wait so program doesnt complain that vars arent declared

    # Define pins and set init values
    led = Pin(led_pin, Pin.OUT)
    pir = Pin(pir_pin, Pin.IN)
    power = Pin(power_pin)
    led.off()
    pir.off()
    power.value(1)

    time.sleep(4) # Wait so program doesnt complain that pins arent set

    while True:
        if pir.value() == 1 and curr_state == 0:
            print("Motion detected!")
            led.on()
            curr_state = 1
        if pir.value() == 0 and curr_state == 1:
            print("Motion stopped!")
            led.off()
            curr_state = 0
            time.sleep(5)
