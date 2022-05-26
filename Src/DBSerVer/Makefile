CXX := g++
CFLAGS := -std=c++14 -O1 -g
LIB := -lsqlite3
OBJ_DIR := ./obj
BIN_DIR := ./bin

OBJS = ${OBJ_DIR}/main.o ${OBJ_DIR}/singaldb.o ${OBJ_DIR}/cardb.o ${OBJ_DIR}/userdb.o \
	   ${OBJ_DIR}/dbserver.o

.PHONY: mk_dir bin clean

all: mk_dir bin

mk_dir:
	if [ ! -d ${OBJ_DIR}  ]; then mkdir ${OBJ_DIR};fi
	if [ ! -d ${BIN_DIR}  ]; then mkdir ${BIN_DIR};fi

bin: $(OBJS)
	${CXX} ${CFLAGS} ${OBJS} -o ./bin/dbserver ${LIB}
	
${OBJ_DIR}/main.o: ./main.cpp
	${CXX} ${CFLAGS} -o $@ -c $< ${LIB}

${OBJ_DIR}/singaldb.o: ./singaldb.cpp 
	${CXX} ${CFLAGS} -o $@ -c $< ${LIB}

${OBJ_DIR}/cardb.o: ./cardb.cpp 
	${CXX} ${CFLAGS} -o $@ -c $< ${LIB}

${OBJ_DIR}/userdb.o: ./userdb.cpp 
	${CXX} ${CFLAGS} -o $@ -c $< ${LIB}

${OBJ_DIR}/dbserver.o: ./dbserver.cpp
	${CXX} ${CFLAGS} -o $@ -c $< ${LIB}

clean:
	rm -rf ./bin ./obj 
