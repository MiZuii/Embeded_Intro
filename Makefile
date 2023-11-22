FILENAME = i2c_bme280
DIRECTORY = twi
DEVICE = m32u4
PORT = /dev/bus/usb/001/003
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


# ---------------------------------------------------------------------------- #
#                                BAREMETAL FUSES                               #
# ---------------------------------------------------------------------------- #
fuse_bm:
	make lfuse_bm
	make hfuse_bm
	make efuse_bm

lfuse_bm:
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U lfuse:w:0x9E:m

hfuse_bm:
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U hfuse:w:0xD8:m

efuse_bm:
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U efuse:w:0xCB:m

# ---------------------------------------------------------------------------- #
#                           UBABOOT BOOTLOADER FUSES                           #
# ---------------------------------------------------------------------------- #
fuse_ub:
	make lfuse_ub
	make hfuse_ub
	make efuse_ub

lfuse_ub:
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U lfuse:w:0x9E:m

hfuse_ub:
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U hfuse:w:0x9D:m

efuse_ub:
	sudo avrdude -v -p $(DEVICE) -c $(PROGRAMER) -P $(PORT) -U efuse:w:0xCB:m


# ----------------------------------- CLEAR ---------------------------------- #

clean:
	rm -f $(BINARIES_DIR)/*.*
