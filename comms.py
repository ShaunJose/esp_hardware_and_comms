import network
import urequests
import ubluetooth
from micropython import const


_IRQ_SCAN_RESULT = const(5)
_IRQ_SCAN_DONE = const(6)
_IRQ_PERIPHERAL_CONNECT = const(7)
_IRQ_PERIPHERAL_DISCONNECT = const(8)
_IRQ_GATTC_SERVICE_RESULT = const(9)
_IRQ_GATTC_CHARACTERISTIC_RESULT = const(11)
_IRQ_GATTC_READ_RESULT = const(15)
_IRQ_GATTC_NOTIFY = const(18)

# call connect_wifi() once at the start, then call hit_api() every time we want to hit the API

SSID = 'myssid'
PASSWORD = 'password'

DOOR_CONTROLLER_UUID = ubluetooth.UUID('20B16612-AD37-48E0-8C3D-BB9D50F67BFE')
# camera esp writes to change whether door is opening/closing, reads to check current status,
# is notified to tell it to read the current status
DOOR_CONTROLLER_CHAR_UUID = ubluetooth.UUID('30B16612-AD37-48E0-8C3D-BB9D50F67BFE')
DOOR_CONTROLLER_CHARACTERISTICS = (DOOR_CONTROLLER_CHAR_UUID,
                                   ubluetooth.FLAG_WRITE | ubluetooth.FLAG_READ | ubluetooth.FLAG_NOTIFY)


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


class BLEDoorCentral:
    def __init__(self, ble):
        self.ble = ble
        self.ble.active(True)
        self.ble.irq(handle=self.irq)

        self.addr_type = None
        self.addr = None
        self.rssi = None
        self.conn_handle = None
        self.value_handle = None

    def reset(self):
        self.addr_type = None
        self.addr = None
        self.rssi = None
        self.conn_handle = None
        self.value_handle = None

    def scan(self, on=True):
        self.ble.gap_scan(0 if on else None)

    def irq(self, event, data):
        if event == _IRQ_SCAN_RESULT:
            # A single scan result.
            addr_type, addr, adv_type, rssi, adv_data = data
            if adv_data == b'door_esp32':
                self.addr_type = addr_type
                self.addr = bytes(addr)
                self.scan(on=False)
        elif event == _IRQ_SCAN_DONE:
            if self.addr and self.addr_type:
                self.ble.connect(self.addr_type, self.addr)
        elif event == _IRQ_PERIPHERAL_CONNECT:
            self.conn_handle, addr_type, addr = data
            self.ble.gattc_discover_services(self.conn_handle)
        elif event == _IRQ_PERIPHERAL_DISCONNECT:
            self.reset()
            self.scan(on=True)
        elif event == _IRQ_GATTC_SERVICE_RESULT:
            conn_handle, start_handle, end_handle, uuid = data
            if conn_handle == self.conn_handle and uuid == DOOR_CONTROLLER_UUID:
                self.ble.gattc_discover_characteristics(self.conn_handle, start_handle, end_handle)
        elif event == _IRQ_GATTC_CHARACTERISTIC_RESULT:
            conn_handle, def_handle, value_handle, properties, uuid = data
            if conn_handle == self.conn_handle and uuid == DOOR_CONTROLLER_CHAR_UUID:
                self.value_handle = value_handle
        elif event == _IRQ_GATTC_NOTIFY:
            conn_handle, value_handle, notify_data = data
            if conn_handle == self.conn_handle and value_handle == self.value_handle:
                # TODO: the ESP32 controlling the door has updated the door state (value is in notify_data), react here
                pass
        elif event == _IRQ_GATTC_READ_RESULT:
            conn_handle, value_handle, char_data = data
            if conn_handle == self.conn_handle and value_handle == self.value_handle:
                # TODO: deal with data read from the door ESP32
                pass

    def get_door_state(self):
        # result is returned in _IRQ_GATTC_READ_RESULT IRQ
        self.ble.gattc_read(self.conn_handle, self.value_handle)

    def set_door_state(self, door_state):
        self.ble.gattc_write(self.conn_handle, self.value_handle, door_state)


def init_bluetooth():
    ble = ubluetooth.BLE()
    door_central = BLEDoorCentral(ble)
    door_central.scan(on=True)
