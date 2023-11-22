# Embeded_Intro

A collection of easy programs for microcontroller `Atmega32u4` placed on `Arduino pro micro`. The set contains of:
 + `leds/init.c` - Blinking led
 + `usart/hello.c` - Sending serial signal with string message through UART serial protocol.
 + `usart/receiver.c` - Receiving UART signal and sending message with the signal received
 + `usart/interrupt_receiver.c` - Receiving UART signal and sending message back [but the receiving is done through interrupts]
 + `twi/i2c_sweep.c` - Performs a addres sweep on the TWI bus.
 + `twi/i2c_bme280.c` - Reads from an BME280 sensor and displays the Temperature, Pressure and Humidity through uart.
 + `st7789_disp/st7789_main.c` - Performs the BME280 reading and displays it on lcd screen.

 ## Building and Uploading

 ### Bare-metal

 Files builds are made by `Makefile`. To compile targeted file, init.c for example, run command

 > make FILENAME=init

 The hex files and .o are stored in bin directory. The upload is done by [usbasp programmer](http://msx-elektronika.pl/pl/usbasp-avr/). Uploading is done through SPI on linux system using command:

> make FILENAME=init PORT=/dev/bus/001/002

Where `PORT` is the programer device file.

### Bootloader

Only the `st7789_disp` program is uploaded through bootloader as the lcd screen occupies the SPI bus. The [ubaboot](https://github.com/rrevans/ubaboot) bootloader is used. The configuration parameters for the `config.h` file are:

```C
#define OSC_MHZ_16
#define USB_REGULATOR
```
Bootloader flashing is explained on the [ubaboot](https://github.com/rrevans/ubaboot) github page. Caution, for correct usage of the bootloader python script a venv should be created. For tips on the script semantics run command:

```bash
sudo .venv/bin/python3 ./st7789_disp/ubaboot/ubaboot.py --help
```

### st7789_disp

As this is the special case the build is not done through the main `Makefile`. The `st7789_disp` directory has it's own makefile with all the needed dependencies. It also contains it's own `bin` direcotry for the hex files. To build and upload the st7789_disp run the below commands in the `st7789_disp` directory.

```bash
make
make upload
```

### Changeing bare-metal and bootloader fuses

Because there are two different ways of flashing there are two sets of fuses for the microcontroller. To upload fuses used for bare-metal programs run
> make fuse_bm

For fuses used with the bootloader use the command
> make fuse_ub

## Development software

### Usbipd

The development should be done through linux but it can also be configured for windows using the wsl. To connect the programer(or the device if bootloader is used) usbipd software is used. The below commands allow for the attachment and detachment of devices.

Listing available usb connections
> usbipd wsl list

attaching usb to wsl
> usbipd wsl attach --busid \<busid\>

dettaching usb from wsl
> usbipd wsl dettach --busid \<busid\>

To check if attachment worked and to read the file path of the device use command
> lsusb

### Serial software

Signal receiving is done with [RealTerm](https://realterm.sourceforge.io/) software. Can also be achieved through [Putty](https://www.putty.org/) with correct configuration.

### Compilation tools

To compile code appropriate compiler and other tools are necesery. Installation commands are

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
 All necesery datasheets can be found in the directory `datasheets`. Additional links to useful pages, pinouts and schematics are also there.