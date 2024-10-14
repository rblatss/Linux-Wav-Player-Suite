#include <dirent.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#include "../include/play.h"
#include "../include/stop.h"
#include "../include/wav_file.h"

// Constants
constexpr uint8_t WAV_EXT_SIZE = 4;
constexpr uint8_t NULL_TERM_SIZE = 1;

void FindWavs(const char* curr_pwd, unsigned int plen, std::vector<char*>* wav_names,
              std::vector<char*>* wav_paths)
{
  DIR* contents;
  dirent* entry;
  int filename_len;
  char new_pwd[PATH_MAX + NULL_TERM_SIZE];

  // Ensure # characters of path below PATH_MAX limit
  if (plen > PATH_MAX)
  {
    printf("Error reading path (PATH_MAX reached).\n");
    return;
  }

  // Iterate thru entries in directory and process if DT_DIR or DT_REG
  contents = opendir(curr_pwd);
  if (contents == NULL)
  {
    printf("Error opening directory.\n");
    return;
  }

  while ((entry = readdir(contents)) != NULL)
  {

    // Skip current ('.'), parent ('..') and hidden folders/files
    if (entry->d_name[0] == '.')
    {
      continue;
    }

    // Recurse into directories.
    if (entry->d_type == DT_DIR)
    {
      strncpy(new_pwd, curr_pwd, PATH_MAX);
      new_pwd[plen] = '/';
      strncat(new_pwd, entry->d_name, PATH_MAX);
      FindWavs(new_pwd, plen + NULL_TERM_SIZE + strlen(entry->d_name), wav_names, wav_paths);
    }

    else if (entry->d_type == DT_REG)
    {
      // Check for the WAV file extension
      filename_len = strlen(entry->d_name);
      if (strncmp(&entry->d_name[filename_len - WAV_EXT_SIZE], ".wav", WAV_EXT_SIZE) == 0)
      {
        // Cache name of the file
        wav_names->push_back(entry->d_name);

        // Construct the path and cache it
        char* temp = new char[plen + NULL_TERM_SIZE + filename_len];
        strncpy(temp, curr_pwd, PATH_MAX - filename_len - 1);
        temp[plen] = '/';
        strncat(temp, entry->d_name, filename_len);
        wav_paths->push_back(temp);
      }
    }
  }

  closedir(contents);
}

int GetWavPath(char* filename, std::vector<char*>* wav_names)
{
  int index = 0;
  for (auto itr = wav_names->begin(); itr != wav_names->end(); itr++, index++)
  {
    if (strcmp(*itr, filename) == 0)
    {
      return index;
    }
  }

  return EOF;
}

void PrintListOfWavs(std::vector<char*>* wav_names)
{
  for (auto itr = wav_names->begin(); itr != wav_names->end(); itr++)
  {
    printf("%s\n", *itr);
  }
}

void GetMusicDirectoryPath(char* music_dir)
{
  const char* home_path = getenv("HOME");
  if (home_path)
  {
    strncpy(music_dir, home_path, PATH_MAX);
  }
  else
  {
    strncpy(music_dir, getpwuid(getuid())->pw_dir, PATH_MAX);
  }

  strncat((char*)music_dir, "/Music", PATH_MAX);
}

bool VerifyFilepath(char* arg)
{
  int len = strlen(arg);

  // The extension is 4 characters.
  if (len <= WAV_EXT_SIZE)
  {
    return false;
  }

  return strncmp(&arg[len - WAV_EXT_SIZE], ".wav", WAV_EXT_SIZE) == 0;
}

void PrintUsage(FILE* stream, int exit_code, const char* program_name)
{

  fprintf(stream, "Usage: %s [options] filename.wav\n", program_name);
  fprintf(stream, " -h --help           Display usage information.\n"
                  " -m --music-library  Use file from user's music directory.\n"
                  " -p --print          List available wav files.\n"
                  " -i --info           Print wav file information.\n");

  exit(exit_code);
}

int main(int argc, char* argv[])
{
  // For argument parsing
  int next_option;
  const char* short_opts = "imph";
  const struct option long_opts[5] = {{.name = "info", .has_arg = 0, .flag = NULL, .val = 'i'},
                                      {.name = "music", .has_arg = 0, .flag = NULL, .val = 'm'},
                                      {.name = "print", .has_arg = 0, .flag = NULL, .val = 'p'},
                                      {.name = "help", .has_arg = 0, .flag = NULL, .val = 'h'},
                                      {.name = NULL, .has_arg = 0, .flag = NULL, .val = 0}};

  // For m option
  int index;
  bool use_music_library = false;

  // For p option
  bool print_info_only = false;

  // For m / p options
  char filepath[PATH_MAX];
  std::vector<char*> wav_names;
  std::vector<char*> wav_paths;

  if (argc < 2)
  {
    PrintUsage(stderr, 1, argv[0]);
  }

  do
  {
    next_option = getopt_long(argc, argv, short_opts, long_opts, NULL);

    switch (next_option)
    {
      case 'h':
      case '?':
        PrintUsage(stdout, 0, argv[0]);

      case 'i':
        print_info_only = true;
        break;

      case 'm':
        use_music_library = true;
        GetMusicDirectoryPath(filepath);
        FindWavs(filepath, strlen(filepath), &wav_names, &wav_paths);
        index = GetWavPath(argv[optind], &wav_names);
        if (index == EOF)
        {
          printf("Can't find the file. Are you sure you spelled it correctly?\n\n");
          PrintUsage(stderr, 1, argv[0]);
        }
        break;

      case 'p':
        GetMusicDirectoryPath(filepath);
        FindWavs(filepath, strlen(filepath), &wav_names, &wav_paths);
        PrintListOfWavs(&wav_names);
        exit(0);
        break;

      case EOF:
        // Break at the end of options
        break;

      default:
        abort();
    }

  } while (next_option != EOF);

  // Handle music library option specially
  if (use_music_library)
  {
    strncpy(filepath, wav_paths[index], PATH_MAX);
  }
  else
  {
    strncpy(filepath, argv[optind], PATH_MAX);
  }

  // Verify final filepath
  if (VerifyFilepath(filepath) == false)
  {
    PrintUsage(stderr, 1, argv[0]);
  }

  // Handle the option to print the WAV file info specially
  int rc = 0;
  if(print_info_only == true)
  {
    WavFile theWavFile(filepath);
    if (theWavFile.Open() == false)
    {
      rc = 1;
    }
    else if (print_info_only == true)
    {
      theWavFile.DisplayWavInfo();
    }
  }

  // Otherwise, play
  else
  {
    rc = play(filepath);
  }

  return rc;
}
