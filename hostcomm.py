import struct
import time

import serial


class Frame:
    def __init__(self, device_id, func_id, size, buffer=b""):
        assert device_id >= 0 and device_id < 16, f"device ID {device_id} out of range [0, 15]"
        assert func_id >= 0 and func_id < 128, f"device ID {func_id} out of range [0, 127]"
    
        self.device_id = device_id
        self.func_id = func_id
        self.size = size
        self.buffer = buffer


ser = serial.Serial(port="COM16", baudrate=1000000, timeout=0.01)


def transmit(frame):
    can_id = (frame.func_id << 4) | frame.device_id
    header = struct.pack(">HBB", can_id, frame.size, 0)
    #print(header)
    
    buffer = header + frame.buffer

    ser.write(buffer)
    

def receive(size=8):
    buffer = ser.read(4+size)
    #print(buffer)
    
    can_id, size, _ = struct.unpack(">HBB", buffer[0:4])
    frame = Frame(can_id & 0x0F, can_id >> 4, size, buffer[4:])
    return frame


f = Frame(2, 0x0, 0)
#f = Frame(2, 0x0, 1, b"\x01")
#f = Frame(2, 0x0, 2, b"\x01\x02")
#f = Frame(2, 0x2, 3, b"\x03\x06\x04")
#f = Frame(2, 0x0, 8, b"\x01\x02\x03\x04\x05\x06\x07\x08")

counter = 0
t = time.time()
try:
    while True:
        counter += 1
        transmit(f)
        fr = receive(1)
        #print("dev_id:", fr.device_id, "func_id:", fr.func_id, "size:", fr.size, fr.buffer)
except KeyboardInterrupt:
    print(counter / (time.time() - t), "frames / sec")

