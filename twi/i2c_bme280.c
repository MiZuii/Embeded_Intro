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

#define BIT_SIZEOF(x) sizeof(x)*8

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* -------------------------------------------------------------------------- */
/*                                 BME DEFINES                                */
/* -------------------------------------------------------------------------- */

#define BME_ID 0x60

#define BME_INVALID_DATA_SIZE 1
#define BME_SLAVE_ADDR 0x76

#define BME_PT_SIZE 6
#define BME_PTH_SIZE 9
#define BME_BURST_REG 0xF7

#define BME_DIG_SIZE 32

#define BME_BURST_1_DIG 0x88
#define BME_BURST_1_DIG_SIZE 25
#define BME_BURST_2_DIG 0xE1
#define BME_BURST_2_DIG_SIZE 7

#define BME_CTRL_REG 0xF4

/* ----------------------------- CONFIG DEFINES ----------------------------- */

#define BME_REG_CTRL_HUM    0xF2
#define BME_REG_CTRL_MEAS   0xF4
#define BME_REG_CONFIG      0xF5

/*oversampling values*/
#define BME_SAMPLING_SKIP   0x00
#define BME_SAMPLING_X1     0x01
#define BME_SAMPLING_X2     0x02
#define BME_SAMPLING_X4     0x03
#define BME_SAMPLING_X8     0x04
#define BME_SAMPLING_X16    0x05

/*modes*/
#define BME_MODE_NORMAL     0x03
#define BME_MODE_FORCED     0x01
#define BME_MODE_SLEEP      0x00

/*standby time*/
#define BME_STANDBY_T_0_5   0x00
#define BME_STANDBY_T_62_5  0x01
#define BME_STANDBY_T_125   0x02
#define BME_STANDBY_T_250   0x03
#define BME_STANDBY_T_500   0x04
#define BME_STANDBY_T_1000  0x05
#define BME_STANDBY_T_10    0x06
#define BME_STANDBY_T_20    0x07

/*filter settings*/
#define BME_FILTER_OFF      0x00
#define BME_FILTER_2        0x01
#define BME_FILTER_4        0x02
#define BME_FILTER_8        0x03
#define BME_FILTER_16       0x04

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

/* -------------------------------------------------------------------------- */
/*                                     TWI                                    */
/* -------------------------------------------------------------------------- */

volatile uint8_t state = 0xF8;  /* Represents the TWI state mashine current state */
uint8_t cstate = 0;
uint16_t twi_timer = 0;

/* 
Timer Macro - Resets timer and waits for condition.
    if condition met before timout continue, else return
    timeout error.
*/
#define TWI_TIME(x)                     \
    twi_timer = 0;                      \
    while( x ) {                        \
        twi_timer++;                    \
        if(twi_timer > TWI_TIMEOUT) {   \
            return TWI_TIMEOUT_ERR;     \
        }                               \
    }                                   \

/*
State Check Macro - Checks if status returned is success.
    If not the returned error status is propagated onto lower
    level function.
*/
#define STATE_CHECK(x)      \
    cstate = x;             \
    if(cstate) {            \
        return cstate;      \
    }


uint32_t TWI_get_baud_rate(uint32_t scl_speed) {
    /* assums the prescaler bits of TWSR are set to 0 */
    return (((F_CPU/scl_speed) - 16) / 8) & 0xFF;
}

void TWI_init(uint32_t speed) {

    cli();

    /* ensure the TWI receives clock */
    PRR0 = 0;

    /* set the baud rate generating the scl clock fequency*/
    TWSR = 0;
    TWBR = TWI_get_baud_rate(speed);

    /* TWCR is responsible for controlling the twi connection 
    setting the TWEN enables the twi and setting the TWIE enables the twi interrupts*/
    TWCR = (1 << TWEN) | (1 << TWIE);

    sei();
}

int TWI_start() {

    /* Clear TWINT and set TWSTA(start signal) bit to 1 */
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);

    /* wait for start transmit confirmation */
    TWI_TIME(state != TW_START);

    return EXIT_SUCCESS;
}

int TWI_stop() {
    /* Clear TWINT and set TWSTO(stop signal) bit to 1 */
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN) | (1 << TWIE);
    return EXIT_SUCCESS;
}

int TWI_repeated_start() {

    /* Clear TWINT and set TWSTA(start signal) bit to 1 */
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);

    /* Wait for repeated start status */
    TWI_TIME(state != TW_REP_START)

    return EXIT_SUCCESS;
}

int TWI_transmit_slaw(uint8_t slave_addr) {
    /* slave addr is 7 bits (thats why the shift exists) the write bit is 0*/
    TWDR = (slave_addr << 1);

    /* clear TWINT */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);

    /* Wait for ack or nack or arbitration loss */
    TWI_TIME(state != TW_MT_SLA_ACK && state != TW_MT_SLA_NACK && state != TW_MT_ARB_LOST)

    /* Success if acknowladged */
    if(TW_MT_SLA_ACK == state) {
        return EXIT_SUCCESS;
    }

    /* return state as error if failed */
    return state;
}

int TWI_transmit_slar(uint8_t slave_addr) {

    /* slave addr is 7 bits (thats why the shift exists) the read bit is 1*/
    TWDR = (slave_addr << 1) | 1;

    /* clear TWINT */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);

    /* Wait for ack or nack or arbitration loss */
    TWI_TIME(state != TW_MR_SLA_ACK && state != TW_MR_SLA_NACK && state != TW_MR_ARB_LOST)

    /* Success if acknowladged */
    if(TW_MR_SLA_ACK == state) {
        return EXIT_SUCCESS;
    }

    /* return state as error if failed */
    return state;
}

int TWI_transmit_data(uint8_t data) {

    /* load data to transmit */
    TWDR = data;

    /* clear TWINT  */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
    
    /* Wait for ack or nack or arbitration loss */
    TWI_TIME(state != TW_MT_DATA_ACK && state != TW_MT_DATA_NACK && state != TW_MT_ARB_LOST)

    /* Success if acknowladged */
    if(TW_MT_DATA_ACK == state) {
        return EXIT_SUCCESS;
    }

    /* return state as error if failed */
    return state;
}

int TWI_read_data(uint8_t *data, uint8_t ack) {
    /* 
    data -  pointer to a memory space to safe the read data to.
    ack - if 1 the device transmits the acknowladge pulse (this continues the reading loop)
          if 0 the device transmits the not acknowladge pulse to end the transmision
     */

    /* clear TWINT and set if ack or nack is transmited(continue or end further readout) */
    TWCR = (1 << TWINT) | ((1 && ack) << TWEA) | (1 << TWEN) | (1 << TWIE);

    /* Wait for ack or nack or arbitration loss */
    TWI_TIME(state != TW_MR_DATA_ACK && state != TW_MR_DATA_NACK && state != TW_MR_ARB_LOST)

    /* get loaded data */
    *data = TWDR;

    /* Check if proper signal was transmited */
    if(TW_MR_DATA_ACK == state && ack) {
        return EXIT_SUCCESS;
    } else if(TW_MR_DATA_NACK == state && !ack) {
        return EXIT_SUCCESS;
    }

    /* return state as error if failed */
    return state;
}

int TWI_transmit(uint8_t slave_addr, uint8_t *data, size_t data_size) {

    STATE_CHECK(TWI_start())

    STATE_CHECK(TWI_transmit_slaw(slave_addr))

    for (size_t i = 0; i < data_size; i++) {
        STATE_CHECK(TWI_transmit_data(data[i]))
    }

    TWI_stop();
    return EXIT_SUCCESS;
}

int TWI_read(uint8_t slave_addr, uint8_t *data, size_t data_size) {

    if( data_size < 1 ) {
        return EXIT_FAILURE;
    }

    STATE_CHECK(TWI_start())

    STATE_CHECK(TWI_transmit_slar(slave_addr))

    for (size_t i = 0; i < data_size-1; i++) {
        STATE_CHECK(TWI_read_data((data + i), 1))
    }

    STATE_CHECK(TWI_read_data((data + data_size - 1), 0))

    TWI_stop();
    return EXIT_SUCCESS;
}

int TWI_addr_exists(uint8_t slave_addr) {
    
    if(!slave_addr) {
        return EXIT_FAILURE;
    }

    STATE_CHECK(TWI_start())

    cstate = TWI_transmit_slaw(slave_addr);

    TWI_stop();
    STATE_CHECK(cstate)
    return EXIT_SUCCESS;
}


/* ------------------------------- INTERRUPTS ------------------------------- */

ISR(TWI_vect) {
    state = (TWSR & TWSR_MASK);
}

ISR(USART1_RX_vect) {
    buff[buff_iter] = UDR1;
    buff_iter++;
}

/* ---------------------------- BME280 FUNCTIONS ---------------------------- */

/*
Dig Concatenate Macro - Concatenates two uint8 into one uint16
*/
#define BME_DIG_CONCATENATE(lb, rb) (((uint16_t)lb << 8) | (uint16_t)rb)

typedef int32_t BME280_S32_t;
typedef uint32_t BME280_U32_t;
typedef int64_t BME280_S64_t;
BME280_S32_t t_fine;

struct bme_dig
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
};

struct bme_res
{
    int32_t temp;
    uint32_t pres;
    uint32_t humi;
};


int BME_config(uint8_t T_oversampling, uint8_t P_oversampling, uint8_t H_oversampling, uint8_t mode, uint8_t IIR_mode, uint8_t standby_time) {

    /* Caution! - the ctrl_hum register updates only after writeing to ctrl_meas register. */

    uint8_t reg_ctrl_hum = H_oversampling;
    uint8_t reg_ctrl_meas = (T_oversampling << 5) | (P_oversampling << 2) | mode;
    uint8_t reg_config = (standby_time << 5) | (IIR_mode << 2);

    uint8_t data_buff[6];
    data_buff[0] = BME_REG_CTRL_HUM;
    data_buff[1] = reg_ctrl_hum;
    data_buff[2] = BME_REG_CTRL_MEAS;
    data_buff[3] = reg_ctrl_meas;
    data_buff[4] = BME_REG_CONFIG;
    data_buff[5] = reg_config;

    return TWI_transmit(BME_SLAVE_ADDR, data_buff, 6);
}

int BME_burst_read(uint8_t *data, size_t data_size, uint8_t burst_addr) {
    uint8_t loc_state;

    if( data_size < 1 ) {
        return TWI_FAILURE;
    }

    STATE_CHECK(TWI_start())

    loc_state = TWI_transmit_slaw(BME_SLAVE_ADDR);
    if(loc_state) {
        TWI_stop();
        return loc_state;
    }

    loc_state =  TWI_transmit_data(burst_addr);
    if(loc_state) {
        TWI_stop();
        return loc_state;
    }

    STATE_CHECK(TWI_repeated_start())

    loc_state = TWI_transmit_slar(BME_SLAVE_ADDR);
    if(loc_state) {
        TWI_stop();
        return loc_state;
    }

    for(size_t data_idx = 0; data_idx < data_size-1; data_idx++) {
        loc_state = TWI_read_data((data + data_idx), 1);
        if(loc_state) {
            TWI_stop();
            return loc_state;
        }
    }

    loc_state = TWI_read_data((data + data_size - 1), 0);
    if(loc_state) {
        TWI_stop();
        return loc_state;
    }

    TWI_stop();
    return EXIT_SUCCESS;
}

int BME_get_data(uint8_t *data, size_t data_size) {
    /* There are two options for burst read
    1. Only the pressure and temperature is read.
    2. The pressure, temperature and humidity is read.
    
    The first option requires 48bits of memory -> data_size == 6
    The seccond option requires 72bits of memory -> data_size == 9
    
    If other data_size is provided the function returns failure */

    if( BME_PT_SIZE != data_size && BME_PTH_SIZE != data_size ) {
        return BME_INVALID_DATA_SIZE;
    }
    return BME_burst_read(data, data_size, BME_BURST_REG);
}

BME280_S32_t BME280_compensate_T_int32(uint32_t adc_T, struct bme_dig *dig) {
    BME280_S32_t var1, var2, T;
    var1  = ((((adc_T>>3) - ((BME280_S32_t)dig->dig_T1<<1))) * ((BME280_S32_t)dig->dig_T2)) >> 11;
    var2  = (((((adc_T>>4) - ((BME280_S32_t)dig->dig_T1)) * ((adc_T>>4) - ((BME280_S32_t)dig->dig_T1))) >> 12) * ((BME280_S32_t)dig->dig_T3)) >> 14;  
    t_fine = var1 + var2;
    T  = (t_fine * 5 + 128) >> 8;
    return T;
}

BME280_U32_t BME280_compensate_P_int64(uint32_t adc_P, struct bme_dig *dig) {
    BME280_S64_t var1, var2, p;
    var1 = ((BME280_S64_t)t_fine) - 128000;
    var2 = var1 * var1 * (BME280_S64_t)dig->dig_P6;
    var2 = var2 + ((var1*(BME280_S64_t)dig->dig_P5)<<17);
    var2 = var2 + (((BME280_S64_t)dig->dig_P4)<<35);
    var1 = ((var1 * var1 * (BME280_S64_t)dig->dig_P3)>>8) + ((var1 * (BME280_S64_t)dig->dig_P2)<<12);
    var1 = (((((BME280_S64_t)1)<<47)+var1))*((BME280_S64_t)dig->dig_P1)>>33;
    if (var1 == 0) {   
        return 0; /* avoid exception caused by division by zero */
    }
    p = 1048576-adc_P;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((BME280_S64_t)dig->dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((BME280_S64_t)dig->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)dig->dig_P7)<<4);
    return (BME280_U32_t)p;
}

BME280_U32_t BME280_compensate_H_int32(uint32_t adc_H, struct bme_dig *dig) {
    BME280_S32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((BME280_S32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)dig->dig_H4) << 20) - (((BME280_S32_t)dig->dig_H5) * v_x1_u32r)) + ((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)dig->dig_H6)) >> 10) * (((v_x1_u32r * ((BME280_S32_t)dig->dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152)) * ((BME280_S32_t)dig->dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)dig->dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r); 
    return (BME280_U32_t)(v_x1_u32r>>12);
}

int BME_fill_dig(struct bme_dig *dig) {
    uint8_t bme_dig_buff[BME_DIG_SIZE];

    STATE_CHECK(BME_burst_read(bme_dig_buff, BME_BURST_1_DIG_SIZE, BME_BURST_1_DIG))

    STATE_CHECK(BME_burst_read(bme_dig_buff + BME_BURST_1_DIG_SIZE, BME_BURST_2_DIG_SIZE, BME_BURST_2_DIG))

    dig->dig_T1 = BME_DIG_CONCATENATE(bme_dig_buff[1], bme_dig_buff[0]);
    dig->dig_T2 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[3], bme_dig_buff[2]);
    dig->dig_T3 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[5], bme_dig_buff[4]);
    dig->dig_P1 = BME_DIG_CONCATENATE(bme_dig_buff[7], bme_dig_buff[6]);
    dig->dig_P2 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[9], bme_dig_buff[8]);
    dig->dig_P3 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[11], bme_dig_buff[10]);
    dig->dig_P4 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[13], bme_dig_buff[12]);
    dig->dig_P5 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[15], bme_dig_buff[14]);
    dig->dig_P6 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[17], bme_dig_buff[16]);
    dig->dig_P7 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[19], bme_dig_buff[18]);
    dig->dig_P8 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[21], bme_dig_buff[20]);
    dig->dig_P9 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[23], bme_dig_buff[22]);
    dig->dig_H1 = bme_dig_buff[24];
    dig->dig_H2 = (int16_t)BME_DIG_CONCATENATE(bme_dig_buff[26], bme_dig_buff[25]);
    dig->dig_H3 = bme_dig_buff[27];
    dig->dig_H4 = (int16_t)((uint16_t)bme_dig_buff[28] << 4) | (uint16_t)(bme_dig_buff[29] && 0x0E);
    dig->dig_H5 = (int16_t)((uint16_t)bme_dig_buff[30] << 4) | (uint16_t)(bme_dig_buff[29] >> 4);
    dig->dig_H6 = (int8_t)bme_dig_buff[31];

    return EXIT_SUCCESS;
}

void BME_refractor_data(uint8_t *raw, struct bme_dig *dig, struct bme_res *res) {
    uint32_t adc_P = ((uint32_t)raw[0] << 12) | ((uint32_t)raw[1] << 4) | ((uint32_t)raw[2] >> 4);
    uint32_t adc_T = ((uint32_t)raw[3] << 12) | ((uint32_t)raw[4] << 4) | ((uint32_t)raw[5] >> 4);
    uint32_t adc_H = ((uint32_t)raw[6] << 8) | (uint32_t)raw[7];

    res->temp = BME280_compensate_T_int32(adc_T, dig);
    res->pres = BME280_compensate_P_int64(adc_P, dig);
    res->humi = BME280_compensate_H_int32(adc_H, dig);
}

int BME_id_check() {
    uint8_t id;

    STATE_CHECK(BME_burst_read(&id, 1, 0xD0))

    if(BME_ID == id) {
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

void send_temperature(int32_t temp) {
    char buff[24];
    sprintf(buff, "%ld.%ldC", temp / 100, temp % 100);
    buff[23] = '\0';
    send_string(buff);
}

void send_pressure(uint32_t press) {
    char buff[26];
    sprintf(buff, "%lu.%03luhPa", (press/25600), (((press % 25600) * 1000) / 25600));
    buff[25] = '\0';
    send_string(buff);
}

void send_humidity(uint32_t humi) {
    char buff[25];
    sprintf(buff, "%lu.%03lu%%RH", humi/1024, ((humi % 1024) * 1000) / 1024);
    buff[24] = '\0';
    send_string(buff);
}

/* ---------------------------------- MAIN ---------------------------------- */

int main(void)
{
    USART_Init(4800);
    TWI_init(100000); /* 100kHz speed */

    struct bme_dig dig;
    struct bme_res res;
    uint8_t stat = 0;
    uint8_t bme_buff[BME_PTH_SIZE];
    int32_t max_t=0, min_t=0;

    stat = BME_config(BME_SAMPLING_X4, BME_SAMPLING_X4, BME_SAMPLING_X4, BME_MODE_NORMAL, BME_FILTER_8, BME_STANDBY_T_62_5);
    if(stat) {
        send_string("Failed config\r\n");
        return EXIT_FAILURE;
    }

    stat = BME_fill_dig(&dig);
    if(stat) {
        send_string("Failed retriveing the dig_ registers\r\n");
        return EXIT_FAILURE;
    }

    stat = BME_id_check();
    if(stat) {
        send_string("Failed ID check\r\n");
        return EXIT_FAILURE;
    }

    while(1)
    {
        _delay_ms(150);

        /*get and refractor data*/
        STATE_CHECK(BME_get_data(bme_buff, BME_PTH_SIZE))
        BME_refractor_data(bme_buff, &dig, &res);
        if(0 == max_t) {
            max_t = res.temp;
        } else if (max_t < res.temp) {
            max_t = res.temp;
        }

        if(0 == min_t) {
            min_t = res.temp;
        } else if (min_t > res.temp) {
            min_t = res.temp;
        }

        /*print results*/
        send_string("-------------------\r\n");
        send_string("Temperature: ");
        send_temperature(res.temp);
        send_string("\r\nPresure: ");
        send_pressure(res.pres);
        send_string("\r\nHumidity: ");
        send_humidity(res.humi);
        send_string("\r\nSession Max: ");
        send_temperature(max_t);
        send_string("\r\nSession Min: ");
        send_temperature(min_t);
        send_string("\x1b[A\x1b[A\x1b[A\x1b[A\x1b[A\x1b[A\r");
        for(uint8_t i=0; i<7; i++){
            send_string("                \r\n");
        }
        send_string("\x1b[A\x1b[A\x1b[A\x1b[A\x1b[A\x1b[A\r");
    }
}
