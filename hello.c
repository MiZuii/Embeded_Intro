//#ifndef F_CPU
#define F_CPU 16000000UL // cpu clock frequency
#define BUFFER_SIZE 200
//#endif
 
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
 

void USART_Init( uint16_t ubrr_val )
{
    UBRR1H = (unsigned char)(ubrr_val >> 8);
    UBRR1L = (unsigned char)ubrr_val;
    
    /* Set frame format: 8data, 1stop bit */
    UCSR1C = (3 << UCSZ10);
    /* to turn on 2bit stop use 
    UCSR1C = (1 << USBS1) | (3 << UCSZ10);
    */

    // enable transmiter and receiver
    UCSR1B = (1 << TXEN1) | (1 << RXEN1);
}

uint16_t USART_get_ubrr( uint16_t baud ) {

    uint16_t speed = 16;

    if( UCSR1A & U2X1 ) {
        // high speed is enabled
        speed = 8;
    }

    return (F_CPU/((unsigned long)speed*(unsigned long)baud)) - 1;
}

void send_char( unsigned char ch ) {
    while( !(UCSR1A & (1 << UDRE1)) );
    UDR1 = ch;
}

void send_string( const char *string ) {

    for(int i=0; i<strlen(string); i++) {
        send_char((unsigned char)string[i]);
    }
    
}

int main(void)
{
    USART_Init(USART_get_ubrr( 4800 ));
    DDRF = 0x01;

    while(1)
    {

        PORTF = 0b10000000;            // PC0 = High = Vcc
        send_string("hello fien c:");
        send_string("\r\n");
        _delay_ms(250);
        
        PORTF = 0b00000000;            // PC0 = Low = 0v
        _delay_ms(250);
    }
}