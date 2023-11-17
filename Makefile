FILENAME = i2c_disp
DIRECTORY = twi
DEVICE = m32u4
PORT = /dev/bus/usb/001/002
COMPILE = avr-gcc
FLAGS = -Wall -Os -mmcu=atmega32u4
PROGRAMER = usbasp-clone
BINARIES_DIR = bin

all:
	make $(DIRECTORY)/$(FILENAME)

$(DIRECTORY)/$(FILENAME): $(DIRECTORY)/$(FILENAME).c
	$(COMPILE) $(FLAGS) -c $(DIRECTORY)/$(FILENAME).c -o $(BINARIES_DIR)/$(FILENAME).o
	$(COMPILE) $(FLAGS) -o $(BINARIES_DIR)/$(FILENAME).elf  $(BINARIES_DIR)/$(FILENAME).o
	avr-objcopy -j .text -j .data -O ihex $(BINARIES_DIR)/$(FILENAME).elf $(BINARIES_DIR)/$(FILENAME).hex

upload: $(BINARIES_DIR)/$(FILENAME).hex
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U flash:w:$(BINARIES_DIR)/$(FILENAME).hex:i

assembly: $(FILENAME).c
	$(COMPILE) $(FLAGS) -S $(FILENAME).c -o $(BINARIES_DIR)/$(FILENAME).s

lfuse:
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U lfuse:w:0x9E:m 

clean:
	rm -f $(BINARIES_DIR)/*.*
