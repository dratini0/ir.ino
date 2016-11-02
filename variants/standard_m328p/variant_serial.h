#ifndef VARIANT_SERIAL_H_INCLUDED
#define VARIANT_SERIAL_H_INCLUDED

inline void SERIAL_SETUP(long baudrate)
{
    cbi();
    // hardcoded exception for 57600 for compatibility with the bootloader
    // shipped with the Duemilanove and previous boards and the firmware
    // on the 8U2 on the Uno and Mega 2560. Also, The baud_setting cannot
    // be > 4095, so switch back to non-u2x mode if the baud rate is too
    // low.
    if (((F_CPU == 16000000UL) && (baud == 57600)) || (baud_setting >4095))
    {
        UBRR0 = (F_CPU / 8 / baud - 1) / 2;
        UCSR0A = _BV(TXC0);
    }
    else
    {
        UBRR0 = (F_CPU / 4 / baud - 1) / 2;
        UCSR0A = _BV(TXC0) | _BV(U2X0);
    }

    // assign the baud_setting, a.k.a. ubrr (USART Baud Rate Register)
    //set the data bits, parity, and stop bits
    UCSR0B = _BV(TXEN0) | _BV(RXEN) | _BV(RXCIE0) | _BV(UDRIE0);
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
    sei();
}

#endif // VARIANT_SERIAL_H_INCLUDED
