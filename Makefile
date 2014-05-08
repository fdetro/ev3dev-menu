EV3DEV_LANG=../lang/cpp
EV3DEV_GLIB=../glib

CFLAGS=-O2 -march=armv5 -I${EV3DEV_LANG} -I${EV3DEV_GLIB}/csrc
CCFLAGS=-std=c++11 -D_GLIBCXX_USE_NANOSLEEP

LIBS=-lstdc++ -lpthread -lev3dev-glib -L${EV3DEV_GLIB}/lib

DEPS=${EV3DEV_LANG}/ev3dev.h
OBJ=${EV3DEV_LANG}/ev3dev.o src/main.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(CCFLAGS)

ev3dev-menu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(CCFLAGS) $(LIBS)

.PHONY: all clean

clean:
	-rm $(OBJ) ev3dev-menu

all:  ev3dev-menu
