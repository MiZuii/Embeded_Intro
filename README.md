# Embeded_Intro

A collection of easy programs for microcontroller `Atmega32u4` placed on `Arduino pro micro`. The set contains of:
 + `init.c` - Blinking led
 + `hello.c` - Sending serial signal with string message through UART serial protocol.
 + `receiver.c` - Receiving UART signal and sending message with the signal received
 + `interrupt_receiver.c` - Receiving UART signal and sending message back [but the receiving is done through interrupts]
 + `i2c_sweep.c` - Performs a addres sweep on the TWI bus.

 ## Building and Uploading

 Project build is made by `Makefile`. To compile targeted file, init.c for example, run command

 > make FILENAME=init

 The hex files and .o are stored in bin directory. The upload is done by [usbasp programmer](http://msx-elektronika.pl/pl/usbasp-avr/). Uploading is done through linux system using command (the port below is only an example)

> make FILENAME=init PORT=/dev/bus/001/002

## Additional information

### Usbipd

List of usefull commands for managing usb connections in wsl.

Listing available usb connections
> usbipd wsl list

attaching usb to wsl
> usbipd wsl attach --busid \<busid\>

dettaching usb from wsl
> usbipd wsl dettach --busid \<busid\>

### Serial software

Signal receiving is done with [RealTerm](https://realterm.sourceforge.io/) software.

### Compilation tools

To compile code appropriate compiler and other tools are necesery. Installation command

```bash
sudo apt-get update
sudo apt-get install gcc-avr binutils-avr avr-libc gdb-avr avrdude
```

### VsCode setup

For IntelliSense support the `C/C++` extension should be installed. And appropriate `c_cpp_properties.json` file should be included.  

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/avr-gcc",
            "cStandard": "c17",
            "cppStandard": "gnu++17",
            "intelliSenseMode": "linux-gcc-x64",
            "compilerArgs": [
                "-mmcu=atmega32u4"
            ]
        }
    ],
    "version": 4
}
```

Important fields:
 + `"compilerPath"` - Insert here a path to local avr-gcc compiler
 + `"-mmcu=atmega32u4"` - Inser here the identyfication of the device used.

 ### Datasheets
 All necesery datasheets can be found in the directory `datasheets`. Additional links to useful pages are also there.