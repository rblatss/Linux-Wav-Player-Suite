﻿#include "../include/wav_file.h"

int WavFile::ConvertToInt(char* a)
{
  int result = 0;
  for (int i = 0; i < 4; i++)
  {
    result |= (a[i] << (3 - i) * 8);
  }
  return result;
}

bool WavFile::Open()
{

  if (data_)
  {
    printf("Wav file has already been read.\n");
    return false;
  }

  // Open File
  if (!(wav_file_io_ = fopen(filename_, "rb")))
  {
    printf("Error Reading from %s\n", filename_);
    return false;
  }

  // Read File Header Information and Verify Wav file is in correct format
  bool data_chunk = false;
  unsigned int bytes;
  fseek(wav_file_io_, 0, SEEK_SET);
  while (!data_chunk)
  {

    char strChunkID[5];
    strChunkID[4] = '\0';
    fread(strChunkID, sizeof(char), 4, wav_file_io_);

    unsigned int chunkid = ConvertToInt(strChunkID);
    switch ((WavChunks)chunkid)
    {
      case WavChunks::Format:
#ifdef DEBUG
        printf("Reading Wav fmt chunk ... \n");
#endif

        strcpy(wav_header_.fmt, strChunkID);
        fread(&wav_header_.fmt_size, sizeof(int), 1, wav_file_io_);
        fread(&wav_header_.audio_format, sizeof(short), 1, wav_file_io_);
        fread(&wav_header_.number_channels, sizeof(short), 1, wav_file_io_);
        fread(&wav_header_.sampling_frequency, sizeof(int), 1, wav_file_io_);
        fread(&wav_header_.bytes_per_second, sizeof(int), 1, wav_file_io_);
        fread(&wav_header_.block_align, sizeof(short), 1, wav_file_io_);
        fread(&wav_header_.bits_per_sample, sizeof(short), 1, wav_file_io_);
        if (wav_header_.fmt_size == 18)
        {
          char extra_data[2];
          fread(extra_data, 1, 2, wav_file_io_);
        }
        break;

      case WavChunks::RiffHeader:
#ifdef DEBUG
        printf("Reading Wav RIFF chunk ... \n");
#endif

        strcpy(wav_header_.riff, strChunkID);

        fread(&wav_header_.riff_size, sizeof(int), 1, wav_file_io_);
        fread(wav_header_.wave, sizeof(char), 4, wav_file_io_);
        break;

      case WavChunks::Data:
#ifdef DEBUG
        printf("Reading Wav Data chunk ... \n");
#endif

        data_chunk = true;

        strcpy(wav_header_.data, strChunkID);

        // Read Wav Data
        fread(&wav_header_.data_size, sizeof(int), 1, wav_file_io_);
        if (!(data_ = new uint8_t[wav_header_.data_size]))
        {
          fprintf(stderr, "Out of memory error\n");
          fclose(wav_file_io_);
          return false;
        }

        bytes = fread(data_, sizeof(char), wav_header_.data_size, wav_file_io_);

#ifdef DEBUG
        printf("Read %u bytes of WAV data. \n", bytes);
#endif

        if (ferror(wav_file_io_))
        {
          fprintf(stderr, "Error Reading from %s\n", filename_);
          fclose(wav_file_io_);
          return false;
        }

        if (feof(wav_file_io_))
        {
          fprintf(stderr, "End of file reached.\n");
          fclose(wav_file_io_);
          return false;
        }

        break;

      default:
#ifdef DEBUG
        printf("Skipping bad data ... \n");
#endif

        unsigned int skipsize;
        fread(&skipsize, sizeof(int), 1, wav_file_io_);
        fseek(wav_file_io_, skipsize, SEEK_CUR);

        break;
    }
  }

  return true;
}

bool WavFile::Close()
{
  if (wav_file_io_)
  {
    fclose(wav_file_io_);
    delete[] data_;
    return true;
  }
  else
  {
    return false;
  }
}

bool WavFile::DisplayWavInfo() const
{
  if (!wav_file_io_)
  {
    return false;
  }

  printf("\n");
  printf("RIFF header                : %c%c%c%c\n", wav_header_.riff[0], wav_header_.riff[1],
         wav_header_.riff[2], wav_header_.riff[3]);
  printf("RIFF size                  : %u\n", wav_header_.riff_size);

  printf("WAVE header                : %c%c%c%c\n", wav_header_.wave[0], wav_header_.wave[1],
         wav_header_.wave[2], wav_header_.wave[3]);

  printf("FMT                        : %c%c%c%c\n", wav_header_.fmt[0], wav_header_.fmt[1],
         wav_header_.fmt[2], wav_header_.fmt[3]);
  printf("FMT size                   : %u\n", wav_header_.fmt_size);
  printf("Audio Format               : %hu\n", wav_header_.audio_format);
  printf("Number of channels         : %hu\n", wav_header_.number_channels);
  printf("Sampling Rate              : %u\n", wav_header_.sampling_frequency);
  printf("Number of bytes per second : %u\n", wav_header_.bytes_per_second);
  printf("Block align                : %hu\n", wav_header_.block_align);
  printf("Number of bits per sample  : %hu\n", wav_header_.bits_per_sample);

  printf("Data string                : %c%c%c%c\n", wav_header_.data[0], wav_header_.data[1],
         wav_header_.data[2], wav_header_.data[3]);
  printf("Data length                : %u\n", wav_header_.data_size);

  return true;
}

uint8_t* WavFile::GetData() const { return data_; }

char* WavFile::GetFilename() const { return filename_; }

uint8_t WavFile::operator[](int pos) const { return data_[pos]; }

unsigned int WavFile::GetDataLength() const { return wav_header_.data_size; }

unsigned short WavFile::GetNumberChannels() const { return wav_header_.number_channels; }

unsigned int WavFile::GetSampleRate() const { return wav_header_.sampling_frequency; }

unsigned short WavFile::GetSampleSize() const
{
  return static_cast<unsigned short>(wav_header_.bits_per_sample / CHAR_BIT);
}

unsigned int WavFile::GetBytesPerSecond() const { return wav_header_.bytes_per_second; }
