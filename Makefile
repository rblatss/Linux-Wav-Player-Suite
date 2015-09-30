CC=g++
#add -D DEBUG to CFLAGS for DEBUG output
CFLAGS=-std=c++11 
IPATH=include/
LIBS=-lpthread -lm -lasound
PLAY_SRC=src/Play.cc src/Wav_File.cc
STOP_SRC=src/Stop.cc
PAUSE_SRC=src/Pause.cc

all: play stop pause

play : $(PLAY_SRC)
	$(CC) -I $(IPATH) $(CFLAGS) $(PLAY_SRC) -o bin/Play $(LIBS)

stop : $(STOP_SRC)
	$(CC) -I $(IPATH) $(STOP_SRC) -o bin/Stop

pause : $(PAUSE_SRC)
	$(CC) -I $(IPATH) $(PAUSE_SRC) -o bin/Pause

.PHONY : clean
clean:
	-rm *.o
