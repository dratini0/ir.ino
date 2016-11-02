#ifndef __AVR_ATmega328P__
#error This header was designed for the Atmega 328P
#endif // __AVR_ATmega328P__

#include <avr/io.h>

FUSES = {
    .low = 0xFF,
    .high = FUSE_SPIEN & FUSE_EESAVE & FUSE_BOOTSZ1 & FUSE_BOOTSZ0,
    .extended = EFUSE_DEFAULT
};
