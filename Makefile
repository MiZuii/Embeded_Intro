FILENAME = hello
DEVICE = m32u4
PORT = /dev/bus/usb/001/004
COMPILE = avr-gcc -Wall -Os -mmcu=atmega32u4
PROGRAMER = usbasp-clone
BINARIES_DIR = bin

all:
	make $(FILENAME)

$(FILENAME): $(FILENAME).c
	$(COMPILE) -c $(FILENAME).c -o $(BINARIES_DIR)/$(FILENAME).o
	$(COMPILE) -o $(BINARIES_DIR)/$(FILENAME).elf  $(BINARIES_DIR)/$(FILENAME).o
	avr-objcopy -j .text -j .data -O ihex $(BINARIES_DIR)/$(FILENAME).elf $(BINARIES_DIR)/$(FILENAME).hex

upload: $(BINARIES_DIR)/$(FILENAME).hex
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U flash:w:$(BINARIES_DIR)/$(FILENAME).hex:i

lfuse:
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U lfuse:w:0x9E:m 

clean:
	rm -f $(BINARIES_DIR)/*.*
