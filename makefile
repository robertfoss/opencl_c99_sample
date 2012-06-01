CFLAGS = -g -Wall -O2 -ffast-math -std=c99
CC = gcc
LIBS = -lm -lOpenCL
INCLUDES = -Iopencl11/
SRCS = opencl.c util.c liveness.c

all: liveness

# The variable $@ has the value of the target. In this case $@ = psort
liveness: opencl.o util.o liveness.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${SRCS} ${LIBS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

clean:
	rm -f *.o *~


.PHONY: all
# DO NOT DELETE
