# Custom makefile to integrate nauty's graph generator "geng" in the Brill–Noether tests.
# 
# Please change NAUTY_DIR to the directory containing nauty, and CODEBASE_DIR to the
# directory containing my header files "graphs.h", "divisors.h", "graph6.h", and
# "approximate_independent_sets.h".

NAUTY_DIR=~/Downloads/nauty27r1
CODEBASE_DIR=../

CFLAGS += -O4 -march=native -DMAXN=32 -DWORDSIZE=64 -DOUTPROC=myoutproc -DGENG_MAIN=geng_main -I"${NAUTY_DIR}/" -I"${CODEBASE_DIR}"
CXXFLAGS += ${CFLAGS} -Wall -Wextra -ggdb

CCOBJ=${CC} -c ${CFLAGS} -o $@

all: Brill_Noether_geng

geng.o: ${NAUTY_DIR}/geng.c
	${CCOBJ} ${NAUTY_DIR}/geng.c

Brill_Noether_geng: geng.o ${NAUTY_DIR}/nautyL.a

clean:
	-rm -rf geng.o Brill_Noether_geng
