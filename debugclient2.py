#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from serial import Serial
import struct
from time import sleep

AVRint = struct.Struct('<H')
buffer = struct.Struct('<' + 'H'*32);

s = Serial('/dev/ttyACM0', 115200)
s.setDTR(False)
sleep(0.01)
s.setDTR(True)

sleep(1)

s.flushInput()

sleep(1)

s.write(b'R')
while True:
    d = s.read(64)
    print(repr(buffer.unpack(d)))
