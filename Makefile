CC=avr-gcc
CFLAGS=-mmcu=attiny85 -Os

.PHONY: flash clean

SRCS = $(wildcard *.c)
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.o)

# based on a makefile by drj11 which in turn is based on Arduino's build system
flash: main.hex
	avrdude -pattiny85 -cusbasp -Uflash:w:main.hex -B125kHz

main.hex: main.elf
	avr-objcopy -O ihex -R .eeprom $< $@

%.o: %.c
	$(CC) -MMD $(CFLAGS) -c -o $@ $<

main.elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o *.d main.elf main.hex

-include $(DEPS)