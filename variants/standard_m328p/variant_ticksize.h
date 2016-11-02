#ifndef VARIANT_TICKSIZE_H_INCLUDED
#define VARIANT_TICKSIZE_H_INCLUDED

#ifndef __AVR_ATmega328P__
#error This header was designed for hardware using the Atmega 328P
#endif // __AVR_ATmega328P__

#define TICKRATE (F_CPU / 1000 / 8)         // ticks per millisecond
#define GAP 5000                            // us
#define GAP_TICKS (GAP * TICKRATE / 1000)

#endif // VARIANT_TICKSIZE_H_INCLUDED
