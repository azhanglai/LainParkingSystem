CXX := g++
CFLAGS := -std=c++14 -O1 -g
LIB := -pthread
OBJ_DIR := ./obj
BIN_DIR := ./bin

OBJS = ${OBJ_DIR}/main.o ${OBJ_DIR}/epollserver.o ${OBJ_DIR}/epoller.o

.PHONY: mk_dir bin clean

all: mk_dir bin

mk_dir:
	if [ ! -d ${OBJ_DIR}  ]; then mkdir ${OBJ_DIR};fi
	if [ ! -d ${BIN_DIR}  ]; then mkdir ${BIN_DIR};fi

bin: $(OBJS)
	${CXX} ${CFLAGS} ${OBJS} -o ./bin/epollserver ${LIB}
	
${OBJ_DIR}/main.o: ./main.cpp
	${CXX} ${CFLAGS} -o $@ -c $<

${OBJ_DIR}/epollserver.o: ./epollserver.cpp 
	${CXX} ${CFLAGS} -o $@ -c $<

${OBJ_DIR}/epoller.o: ./epoller.cpp
	${CXX} ${CFLAGS} -o $@ -c $<

clean:
	rm -rf ./bin ./obj 
