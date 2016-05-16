DEBUG ?= 1

FLAGS= -std=c++11
LIBS= -lssl -lcrypto -pthread

ifeq ($(DEBUG), 1)
	FLAGS += -DDEBUG
endif



all: chord_emu run

chord_emu: lib.cpp chord_emu.cpp defines.h
	g++ ${FLAGS} -o $@ $^ $(LIBS)

run: run.cpp lib.cpp defines.h
	g++ ${FLAGS} -o $@ $^ $(LIBS)
	
clean: 
	rm chord_emu run
