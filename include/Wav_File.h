#ifndef WAV
#define WAV

#include <cstring> // strcat()
#include <stdexcept>
#include <stdio.h>

#include "constants.h"

// Purposefully prevent copying of WavFile
// By deriving WavFile from Uncopyable, error from attempting to copy WavFile
// will be generated at compile time
class Uncopyable {
private:
  
  Uncopyable(const Uncopyable &);
  Uncopyable &operator=(const Uncopyable &);
 
public:
  Uncopyable() {}
  ~Uncopyable() {}
  
};

// Class contains operations used to access
// a wav file, including opening/closing the file,
// accessing it's header information and data etc.
class WavFile : private Uncopyable {
private:
 
  char* data_;
  char* filename_;
  FILE* wav_file_io_;
  
  WavHeaderFormat wav_header_;
  
  int ConvertToInt(char*);
  void ReadConfig();
  
public:
  WavFile(const char *filename) : 
    data_(NULL), wav_file_io_(NULL)
  { 
    
    if(filename){
      filename_ = new char[strlen(filename)];
      strcpy(filename_, filename);
    }
    
  }
  ~WavFile(){
    Close();
  }
  
  bool Open();
  bool Close();
  
  bool DisplayWavInfo() const;
  char* GetData() const;
  char* GetFilename() const;
  char operator[](int pos) const;
  int GetDataLength() const;
  int GetNumberChannels() const;
  int GetSampleRate() const;
  int GetSampleSize() const;
  unsigned int GetBytesPerSecond() const;

};

#endif //WAV