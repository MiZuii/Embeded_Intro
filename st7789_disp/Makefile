MAIN = st7789_main
DEFS = st7789_defs
UART = st7789_usart
DEVICE = m32u4
PORT = /dev/bus/usb/001/003
CC = avr-gcc
FLAGS = -Wall -Os -mmcu=atmega32u4
PROGRAMER = usbasp-clone
BIN = bin

all:
	make $(MAIN)


$(MAIN): $(MAIN).c $(DEFS).h lib$(UART)
	$(CC) $(FLAGS) -c $(MAIN).c -o $(BIN)/$(MAIN).o
	$(CC) $(FLAGS) $(BIN)/$(MAIN).o $(BIN)/lib$(UART).a -o $(BIN)/$(MAIN).elf
	avr-objcopy -j .text -j .data -O ihex $(BIN)/$(MAIN).elf $(BIN)/$(MAIN).hex

lib$(UART): $(UART).c $(UART).h
	$(CC) $(FLAGS) -c $(UART).c -o $(BIN)/$(UART).o
	ar rcs $(BIN)/lib$(UART).a $(BIN)/$(UART).o

upload: $(MAIN)
	sudo ../.venv/bin/python3 ./ubaboot/ubaboot.py --write ./bin/st7789_main.hex

assembly: $(MAIN).c
	$(COMPILE) $(FLAGS) -S $(MAIN).c -o $(BIN)/$(MAIN).s

clean:
	rm -f $(BIN)/*.*