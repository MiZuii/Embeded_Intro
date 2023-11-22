#ifndef ST7789_USART_H
#define ST7789_USART_H

#include <stdint.h>

#define USART_BAUD 4800

#define BIT_SIZEOF(x) sizeof(x)*8

/* USART write only implementation */
uint16_t USART_get_ubrr( uint16_t baud );
void USART_Init( uint16_t baud );
void send_char( unsigned char ch );
void send_string( const char *string );

void send_uint8(uint8_t num);
void send_uint8_bin(uint8_t num);
void send_uint16(uint16_t num);
void send_uint16_bin(uint16_t num);
void send_uint32(uint32_t num);
void send_uint32_bin(uint32_t num);
void send_int8(int8_t num);
void send_int8_bin(int8_t num);
void send_int16(int16_t num);
void send_int16_bin(int16_t num);
void send_int32(int32_t num);
void send_int32_bin(int32_t num);

#endif // ST7789_USART_H