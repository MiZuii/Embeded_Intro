#include "st7789_defs.h"
#include "st7789_usart.h"

#include <stdio.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/*                             SPI IMPLEMENTATION                             */
/* -------------------------------------------------------------------------- */

void SPI_MasterInit(void)
{
/* Set MOSI and SCK output, all others input */
DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK);
/* Enable SPI, Master, set clock rate fck/16 */
SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}
void SPI_MasterTransmit(char cData)
{
/* Start transmission */
SPDR = cData;
/* Wait for transmission complete */
while(!(SPSR & (1<<SPIF)))
;
}

int main(void) {

    USART_Init(USART_BAUD);

    return EXIT_SUCCESS;
}