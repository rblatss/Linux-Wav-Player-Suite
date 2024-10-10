#ifndef WAV
#define WAV

#include <cstring> // strcat()
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>

#include "constants.h"

// Purposefully prevent copying of WavFile
// By deriving WavFile from Uncopyable, error from attempting to copy WavFile
// will be generated at compile time
class Uncopyable
{
  private:
  Uncopyable(const Uncopyable&);
  Uncopyable& operator=(const Uncopyable&);

  public:
  Uncopyable() {}
  ~Uncopyable() {}
};

// Class contains operations used to access
// a wav file, including opening/closing the file,
// accessing it's header information and data etc.
class WavFile : private Uncopyable
{
  private:
  uint8_t* data_;
  char* filename_;
  FILE* wav_file_io_;

  WavHeaderFormat wav_header_;

  // Convert Wav id from char array to integer
  int ConvertToInt(char*);

  public:
  WavFile(const char* filename) : data_(NULL), wav_file_io_(NULL), filename_(NULL)
  {
    if (filename != NULL)
    {
      filename_ = new char[strlen(filename)];
      strcpy(filename_, filename);
    }
  }
  ~WavFile() { Close(); }

  /*
   * Open:
   *
   * Read the WAV file's header info and data.
   *
   * Returns:
   *   true on success
   *   false on error
   */
  bool Open();

  /*
   * Close:
   *
   * Close stream to WAV file and free memory.
   *
   * Returns:
   *   true on success,
   *   false otherwise.
   */
  bool Close();

  /*
   * DisplayWavInfo:
   *
   * Print WAV file header info.
   *
   * Returns:
   *   true on success,
   *   false otherwise.
   *
   */
  bool DisplayWavInfo() const;

  /* Accessors */
  uint8_t* GetData() const;
  char* GetFilename() const;
  uint8_t operator[](int pos) const;
  int GetDataLength() const;
  int GetNumberChannels() const;
  int GetSampleRate() const;
  int GetSampleSize() const;
  unsigned int GetBytesPerSecond() const;
};

#endif // WAV