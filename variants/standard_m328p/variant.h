#ifndef VARIANT_H_INCLUDED
#define VARIANT_H_INCLUDED

#ifndef __AVR_ATmega328P__
#error This header was designed for hardware using the Atmega 328P
#endif // __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>

#include "variant_ticksize.h"

inline void PIN_SETUP() //leaves serial and input capture pin intact
{
    DDRB = 0;
    PORTB =               _BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3) |
            _BV(PORTB4) | _BV(PORTB5) | _BV(PORTB6) | _BV(PORTB7);
    DDRC = 0;
    PORTC = _BV(PORTC0) | _BV(PORTC1) | _BV(PORTC2) | _BV(PORTC3) |
            _BV(PORTC4) | _BV(PORTC5);
    DDRD = 0;
    PORTD =               _BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3) |
            _BV(PORTB4) | _BV(PORTB5) | _BV(PORTB6) | _BV(PORTB7);
}

inline void PWM_PIN_SETUP()
{
    PORTD &= ~(_BV(PORTD3));
    DDRD |= _BV(DDB3);
}

inline void PWM_CONFIG_KHZ(int val)
{
    const uint8_t pwmval = F_CPU / 2000 / (val);
    TCCR2A = _BV(WGM20);
    TCCR2B = _BV(WGM22) | _BV(CS20); //phase correct PWM
    OCR2A = pwmval;
    OCR2B = pwmval / 3; //?
}

inline void PWM_ENABLE()
{
    TCCR2A |= _BV(COM2B1);
}

inline void PWM_DISABLE()
{
    TCCR2A &= ~(_BV(COM2B1));
}

#define ICP_INTERRUPT_NAME TIMER1_CAPT_vect
#define ICP_ICR ICR1
#define ICP_TCNT TCNT1

inline void ICP_PIN_SETUP()
{
    DDRB &= ~(_BV(DDB0));
    PORTB &= ~(_BV(PORTB0));
}

inline void ICP_CONFIG()
{
    TCCR1A = 0;
    TCCR1B = _BV(ICNC1) | // noise canceler
           //_BV(ICES1) | // falling edge first, i. e. mark begin
             _BV(CS11);   // /8 prescaler
    TIFR1 = _BV(ICF1);    // cancel any pending capture interrupts
}

inline void ICP_ENABLE_INTERUPT()
{
    TIMSK1 |= _BV(ICIE1);
}

inline void ICP_DISABLE_INTERRUPT()
{
    TIMSK1 &= ~(_BV(ICIE1));
}

inline void ICP_TOGGLE_EDGE()
{
    TCCR1B ^= _BV(ICES1);
}

#endif // VARIANT_H_INCLUDED
