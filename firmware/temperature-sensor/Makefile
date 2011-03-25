.PHONY:	force

CC = avr-gcc
OBJDUMP = avr-objdump
CPU = atmega8

INCS = -I . -I ../common
COMMONSRCS  = ../common/crc.c ../common/sensor-uart.c ../common/i2c.c
COMMONSRCS += ../common/tcn75a.c
SRCS = main.c version.c sensor.c
OBJS = ${COMMONSRCS:../common/%.c=%.o} ${SRCS:%.c=%.o}

ELF  = temperature_sensor.elf
DISS = ${OBJS:%.o=%.dis} ${ELF:%.elf=%.dis}

all:	${ELF}

%.o:	../common/%.c
	${CC} -g -O3 -mmcu=${CPU} ${INCS} -c $+ -o $@

%.o:	%.c
	${CC} -g -O3 -mmcu=${CPU} ${INCS} -c $+ -o $@

%.dis:	%.o
	${OBJDUMP} -Sx $+ > $@

%.dis:	%.elf
	${OBJDUMP} -Sx $+ > $@

${ELF}:	${OBJS}
	${CC} -g -O3 -mmcu=${CPU} ${OBJS} -o $@

version.c:	force
	git describe --always --dirty | ../common/mkversion.sh

clean:
	${RM} ${OBJS} ${DISS} ${ELF} version.c
