#ifndef F_CPU
#define F_CPU 16000000UL // cpu clock frequency
#endif
 
#include <avr/io.h>
#include <util/delay.h>
 
int main(void)
{
    DDRF = 0x01;                      // initialize port C
    while(1)
    {
        // LED on
        PORTF = 0b10000000;            // PC0 = High = Vcc
        _delay_ms(500);                // wait 500 milliseconds
 
        //LED off
        PORTF = 0b00000000;            // PC0 = Low = 0v
        _delay_ms(500);                // wait 500 milliseconds
    }
}