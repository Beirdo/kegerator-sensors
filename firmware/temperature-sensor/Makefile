.PHONY:	force

CC = avr-gcc
OBJDUMP = avr-objdump
OBJCOPY = avr-objcopy
CPU = atmega8

PRG = temp_sensor

INCS = -I . -I ../common
LDFLAGS = -Wl,-Map,${MAP}
CFLAGS = -g -O3 -Wall -Werror -mmcu=${CPU}

COMMONSRCS  = ../common/main.c ../common/crc.c ../common/sensor-uart.c 
COMMONSRCS += ../common/i2c.c ../common/tcn75a.c ../common/sensor.c
SRCS = local.c version.c
OBJS = ${COMMONSRCS:../common/%.c=%.o} ${SRCS:%.c=%.o}

ELF  = ${PRG}.elf
MAP  = ${PRG}.map
DISS = ${OBJS:%.o=%.dis} ${ELF:%.elf=%.dis}
HEX  = flash.hex eeprom.hex

all:	${HEX}

flash.hex:	${ELF}
	${OBJCOPY} -j .text -j .data -O ihex $+ $@
	
eeprom.hex:	${ELF}
	${OBJCOPY} -j .eeprom --change-section-lma .eeprom=0 -O ihex $+ $@

%.o:	../common/%.c
	${CC} ${CFLAGS} ${INCS} -c $+ -o $@

%.o:	%.c
	${CC} ${CFLAGS} ${INCS} -c $+ -o $@

${ELF}:	${OBJS}
	${CC} -g -O3 -mmcu=${CPU} ${OBJS} -o $@

version.c:	force
	git describe --always --dirty | ../common/mkversion.sh

clean:
	${RM} ${OBJS} ${DISS} ${ELF} version.c ${HEX} ${MAP}
