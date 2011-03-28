.PHONY:	force

CC = avr-gcc
OBJDUMP = avr-objdump
OBJCOPY = avr-objcopy
CPU = atmega128

PRG = receiver

INCS = -I . -I ../common
LDFLAGS = -Wl,-Map,${MAP}
CFLAGS = -g -O3 -Wall -Werror -mmcu=${CPU}

COMMONSRCS  = ../common/main.c ../common/crc.c ../common/i2c.c 
COMMONSRCS += ../common/tcn75a.c ../common/sensor.c
SRCS = local.c version.c uart0.c uart1.c
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

%.dis:	%.o
	${OBJDUMP} -Sx $+ > $@

%.dis:	%.elf
	${OBJDUMP} -Sx $+ > $@

${ELF}:	${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

version.c:	force
	git describe --always --dirty | ../common/mkversion.sh

clean:
	${RM} ${OBJS} ${DISS} ${ELF} version.c ${HEX} ${MAP}
