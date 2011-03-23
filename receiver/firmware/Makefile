.PHONY:	force

CC = avr-gcc
OBJDUMP = avr-objdump

INCS = -I /usr/lib/avr/include
SRCS = main.c version.c uart0.c uart1.c crc.c sensor.c tcn75a.c i2c.c
OBJS = ${SRCS:%.c=%.o}

ELF  = receiver.elf
DISS = ${SRCS:%.c=%.dis} ${ELF:%.elf=%.dis}

all:	${ELF}

%.o:	%.c
	${CC} -g -O3 -mmcu=atmega128 -c $+ -o $@

%.dis:	%.o
	${OBJDUMP} -Sx $+ > $@

%.dis:	%.elf
	${OBJDUMP} -Sx $+ > $@

${ELF}:	${OBJS}
	${CC} -g -O3 -mmcu=atmega128 ${OBJS} -o $@

version.c:	force
	git describe --always --dirty | ./mkversion.sh

clean:
	${RM} ${OBJS} ${DISS} ${ELF} version.c
