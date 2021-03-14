import time
from machine import Pin

print('blink_led.py is loaded!')

def run():

    # Define pin numbers
    led_pin = 33
    pir_pin = 25
    curr_state = 0

    # Define pins and turn everything off
    led = Pin(led_pin, Pin.OUT)
    pir = Pin(pir_pin, Pin.IN)
    led.off()
    pir.off()

    # Turn led on when motion detected and off when it's stopped
    while True:
        if curr_state == 0 and pir.value() == 1:
            print("Motion detected!")
            led.on()
            curr_state = 1
            pir.value(0) # Now ignore motion from the same person
        if curr_state == 1 and pir.value() == 0:
            print("Motion sensor sleeping")
            led.off()
            curr_state = 0
            time.sleep(15) # Ignore any motion for a few seconds - it's probably from the same person!
            print("Ready for next motion!")
