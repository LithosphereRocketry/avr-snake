CC=avr-gcc
CFLAGS=-mmcu=attiny85 -Os -flto

.PHONY: flash flash-hv clean

SRCS = $(wildcard *.c)
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.o)

# based on a makefile by drj11 which in turn is based on Arduino's build system
flash: main.hex at85fuses.py
	./at85fuses.py  --format avrdude --no-prescale --disable-reset \
		| xargs -L 1 \
		avrdude -p attiny85 -c arduino -U flash:w:main.hex -P /dev/ttyUSB0 -b 19200

main.hex: main.elf
	avr-objcopy -O ihex -R .eeprom $< $@

main.bin: main.elf
	avr-objcopy -O binary -R .eeprom $< $@

%.o: %.c
	$(CC) -MMD $(CFLAGS) -c -o $@ $<

main.elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o *.d main.elf main.hex main.bin

-include $(DEPS)