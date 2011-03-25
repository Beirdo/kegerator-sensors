.PHONY:	force

CC = avr-gcc
OBJDUMP = avr-objdump

INCS = -I /usr/lib/avr/include
SRCS = main.c version.c uart.c crc.c sensor.c tcn75a.c i2c.c
OBJS = ${SRCS:%.c=%.o}

ELF  = temperature_sensor.elf
DISS = ${SRCS:%.c=%.dis} ${ELF:%.elf=%.dis}

all:	${ELF}

%.o:	%.c
	${CC} -g -O3 -mmcu=atmega8 -c $+ -o $@

%.dis:	%.o
	${OBJDUMP} -Sx $+ > $@

%.dis:	%.elf
	${OBJDUMP} -Sx $+ > $@

${ELF}:	${OBJS}
	${CC} -g -O3 -mmcu=atmega8 ${OBJS} -o $@

version.c:	force
	git describe --always --dirty | ./mkversion.sh

clean:
	${RM} ${OBJS} ${DISS} ${ELF} version.c
