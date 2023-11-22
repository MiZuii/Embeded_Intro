#include "st7789_usart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

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

void send_uint8(uint8_t num) {
    char buff[4];
    sprintf(buff, "%d", num);
    buff[3] = '\0';
    send_string(buff);
}

void send_uint8_bin(uint8_t num) {
    char buff[BIT_SIZEOF(num) + 1];

    for(uint8_t i=BIT_SIZEOF(num); i > 0; i--) {
        buff[i-1] = num%2 ? '1' : '0';
        num = num >> 1;
    }
    buff[BIT_SIZEOF(num)] = '\0';
    send_string(buff);
}

void send_uint16(uint16_t num) {
    char buff[7];
    sprintf(buff, "%d", num);
    buff[6] = '\0';
    send_string(buff);
}

void send_uint16_bin(uint16_t num) {
    char buff[BIT_SIZEOF(num) + 1];

    for(uint8_t i=BIT_SIZEOF(num); i > 0; i--) {
        buff[i-1] = num%2 ? '1' : '0';
        num = num >> 1;
    }
    buff[BIT_SIZEOF(num)] = '\0';
    send_string(buff);
}

void send_uint32(uint32_t num) {
    char buff[11];
    sprintf(buff, "%ld", num);
    buff[10] = '\0';
    send_string(buff);
}

void send_uint32_bin(uint32_t num) {
    char buff[BIT_SIZEOF(num) + 1];

    for(uint8_t i=BIT_SIZEOF(num); i > 0; i--) {
        buff[i-1] = num%2 ? '1' : '0';
        num = num >> 1;
    }
    buff[BIT_SIZEOF(num)] = '\0';
    send_string(buff);
}

void send_int8(int8_t num) {
    char buff[5];
    sprintf(buff, "%d", num);
    buff[4] = '\0';
    send_string(buff);
}

void send_int8_bin(int8_t num) {
    char buff[BIT_SIZEOF(num) + 1];

    for(uint8_t i=BIT_SIZEOF(num); i > 0; i--) {
        buff[i-1] = num%2 ? '1' : '0';
        num = num >> 1;
    }
    buff[BIT_SIZEOF(num)] = '\0';
    send_string(buff);
}

void send_int16(int16_t num) {
    char buff[8];
    sprintf(buff, "%d", num);
    buff[7] = '\0';
    send_string(buff);
}

void send_int16_bin(int16_t num) {
    char buff[BIT_SIZEOF(num) + 1];

    for(uint8_t i=BIT_SIZEOF(num); i > 0; i--) {
        buff[i-1] = num%2 ? '1' : '0';
        num = num >> 1;
    }
    buff[BIT_SIZEOF(num)] = '\0';
    send_string(buff);
}

void send_int32(int32_t num) {
    char buff[12];
    sprintf(buff, "%ld", num);
    buff[11] = '\0';
    send_string(buff);
}

void send_int32_bin(int32_t num) {
    char buff[BIT_SIZEOF(num) + 1];

    for(uint8_t i=BIT_SIZEOF(num); i > 0; i--) {
        buff[i-1] = num%2 ? '1' : '0';
        num = num >> 1;
    }
    buff[BIT_SIZEOF(num)] = '\0';
    send_string(buff);
}