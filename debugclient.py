#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from serial import Serial
import struct
from time import sleep

AVRint = struct.Struct('<H')

s = Serial('/dev/ttyUSB0', 115200)

#s.setDTR(False)
#sleep(0.01)
#s.setDTR(True)

#sleep(1)

#s.flushInput()

#sleep(1)

s.write(b'R')
while True:
    d = s.read()
    if d == b'C':
        print('C', end='')
    elif d == b'E':
        print('E ' + str(AVRint.unpack(s.read(AVRint.size))[0]))
    elif d == b'F':
        print('F' + str(AVRint.unpack(s.read(AVRint.size))[0]))
    elif d == b'M':
        len = AVRint.unpack(s.read(AVRint.size))[0]
        data = struct.unpack('<' + 'H'*len, s.read(AVRint.size * len))
        print('M ' + repr(data))
        sleep(1)
        s.write(b'R')
    else:
        print(d.decode('windows-1252'))

