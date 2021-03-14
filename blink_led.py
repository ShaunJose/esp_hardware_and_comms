print('blink_led.py is loaded!')

import time
from machine import Pin


def run():

    led_pin = 33
    pir_pin = 25
    curr_state = 0

    time.sleep(4) # Wait so program doesnt complain that vars arent declared

    # Define pins and set init values
    led = Pin(led_pin, Pin.OUT)
    pir = Pin(pir_pin, Pin.IN)
    led.off()
    pir.off()

    time.sleep(4) # Wait so program doesnt complain that pins arent set

    time.sleep(60)

    while True:
        if curr_state == 1 and pir.value() == 0:
            print("Motion stopped!")
            led.off()
            curr_state = 0
            time.sleep(5)
        if curr_state == 0 and pir.value() == 1:
            print("Motion detected!")
            led.on()
            curr_state = 1
