CC=LD_LIBRARY_PATH=/usr/local/lib g++
CFLAGS=-c -Wall -std=c++0x
LDFLAGS=-L/usr/lib64 -lodbc -lgcrypt -lSDL2 -lSDL2_net
SOURCES=svmain.cpp\
../common/net/net.cpp\
../common/net/netconn.cpp\
../common/net/readpackets.cpp\
../common/net/sendpackets.cpp\
../common/sim/simflow.cpp\
../common/window.cpp\
../common/utils.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=csmm

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o csmm