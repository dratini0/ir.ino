/*
 * IRremote
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz>
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#include "IRremote2.h"
#include <variant.h>

// Provides ISR
#include <avr/interrupt.h>
#include <Arduino.h>

unsigned int* sendBuf;
int sendBufLen;
size_t sendBufIdx;
int lastSend;

/*void IRsend::sendRaw(unsigned int buf[], int len)
{
    for (int i = 0; i < len; i++)
    {
        if (i & 1)
        {
            space(buf[i]);
        }
        else
        {
            mark(buf[i]);
        }
    }
    space(0); // Just to be sure
}*/

void IRsend::sendRaw(unsigned int buf[], int len)
{
    sendBuf = buf;
    sendBufLen = len;
    sendBufIdx = 0;
    //Serial.write('P');
    cli();
    OCR1A = lastSend = TCNT1 + 8;
    TIFR1 = _BV(OCF1A);
    TIMSK1 |= _BV(OCIE1A);
    sei();
}

void IRsend::mark(int time)
{
    // Sends an IR mark for the specified number of microseconds.
    // The mark output is modulated at the PWM frequency.
    PWM_ENABLE(); // Enable pin 3 PWM output
    delayMicroseconds(time);
}

/* Leave pin off for time (given in microseconds) */
void IRsend::space(int time)
{
    // Sends an IR space for the specified number of microseconds.
    // A space is no output, so the PWM output is disabled.
    PWM_DISABLE(); // Disable pin 3 PWM output
    delayMicroseconds(time);
}

void IRsend::enableIROut(int khz)
{
    // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
    // The IR output will be on pin 3 (OC2B).
    // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
    // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
    // TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
    // controlling the duty cycle.
    // There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
    // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
    // A few hours staring at the ATmega documentation and this will all make sense.
    // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.


    // Disable the Timer2 Interrupt (which is used for receiving IR)

    PWM_PIN_SETUP(); // When not sending PWM, we want it low

    // COM2A = 00: disconnect OC2A
    // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
    // WGM2 = 101: phase-correct PWM with OCRA as top
    // CS2 = 000: no prescaling
    // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
    PWM_CONFIG_KHZ(khz);
    ICP_CONFIG();
    //Serial.write('O');
}

ISR(TIMER1_COMPA_vect)
{
    if(sendBufIdx & 1)
    {
        PWM_DISABLE();
    }
    else
    {
        PWM_ENABLE();
    }
    //Serial.write('Q');
    if(sendBufIdx == sendBufLen){
        PWM_DISABLE();
        TIMSK1 &= ~(_BV(OCIE1A));
        return;
    }
    OCR1A = lastSend = lastSend + sendBuf[sendBufIdx++];
}

/*
New IRecv:
Timer1 runs on F_CPU/8
Input capture has noise canceller
As long as enabled, ISR records pulses into small-ish (32-value? 64 byte, 16 pulse) ring buffer
ISR has to toggle its sense!
Ready function has to be called *often*, at least every 10 ms
It copies lengths from the ring buffer to the buffer to be returned


*/

volatile unsigned int intBuf[INTBUF];
volatile unsigned int* volatile intBufEnd; // I spik C
volatile unsigned int* intBufBegin;
unsigned int rawBuf[RAWBUF];
size_t rawBufIdx;
bool recording;
int lastEvent;

IRrecv::IRrecv() {}

void IRrecv::enableIRIn()
{
    cli();
    //set pin as input
    ICP_PIN_SETUP();
    //clear raw buffer
    rawBufIdx = 0;
    //start looking for the first gap
    recording = false;
    //measure starting gap from now
    lastEvent = ICP_ICR;
    //clear internal circle buffer
    intBufEnd = &intBuf[0];
    intBufBegin = &intBuf[0];
    ICP_CONFIG();
    ICP_ENABLE_INTERUPT();
    sei();              // enable interrupts, let it go!
    //Serial.write('N');
}

void IRrecv::stop()
{
    ICP_DISABLE_INTERRUPT();
}

int IRrecv::isReady(unsigned int* &pointer)
{
    //Serial.write('C');
    while(intBufBegin != intBufEnd)  // until circular buffer empties
    {
        unsigned int event = *intBufBegin;
        //Serial.write('E');
        //Serial.write((byte *) &event, sizeof event);
        if(event - lastEvent > GAP_TICKS && ((intBufBegin - &intBuf[0]) & 1) == 0)  //if we are on a beginning of a mark, after a sufficient gap
        {
            if(recording)
            {
                stop();
                pointer = &rawBuf[0];
                //Serial.write('S');
                return rawBufIdx;
            }
            else
            {
                recording = true;
                //Serial.write('R');
            }
        }
        if(recording)
        {
            rawBuf[rawBufIdx] = event - lastEvent;
            rawBufIdx++;
            if(rawBufIdx == RAWBUF)
            {
                stop();
                pointer = &rawBuf[0];
                return RAWBUF;
            }
        }
        lastEvent = event;
        intBufBegin++;
        if(intBufBegin == &intBuf[INTBUF]) intBufBegin = &intBuf[0];
    }
    int time = ICP_TCNT;
    //Serial.write('T');
    //Serial.write((char*) &time, sizeof(time));
    if (recording && ((unsigned int)(time - lastEvent) >= GAP_TICKS))   // quit as soon as long enoug gap is found
    {
        stop();
        pointer = &rawBuf[0];
        //Serial.write('s');
        return rawBufIdx;
    }
    if(!recording && ((unsigned int)(time - lastEvent) >= 0x8000U))  // make sure the first gap won't overflow
    {
        lastEvent = time - GAP_TICKS;
        //Serial.write('r');
    }
    //Serial.write((byte*) intBuf, 64); //dump the entire buffer for debugclient2.py
    return 0;
}

/*
ISR(TIMER1_CAPT_vect){
asm volatile(
  "lds __tmp_reg__, %[TCCR1B_]\n\t"
  "ldi r16, %[ICES1_]\n\t"
  "eor __tmp_reg__, r16\n\t"
  "sts %[TCCR1B_], __tmp_reg__\n\t"
  :
  :[TCCR1B_] "M" (&TCCR1B),
   [ICES1_] "M" (1 << ICES1)
);
}
*/
ISR(ICP_INTERRUPT_NAME)
{
    ICP_TOGGLE_EDGE();
    int time = ICP_ICR;
    //time critical part ends, we can dawdle a little
    //sbi(PINB, PINB5); // AVR lingo for toggling LED (1 cycle!)
    *(intBufEnd++) = time; // is this valid, and will it compile to postincrement-acces? (yes, no)
    if(intBufEnd == &intBuf[INTBUF]) intBufEnd = &intBuf[0];
    //Serial.write('F');
    //Serial.write((char*) &time, sizeof(time));
}
