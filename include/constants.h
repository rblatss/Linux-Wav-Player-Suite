#ifndef CONSTANTS
#define CONSTANTS

#include <climits>
#include <cmath>

const int PI = 3.14159265359;


// C++11 scoped enums
enum class WavChunks {
    RiffHeader = 0x52494646,
    WavRiff = 0x54651475,
    Format = 0x666d7420,
    LabeledText = 0x478747C6,
    Instrumentation = 0x478747C6,
    Sample = 0x6C706D73,
    Fact = 0x47361666,
    Data = 0x64617461,
    Junk = 0x4b4e554a
};

enum class WavFormat {
    PulseCodeModulation = 0x01,
    IEEEFloatingPoint = 0x03,
    ALaw = 0x06,
    MuLaw = 0x07,
    IMAADPCM = 0x11,
    YamahaITUG723ADPCM = 0x16,
    GSM610 = 0x31,
    ITUG721ADPCM = 0x40,
    MPEG = 0x50,
    Extensible = 0xFFFE
};

// Struct written according to http://soundfile.sapp.org/doc/WaveFormat/wav-sound-format.gif 
typedef struct WavHeader{
  // RIFF Chunk
  char                riff[4];            // RIFF Header
  unsigned int	      riff_size;          // RIFF Chunk Size  
  char                wave[4];            // WAVE Header      

  // Sub Chunk 1
  char                fmt[4];             // FMT header       
  unsigned int	      fmt_size;           // Size of the fmt chunk                                
  unsigned short      audio_format;       // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 
  unsigned short      number_channels;    // Number of channels 1=Mono 2=Stereo                   
  unsigned int        sampling_frequency; // Sampling Frequency in Hz                             
  unsigned int        bytes_per_second;   // Bytes per second 
  unsigned short      block_align;        // 2=16-bit mono, 4=16-bit stereo
  unsigned short      bits_per_sample;    // Number of bits per sample      
  
  // Sub Chunk 2
  char                data[4];            // "Data" ID
  unsigned int        data_size;          // Sampled data length    
} WavHeaderFormat; 

#endif // CONSTANTS