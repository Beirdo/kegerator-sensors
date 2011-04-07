CC = arm-unknown-linux-gnueabi-gcc
OBJDUMP = arm-unknown-linux-gnueabi-objdump

CFLAGS = -g -O3 -Wall -Werror -I../libcgi
LDFLAGS = -L../libcgi -lcgi

SRCS = main.c data.c
OBJS = ${SRCS:%.c=%.o}

EXE = graph-data

all: ${EXE}

%.o:	%.c
	${CC} ${CFLAGS} -c $+ -o $@

%.dis:	%.o
	${OBJDUMP} -Sx $+ > $@

${EXE}:	${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

clean:
	${RM} ${OBJS} ${EXE} *.dis
