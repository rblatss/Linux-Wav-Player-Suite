CC=g++
AR=ar
CFLAGS=-std=c++11  # -D DEBUG
IPATH=include/
LIBS=-lpthread -lm -lasound -ldl
PLAY_SRC=src/Play.cc src/Wav_File.cc

# Order is important. The libraries must be built before building the play executable.
all: setup stop pause play

.PHONY: setup
setup:
	mkdir -p build

# Build stop and pause as executables and also libraries for linking with play.
stop : src/Stop.cc src/driver_stop.c
	$(CC) -I $(IPATH) src/Stop.cc -c -o build/stop.o
	$(AR) r build/libstop.a build/stop.o
	$(CC) -B build/ -I $(IPATH) src/driver_stop.c build/stop.o -o build/stop -lstop

pause : src/Pause.cc src/driver_pause.c
	$(CC) -I $(IPATH) src/Pause.cc -c -o build/pause.o
	$(AR) r build/libpause.a build/pause.o
	$(CC) -B build/ -I $(IPATH) src/driver_pause.c build/pause.o -o build/pause -lpause

play : $(PLAY_SRC)
	$(CC) -B build/ -I $(IPATH) $(CFLAGS) $(PLAY_SRC) -o build/play $(LIBS) -lstop -lpause

.PHONY : clean
clean:
	-rm -rf build
