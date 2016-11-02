#!/usr/bin/env python3

import struct
from time import sleep
from serial import Serial

def fromSerial(filename):
    tty = Serial(filename, 115200)
    return SerialIR(tty)

class SerialIR:
    aboutStruct = struct.Struct('<cHHL')
    def __init__(self, tty, sensorLag=34):
        self.tty = tty
        self.sensorLag = sensorLag
        self.autoConf()

    def autoConf(self):
        self.tty.write(b'?');
        data = self.tty.read(self.aboutStruct.size)
        letter, sendBuf, recieveBuf, tickrate = self.aboutStruct.unpack(data)
        if letter == b'?':
            self.uSecPerTick = 1000 / tickrate
            self.sendBuf = sendBuf
            self.recieveBuf = recieveBuf

    def writeMessage(self, data):
        data = tuple(int(i/self.uSecPerTick) for i in data)
        dataLen = len(data)
        self.tty.write(b'S')
        self.tty.write(struct.pack('<H', dataLen))
        self.tty.write(struct.pack('<' + 'H'*dataLen, *data))

    def readMessage(self):
        self.tty.write(b'R')
        if self.tty.read() != b'M': return False
        msgLen = self.tty.read(2)
        msgLen = struct.unpack('<H', msgLen)[0]
        msg = struct.unpack('<' + 'H'*msgLen, self.tty.read(2*msgLen))
        #print(msg)
        msg = msg[1:]
        msg = tuple(i * self.uSecPerTick for i in msg)
        return self.correctSensorLag(msg)

    def _correctSensorLag(self, data):
        i = iter(data)
        while True:
            yield next(i) - self.sensorLag
            yield next(i) + self.sensorLag

    def correctSensorLag(self, data):
        return tuple(self._correctSensorLag(data))

    def learn(self, samples = 100):
        data = [self.readMessage() for i in range(samples)]
        return tuple(sum(i) / samples for i in zip(*data))


if __name__ == "__main__":
    import sys
    instance = fromSerial(sys.argv[1])
    print(instance.learn())

