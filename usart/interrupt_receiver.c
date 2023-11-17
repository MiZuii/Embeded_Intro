//#ifndef F_CPU
#define F_CPU 16000000UL // cpu clock frequency
#define BUFFER_SIZE 200
//#endif
 
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
#include <avr/interrupt.h>

volatile uint8_t buff_iter = 0;
volatile unsigned char buff[BUFFER_SIZE];


uint16_t USART_get_ubrr( uint16_t baud ) {

    uint16_t speed = 16;

    if( UCSR1A & U2X1 ) {
        // high speed is enabled
        speed = 8;
    }

    return (F_CPU/((unsigned long)speed*(unsigned long)baud)) - 1;
}


void USART_Init( uint16_t baud )
{
    cli();  /* block interrupts */

    uint16_t ubrr_val = USART_get_ubrr( baud );

    UBRR1H = (unsigned char)(ubrr_val >> 8);
    UBRR1L = (unsigned char)ubrr_val;
    
    /* Set frame format: 8data, 1stop bit */
    UCSR1C = (3 << UCSZ10);
    /* to turn on 2bit stop use 
    UCSR1C = (1 << USBS1) | (3 << UCSZ10);
    */

    // enable transmiter and receiver
    UCSR1B = (1 << TXEN1) | (1 << RXEN1);

    // turn on recive interrupts
    UCSR1B |= (1 << RXCIE1);

    sei(); /* enable interrupts */
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

int not_eot_char(unsigned char c) {
    return c;
}

int main(void)
{
    USART_Init(4800);

    while(1)
    {
        // wait for full message to be received
        while ( (buff_iter == 0) | (not_eot_char(buff[buff_iter - 1]) && buff_iter < BUFFER_SIZE - 1) );

        // safety null (in case string was longer than buffer)
        buff[buff_iter] = '\0';

        //reset buff
        buff_iter = 0;

        send_string("> received: ");
        send_string((char *)buff);
        send_string("\r\n");
    }
}

ISR(USART1_RX_vect) {
    buff[buff_iter] = UDR1;
    buff_iter++;
}