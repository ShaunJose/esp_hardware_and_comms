import ubluetooth
from micropython import const

_IRQ_CENTRAL_CONNECT = const(1)
_IRQ_CENTRAL_DISCONNECT = const(2)
_IRQ_GATTS_WRITE = const(3)
_IRQ_GATTS_READ_REQUEST = const(4)
_IRQ_GATTC_READ_DONE = const(16)
_IRQ_GATTC_WRITE_DONE = const(17)
_IRQ_GATTC_NOTIFY = const(18)


_GATTS_NO_ERROR = const(0x00)

DOOR_CONTROLLER_UUID = ubluetooth.UUID('20B16612-AD37-48E0-8C3D-BB9D50F67BFE')
# camera esp writes to change whether door is opening/closing, reads to check current status,
# is notified to tell it to read the current status
DOOR_CONTROLLER_CHAR_UUID = ubluetooth.UUID('30B16612-AD37-48E0-8C3D-BB9D50F67BFE')
DOOR_CONTROLLER_CHARACTERISTICS = (DOOR_CONTROLLER_CHAR_UUID,
                                   ubluetooth.FLAG_WRITE | ubluetooth.FLAG_READ | ubluetooth.FLAG_NOTIFY)


class BLEDoor:
    def __init__(self, ble):
        self.ble = ble
        self.ble.active(True)
        self.ble.irq(handler=self.irq)
        door_controller_service = (DOOR_CONTROLLER_UUID, (DOOR_CONTROLLER_CHARACTERISTICS,),)
        services = (door_controller_service,)
        ((self.door_controller_handle,),) = self.ble.gatts_register_services(services)
        self.conn_handle = None

        self.advertise()

    def irq(self, event, data):
        if event == _IRQ_CENTRAL_CONNECT:
            conn_handle, _, _, = data
            self.conn_handle = conn_handle
        elif event == _IRQ_CENTRAL_DISCONNECT:
            conn_handle, _, _, = data
            if conn_handle == self.conn_handle:
                self.conn_handle = None
                # Start advertising again to allow a new connection.
                self.advertise()
        elif event == _IRQ_GATTS_READ_REQUEST:
            # A client has issued a read. Note: this is only supported on STM32.
            # Return a non-zero integer to deny the read (see below), or zero (or None)
            # to accept the read.
            return 0
        elif event == _IRQ_GATTS_WRITE:
            # TODO: the ESP32 with the camera has updated the door state, react here
            pass

    def advertise(self):
        self.ble.gap_advertise(interval=100000, adv_data=b'door_esp32')

    def get_door_state(self):
        return self.ble.gatts_read(self.door_controller_handle)

    def set_door_state(self, door_state):
        self.ble.gatts_write(self.door_controller_handle, door_state)
        self.ble.notify(self.conn_handle, self.door_controller_handle)


def init_bluetooth():
    ble = ubluetooth.BLE()
    return BLEDoor(ble)
