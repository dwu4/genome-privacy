CC=g++
OT=ot
CFLAGS=-std=c++11 -O2 -g
LIBRARIES= util/Miracl/miracl.a -lssl -lcrypto
MIRACL_PATH= -I./util/Miracl
SOURCES_UTIL=util/*.cpp
OBJECTS_UTIL=util/*.o
SOURCES_PROTOCOL=protocol/OTServer.cpp protocol/OTClient.cpp protocol/OTParty.cpp
OBJECTS_PROTOCOL=protocol/OTServer.o protocol/OTClient.o protocol/OTParty.o
OBJECTS_OT=ot/*.o
OBJECTS_MIRACL= util/Miracl/*.o
OBJECTS_ALL=${OBJECTS_PROTOCOL} ${OBJECTS_UTIL} ${OBJECTS_OT}
BATCH=
INCLUDE=-I..

all: lib

lib: ${OBJECTS_ALL}
	mkdir -p ../lib
	ar cr ../lib/libot.a ${OBJECTS_ALL} ${OBJECTS_MIRACL}

${OBJECTS_PROTOCOL}: ${SOURCES_PROTOCOL}$
	@cd protocol; ${CC} -c ${INCLUDE} ${CFLAGS} *.cpp

${OBJECTS_UTIL}: ${SOURCES_UTIL}$
	@cd util; ${CC} -c ${INCLUDE} ${CFLAGS} ${BATCH} *.cpp

${OBJECTS_OT}: ${SOURCES_OT}$
	@cd ot; ${CC} -c ${INCLUDE} ${CFLAGS} ${BATCH} *.cpp

miracl:
	bash build_miracl.sh

clean:
	rm -rf *.exe ${OBJECTS_ALL} protocol/*.o ${LIBNAME}
