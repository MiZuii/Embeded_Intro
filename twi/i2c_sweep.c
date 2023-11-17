//#ifndef F_CPU
#define F_CPU 16000000UL // cpu clock frequency
#define BUFFER_SIZE 256
//#endif
 
#include <stdio.h>
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
#include <avr/interrupt.h>

#define TWSR_MASK 0xF8 /* masks the prescaler bits of the twsr register*/
#define TWI_TIMEOUT 4000 /* Clock ticks after which the twi tramsmision is considered faulty
    (must be lower than uint16_t max) */
#define TWI_TIMEOUT_ERR -1 /* error code returned if a twi function ended in a timeout */
#define TWI_FAILURE -1 /* main twi exit error */
#define TWI_SUCCESS 0 /* returned on corrent behaviour of twi */

char addr_buff[4];
uint8_t stat = 0;

/* ---------------------------------- USART --------------------------------- */

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

/* ----------------------------------- I2C ---------------------------------- */

volatile uint8_t state = 0xF8;  /* Represents the TWI state mashine current state */
uint16_t twi_timer = 0;

uint32_t TWI_get_baud_rate(uint32_t baud) {
    /* assums the prescaler bits of TWSR are set to 0 */
    return (((F_CPU/baud) - 16) / 2) & 0xFF;
}

void TWI_init(uint8_t baud) {

    /* set the baud rate generating the scl clock fequency*/
    TWBR = baud;

    /* TWCR is responsible for controlling the twi connection 
    setting the TWEN enables the twi and setting the TWIE enables the twi interrupts*/
    TWCR = (1 << TWEN) | (1 << TWIE);

}

int TWI_start() {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);

    twi_timer = 0;
    while( state != TW_START ) {
        twi_timer++;
        if(twi_timer > TWI_TIMEOUT) {
            return TWI_TIMEOUT_ERR;
        }
    }

    return TWI_SUCCESS;
}

int TWI_stop() {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN) | (1 << TWIE);
    return TWI_SUCCESS;
}

int TWI_repeated_start() {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);

    twi_timer = 0;
    while( state != TW_REP_START ) {
        twi_timer++;
        if(twi_timer > TWI_TIMEOUT) {
            return TWI_TIMEOUT_ERR;
        }
    }

    return TWI_SUCCESS;
}

int TWI_transmit_slaw(uint8_t slave_addr) {

    TWDR = (slave_addr << 1); /* slave addr is 7 bits (thats why the shift exists) the write bit is 0*/
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);

    twi_timer = 0;
    while( state != TW_MT_SLA_ACK && state != TW_MT_SLA_NACK && state != TW_MT_ARB_LOST ) {
        twi_timer++;
        if(twi_timer > TWI_TIMEOUT) {
            return TWI_TIMEOUT_ERR;
        }
    }

    if(TW_MT_SLA_ACK == state) {
        return TWI_SUCCESS;
    }
    return state;
}

int TWI_transmit_slar(uint8_t slave_addr) {
    TWDR = (slave_addr << 1) | 1; /* slave addr is 7 bits (thats why the shift exists) the read bit is 1*/
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);

    twi_timer = 0;
    while( state != TW_MR_SLA_ACK && state != TW_MR_SLA_NACK && state != TW_MR_ARB_LOST ) {
        twi_timer++;
        if(twi_timer > TWI_TIMEOUT) {
            return TWI_TIMEOUT_ERR;
        }
    }

    if(TW_MR_SLA_ACK == state) {
        return TWI_SUCCESS;
    } 
    return state;
}

int TWI_transmit_data(uint8_t data) {

    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
    
    twi_timer = 0;
    while( state != TW_MT_DATA_ACK && state != TW_MT_DATA_NACK && state != TW_MT_ARB_LOST ) {
        twi_timer++;
        if(twi_timer > TWI_TIMEOUT) {
            return TWI_TIMEOUT_ERR;
        }
    }

    if(TW_MT_DATA_ACK == state) {
        return TWI_SUCCESS;
    }
    return state;
}

int TWI_read_data(uint8_t *data, uint8_t ack) {
    /* 
    data -  pointer to a memory space to safe the read data to.
    ack - if 1 the device transmits the acknowladge pulse (this continues the reading loop)
          if 0 the device transmits the not acknowladge pulse to end the transmision
     */
    *data = TWDR;
    TWCR = (1 << TWINT) | ((1 && ack) << TWEA) | (1 << TWEN) | (1 << TWIE); /* responsible for writeing the acknowladge(or not) signal*/

    twi_timer = 0;
    while( state != TW_MR_DATA_ACK && state != TW_MR_DATA_NACK && state != TW_MR_ARB_LOST ) {
        twi_timer++;
        if(twi_timer > TWI_TIMEOUT) {
            return TWI_TIMEOUT_ERR;
        }
    }

    if(TW_MR_DATA_ACK == state && ack) {
        return TWI_SUCCESS;
    } else if(TW_MR_DATA_NACK == state && !ack) {
        return TWI_SUCCESS;
    }
    return state;
}

int TWI_transmit(uint8_t slave_addr, uint8_t *data, size_t data_size) {
    uint8_t loc_state = 0;

    loc_state = TWI_start();
    if(loc_state) {
        return TWI_FAILURE;
    }

    loc_state = TWI_transmit_slaw(slave_addr);
    if(loc_state) {
        return TWI_FAILURE;
    }

    for (size_t i = 0; i < data_size; i++)
    {
        loc_state = TWI_transmit_data(data[i]);
        if(loc_state) {
            return TWI_FAILURE;
        }
    }

    loc_state = TWI_stop();
    if(loc_state) {
        return TWI_FAILURE;
    }

    return TWI_SUCCESS;
}

int TWI_read(uint8_t slave_addr, uint8_t *data, size_t data_size) {
    uint8_t loc_state = 0;

    if( data_size < 1 ) {
        return TWI_FAILURE;
    }

    loc_state = TWI_start();
    if(loc_state) {
        return TWI_FAILURE;
    }

    loc_state = TWI_transmit_slar(slave_addr);
    if(loc_state) {
        return TWI_FAILURE;
    }

    for (size_t i = 0; i < data_size-1; i++)
    {
        loc_state = TWI_read_data((data + i), 1);
        if(loc_state) {
            return TWI_FAILURE;
        }
    }

    loc_state = TWI_read_data((data + data_size - 1), 0);
    if(loc_state) {
        return TWI_FAILURE;
    }

    loc_state = TWI_stop();
    if(loc_state) {
        return TWI_FAILURE;
    }
    return TWI_SUCCESS;
}

int TWI_addr_exists(uint8_t slave_addr) {
    uint8_t loc_state = 0;

    loc_state = TWI_start();
    if(loc_state) {
        return loc_state;
    }

    loc_state = TWI_transmit_slaw(slave_addr);

    TWI_stop();
    if(loc_state) {
        return loc_state;
    }
    return TWI_SUCCESS;
}


/* ------------------------------- INTERRUPTS ------------------------------- */

ISR(TWI_vect) {
    state = (TWSR & TWSR_MASK);
}

ISR(USART1_RX_vect) {
    buff[buff_iter] = UDR1;
    buff_iter++;
}

/* ---------------------------------- MAIN ---------------------------------- */

int main(void)
{
    USART_Init(4800);
    TWI_init(TWI_get_baud_rate(100000)); /* 100kHz speed */

    while(1)
    {
        _delay_ms(250);

        send_string("Active Devices:\r\n");

        for (uint8_t i = 0; i < 129; i++) {
            _delay_ms(10);
            stat = TWI_addr_exists(i);

            if(stat == 0) {
                send_string("Device ");
                sprintf(addr_buff, "%d", i);
                addr_buff[3] = '\0';
                send_string(addr_buff);
                send_string(" found\r\n");
            }
        }
    }
}
