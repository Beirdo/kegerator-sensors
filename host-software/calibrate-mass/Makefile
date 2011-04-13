CC = arm-unknown-linux-gnueabi-gcc
OBJDUMP = arm-unknown-linux-gnueabi-objdump

CFLAGS = -g -O3 -Wall -Werror

SRCS = main.c serial.c crc.c sensors.c
OBJS = ${SRCS:%.c=%.o}

EXE = calibrate-mass

all: ${EXE}

%.o:	%.c
	${CC} ${CFLAGS} -c $+ -o $@

%.dis:	%.o
	${OBJDUMP} -Sx $+ > $@

${EXE}:	${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

clean:
	${RM} ${OBJS} ${EXE} *.dis
