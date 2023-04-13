# Embeded_Intro

A collection of easy programs for microcontroller `Atmega32u4` placed on `Arduino pro micro`. The set contains of:
 + `init.c` - Blinking led
 + `hello.c` - Sending serial signal with string message through UART protocol.
 + `receiver.c` - Receiving (UART) signal turning led on and of

 ## Building and Uploading

 Project build is made by `Makefile`. To compile targeted file, init.c for example, run command

 > make FILENAME=init

 The hex files and .o are stored in bin directory. The upload is done by [usbasp programmer](http://msx-elektronika.pl/pl/usbasp-avr/). Uploading is done through linux system using command (the port below is only an example)

> make FILENAME=init PORT=/dev/bus/001/002

## Additional information

### Uart

Uart comunication is done through uart-to-usb converter. The signal is received with [RealTerm](https://realterm.sourceforge.io/) software.


