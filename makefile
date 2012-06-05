CFLAGS = -g -Wall -Wextra -Werror -O2 -ffast-math -std=c99
CC = gcc
LIBS = -lm -lOpenCL
INCLUDES = -Iopencl11/
SRCS = opencl.c util.c sample.c

all: sample

# The variable $@ has the value of the target. In this case $@ = psort
sample: opencl.o util.o sample.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${SRCS} ${LIBS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

clean:
	rm -f *.o *~


.PHONY: all
# DO NOT DELETE
