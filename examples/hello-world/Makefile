CONTIKI_PROJECT = hello-world
all: hello-world
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex hello-world.avr-atmega128rfa1 hello-world.eep
	avr-objcopy -R .eeprom -R .fuse -R .signature -O ihex hello-world.avr-atmega128rfa1 hello-world.hex

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
