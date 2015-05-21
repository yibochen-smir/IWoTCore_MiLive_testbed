all: udp-sender udp-sink
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex udp-sink.avr-atmega128rfa1 application_sink.eep
	avr-objcopy -R .eeprom -R .fuse -R .signature -O ihex udp-sink.avr-atmega128rfa1 application_sink.hex
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex udp-sender.avr-atmega128rfa1 application_sender.eep
	avr-objcopy -R .eeprom -R .fuse -R .signature -O ihex udp-sender.avr-atmega128rfa1 application_sender.hex

CONTIKI = ../../..
APPS = serial-shell powertrace collect-view
# CONTIKI_PROJECT = udp-sender udp-sink
PROJECT_SOURCEFILES += collect-common.c

ifdef PERIOD
CFLAGS=-DPERIOD=$(PERIOD)
endif

# all: $(CONTIKI_PROJECT)

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
