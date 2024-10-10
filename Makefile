CC=g++
AR=ar
# Pass in "CFLAGS='-D DEBUG'" for debug build
CFLAGS?=
CFLAGS+=-std=c++11
IPATH=include/
LIBS=-lpthread -lm -lasound -ldl
PLAY_SRC=src/play.cc src/wav_file.cc

# Order is important. The libraries must be built before building the play executable.
all: setup stop pause play

.PHONY: setup
setup:
	mkdir -p build

# Build stop and pause as executables and also libraries for linking with play.
stop : src/stop.cc src/driver_stop.c
	$(CC) -I $(IPATH) src/stop.cc -c -o build/stop.o
	$(AR) r build/libstop.a build/stop.o
	$(CC) -B build/ -I $(IPATH) src/driver_stop.c build/stop.o -o build/stop -lstop

pause : src/pause.cc src/driver_pause.c
	$(CC) -I $(IPATH) src/pause.cc -c -o build/pause.o
	$(AR) r build/libpause.a build/pause.o
	$(CC) -B build/ -I $(IPATH) src/driver_pause.c build/pause.o -o build/pause -lpause

play : $(PLAY_SRC)
	$(CC) -B build/ -I $(IPATH) $(CFLAGS) $(PLAY_SRC) -o build/play $(LIBS) -lstop -lpause

.PHONY : clean
clean:
	-rm -rf build
