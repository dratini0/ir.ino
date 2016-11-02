/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 * Edited by Mitra to add new controller SANYO
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#ifndef IRremote_h
#define IRremote_h

#include <variant_ticksize.h>

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
// #define DEBUG
// #define TEST

// main class for receiving IR
class IRrecv
{
public:
    IRrecv();
    int isReady(unsigned int* &pointer);
    void enableIRIn();
    void stop();
}
;

// Only used for testing; can remove virtual for shorter code
#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

class IRsend
{
public:
    IRsend() {}
    void sendRaw(unsigned int buf[], int len);
    // private:
    void enableIROut(int khz);
    VIRTUAL void mark(int usec);
    VIRTUAL void space(int usec);
}
;

// Some useful constants

#define USECPERTICK 1  // microseconds per clock interrupt tick
#define USECPERTICK_DIVISOR 2
#define RAWBUF 250 // Length of raw duration buffer
#define INTBUF 32

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#endif
