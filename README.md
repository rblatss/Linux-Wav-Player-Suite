# Linux-Wav-Programs-Suite
Description
-----------

An exercise in C++ to implement the functionality of a media player (play, stop, plot, ff, rewind etc.) into
the terminal. Written in the spirit of the unix hallmark: Write programs that do one thing, and do it well.

Currently, this "suite of programs" is only composed of *Play, Stop* and *Pause*. I do not have plans to
complete *Plot, ff* and *Rewind* at this time.

Build
-----

I compiled this with g++ version 4.8.4 on Ubuntu 14.04. With g++, run the Makefile from its directory with 
'make'. All binaries are written to the 'bin' folder.


A little bit about the unorthodox code organization...
------------------------------------------------------

To play .wav files, this application uses the [ALSA API](http://www.alsa-project.org/main/index.php/ALSA_Library_API), 
which is a C API. Initially, I developed this in C but resorted to C++ classes to implement functions to 
read/write .wav files.

Typically, C/C++ header files contain the declarations and prototypes for classes, structs, functions etc. The source
for this project is organized a little differently. Namely, the .cc files are the "drivers" of each application
(*Play, Stop, Pause*). They contain the main() function. The .h files contain the implementations of the functions
responsible for carrying out the core functionality of the applications. For example, play.h defines functions
that configure ALSA, read .wav file data, and write the .wav data to the buffer to play sound.
