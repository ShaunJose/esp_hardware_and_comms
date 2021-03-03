import network
import urequests

# call connect_wifi() once at the start, then call hit_api() every time we want to hit the API

SSID = 'myssid'
PASSWORD = 'password'


def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('connecting to network...')
        wlan.connect(SSID, PASSWORD)
        while not wlan.isconnected():
            pass
    print('network config:', wlan.ifconfig())


def hit_api(url, method, data):
    response = urequests.request(method, url, data=data)
    return response.status_code, response.text
