#include <Arduino.h>

#include "IRremote2.h"

const int SENDBUF = 250;

IRrecv irrecv;
IRsend irsend;
bool listening = false;

const struct{
    int sendbuf;
    int recvbuf;
    unsigned long tickrate;
} about = {SENDBUF, RAWBUF, TICKRATE};

void setup()
{
    irsend.enableIROut(38);
    Serial.begin(115200);
    while(!Serial);
    //Serial.println(GAP_TICKS);
}

void loop()
{
    unsigned int* rawbuf;
    /*int len;
    if (len = irrecv.isReady(rawbuf)) {
      Serial.print(len);
      Serial.write('\n');
      irrecv.resume(); // The show must go on
    }*/
    switch(Serial.read())
    {
    case 'S':
        int datalen;
        unsigned int data[SENDBUF];
        Serial.readBytes((char*) &datalen, sizeof datalen);
        listening = false;
        if(datalen > SENDBUF)
        {
            //do not overflow, but discard
            while(datalen > 0)
            {
                if(Serial.read() != -1) datalen--;
            }
            return;
        }
        Serial.readBytes((char*) data, sizeof(unsigned int) * datalen);
        irsend.sendRaw(data, datalen);
        break;
    case '?':
        Serial.write('?');
        Serial.write((uint8_t*) &about, sizeof about);
        break;
    case 'R':
        irrecv.enableIRIn();
        listening = true;
    case -1:
    default:
        int len;
        if(listening && (len = irrecv.isReady(rawbuf)))
        {
            listening = false;
            Serial.write('M');
            Serial.write((byte*) &len, sizeof len);
            Serial.write((byte*) rawbuf, sizeof(unsigned int) * len);
        }
        break;
    }
}
