# Linux-Wav-Programs-Suite
Description
-----------

An exercise in C++ to implement the functionality of a media player (play, stop, plot, ff, rewind etc.) into the terminal. Written in the spirit of the unix hallmark: Write programs that do one thing, and do it well.

Currently, this "suite of programs" is only composed of *Play, Stop* and *Pause*. I do not have plans to complete *Plot, ff* and *Rewind* at this time.

Work in progress: I created a Dockerfile so that this project can be built and demonstrated anywhere. My plan is to figure out how to invoke these apps from within the Docker container by making the audio devices available to the container. Then, I'd also like to create some demonstrational tests. No plans for anything else.

Setup
------------
Install Docker, then
```
docker pull gcc
docker build . -t gcc-alsa
```

Build
-----

Enter the docker container and run make:
```
docker run -it --rm gcc-alsa:latest
make
```


A little bit about the unorthodox code organization...
------------------------------------------------------

To play .wav files, this application uses the [ALSA API](http://www.alsa-project.org/main/index.php/ALSA_Library_API), which is a C API. Initially, I developed this in C but resorted to C++ classes to implement functions to read/write .wav files.

Typically, C/C++ header files contain the declarations and prototypes for classes, structs, functions etc. The source for this project is organized a little differently. Namely, the .cc files are the "drivers" of each application (*Play, Stop, Pause*). They contain the main() function. The .h files contain the implementations of the functions responsible for carrying out the core functionality of the applications. For example, play.h defines functions that configure ALSA, read .wav file data, and write the .wav data to the buffer to play sound.
