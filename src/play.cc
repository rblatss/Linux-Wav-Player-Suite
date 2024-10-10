#include <alsa/asoundlib.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../include/pause.h"
#include "../include/play.h"
#include "../include/stop.h"
#include "../include/wav_file.h"

// Globals
constexpr uint8_t WAV_EXT_SIZE = 4;
constexpr unsigned int US_PER_S = 1000000U;
constexpr char PCM_DEVICE[] = "default";
int pcm_pause_enable_state = 0;
snd_pcm_t* pcm_handle;

// TODO error handling - how do you test this?
int ConfigureALSAAudio(snd_pcm_t* device, snd_pcm_hw_params_t*& hw_params, int channels,
                       int sample_rate, int sample_size)
{
  int err;
  int requested_rate;

  // Allocate memory for hardware parameter structure
  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
  {
    fprintf(stderr, "ERROR: cannot allocate parameter structure (%s)\n", snd_strerror(err));
    return 1;
  }

  // Fill structure from current audio parameters */
  if ((err = snd_pcm_hw_params_any(device, hw_params)) < 0)
  {
    fprintf(stderr, "ERROR: cannot initialize parameter structure (%s)\n", snd_strerror(err));
    return 1;
  }

  // Set access and format
  if (channels == 2)
  {
    if ((err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) <
        0)
    {
      fprintf(stderr, "ERROR: Can't set interleaved mode. %s\n", snd_strerror(err));
      return 1;
    }
    if ((err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
      fprintf(stderr, "ERROR: Can't set format. %s\n", snd_strerror(err));
      return 1;
    }
  }
  else if (channels == 1)
  {
    if ((err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_U8)) < 0)
    {
      fprintf(stderr, "ERROR: Can't set format. %s\n", snd_strerror(err));
      return 1;
    }
  }
  else
  {
    fprintf(stderr, "ERROR: %d channels not supported.\n", channels);
    return 1;
  }

  // Check if hardware supports pausing
  if (snd_pcm_hw_params_can_pause(hw_params) == 0)
  {
    fprintf(stderr, "ERROR: ALSA hardware device doesn't permit pausing.\n");
    snd_pcm_close(pcm_handle);
    return 1;
  }

  // Set number of channels
  if ((err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, channels)) < 0)
  {
    fprintf(stderr, "ERROR: Can't set channels number. %s\n", snd_strerror(err));
    return 1;
  }

  // Set sample rate
  requested_rate = sample_rate;
  if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, (unsigned int*)&requested_rate,
                                             0)) < 0)
  {
    fprintf(stderr, "ERROR: Can't set rate. %s\n", snd_strerror(err));
    return 1;
  }
  if (requested_rate != sample_rate)
  {
    fprintf(stderr, "Could not set requested sample rate, asked for %d got %d\n", sample_rate,
            requested_rate);
  }

  // Set the parameters
  if ((err = snd_pcm_hw_params(device, hw_params)) < 0)
  {
    fprintf(stderr, "Error setting HW params: %s\n", snd_strerror(err));
    return 1;
  }

  return 0;
}

void HandleSignal(int signal)
{

  // Critical Section to perform i/o
  sigset_t newMask, oldMask;
  sigemptyset(&newMask);
  sigemptyset(&oldMask);

  sigaddset(&newMask, signal);
  sigprocmask(SIG_BLOCK, &newMask, &oldMask);

  sigset_t pending;

  switch (signal)
  {

    // Stop or Terminal closes
    case SIGHUP:
      snd_pcm_drop(pcm_handle);
      snd_pcm_close(pcm_handle);
      break;

    // Pause
    case SIGUSR1:
      pcm_pause_enable_state = 1 - pcm_pause_enable_state;
      snd_pcm_pause(pcm_handle, pcm_pause_enable_state);
      break;

    case SIGINT:
      printf("Caught SIGINT, exiting now\n");
      exit(0);

    default:
      fprintf(stderr, "Caught wrong signal: %d\n", signal);
      return;
  }

  // End of critical section
  sigprocmask(SIG_SETMASK, &oldMask, NULL);
}

bool WritePIDRecord(pid_t pid)
{
  FILE* pid_file = fopen("/tmp/pid.txt", "wb");
  if (pid_file == NULL)
  {
    printf("Failed to create pid.txt! %s", strerror(errno));
    return false;
  }
  fprintf(pid_file, "%d", pid);
  fclose(pid_file);
  return true;
}

bool DeletePIDRecord()
{
  char pid_file[] = "/tmp/pid.txt";
  if (remove(pid_file) != 0)
  {
    printf("Failed to remove pid.txt! %s", strerror(errno));
    return false;
  }
  return true;
}

void FindWavs(const char* curr_pwd, unsigned int plen, std::vector<char*>* wav_names,
              std::vector<char*>* wav_paths)
{
  DIR* contents;
  dirent* entry;
  int filename_len;
  char new_pwd[PATH_MAX + 1];

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

      FindWavs(new_pwd, plen + 1 + strlen(entry->d_name), wav_names, wav_paths);
    }

    else if (entry->d_type == DT_REG)
    {
      // Check for the WAV file extension
      filename_len = strlen(entry->d_name);
      if (strncmp(&entry->d_name[filename_len - 4], ".wav", WAV_EXT_SIZE) == 0)
      {
        // Cache name of the file
        wav_names->push_back(entry->d_name);

        // Construct the path and cache it
        char* temp = new char[plen + 1 + filename_len];
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
  if (len < 5)
  {
    return false;
  }

  return strncmp(&arg[len - 4], ".wav", WAV_EXT_SIZE) == 0;
}

int play(char* filename, bool print_info_only)
{
  WavFile theWavFile(filename);
  unsigned int pcm, period_us;
  snd_pcm_hw_params_t* params;
  snd_pcm_uframes_t frames;
  uint8_t* buff;
  int buff_size, err;

  // Open the WAV file
  if (theWavFile.Open() == false)
  {
    return 1;
  }

  // Print WAV file info
  if (print_info_only == true)
  {
    theWavFile.DisplayWavInfo();
    return 0;
  }

  // Get Wav info
  int channels = theWavFile.GetNumberChannels();
  int data_length = theWavFile.GetDataLength();
  int sample_rate = theWavFile.GetSampleRate();
  int sample_size = theWavFile.GetSampleSize();
  double seconds = (double)theWavFile.GetDataLength() / theWavFile.GetBytesPerSecond();

  // Open the PCM device in playback mode
  if ((pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
  {
    fprintf(stderr, "ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE, snd_strerror(pcm));
  }

  // Configure the device
  if (ConfigureALSAAudio(pcm_handle, params, channels, sample_rate, sample_size) != 0)
  {
    return 1;
  }

  // Get period info
  snd_pcm_hw_params_get_period_time(params, &period_us, NULL);
  snd_pcm_hw_params_get_period_size(params, &frames, 0);

  // Allocate buffer to hold single period
  buff_size = frames * channels * sample_size; // e.g. 1024 * 2 * 2
  buff = new uint8_t[buff_size];

#ifdef DEBUG
  printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));
  printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

  unsigned int tmp;
  snd_pcm_hw_params_get_channels(params, &tmp);
  printf("channels: %i ", tmp);
  printf(tmp == 1 ? "(mono)\n" : "(stereo)\n");
  snd_pcm_hw_params_get_rate(params, &tmp, 0);
  printf("rate: %d bps\n", tmp);
  printf("seconds: %f\n", seconds);
  printf("period time (us): %u\n", period_us);
  printf("period size (frames): %llu\n", frames);
#endif

  // TODO convert interleaved to non-interleaved?

  int index = 0;
  for (int loops = static_cast<int>((seconds * US_PER_S) / period_us) + 1; loops > 0; loops--)
  {

    // Fill buffer with a period's worth of samples
    buff_size = std::min(buff_size, data_length - index);
    memcpy(buff, &theWavFile.GetData()[index], buff_size);

    // Write buffer to PCM
    if (channels == 1)
    {
      // TODO
      pcm = snd_pcm_writen(pcm_handle, (void **) &buff, buff_size);
    }
    else
    {
      pcm = snd_pcm_writei(pcm_handle, buff, frames); // TODO frames is probably wrong in the last loop
    }

    // Check for errors
    if (pcm == -EPIPE)
    {
      printf("UNDERRUN.\n");
      snd_pcm_prepare(pcm_handle);
    }
    else if (pcm < 0)
    {
      printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
    }

    // Increment index
    index += buff_size;
  }

  // Play wav
  snd_pcm_start(pcm_handle);

  // Hang here until playthrough ends
  do
  {
    err = snd_pcm_wait(pcm_handle, -1);
  } while (err != 1);

  // Clean up
  snd_pcm_drain(pcm_handle);
  snd_pcm_close(pcm_handle);
  free(buff);

  return 0;
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
  const struct option long_opts[] = {{"info", 0, NULL, 'i'},
                                     {"music", 0, NULL, 'm'},
                                     {"print", 0, NULL, 'p'},
                                     {"help", 0, NULL, 'h'},
                                     {NULL, 0, NULL, 0}};

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
        break;

      case EOF:
        // Break at the end of options
        break;

      default:
        abort();
    }

  } while (next_option != EOF);

  // The main program executes as a child process
  int rc = 0;
  int pid = fork();
  if (pid == 0)
  {
    // Stop music that's currently playing
    if (access("/tmp/pid.txt", F_OK) == 0)
    {
      stop();
    }

    // Setup for signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &HandleSignal;

    // Pause
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
      perror("Error: cannot handle SIGUSR1"); // Should not happen
    }

    // ff or rewind
    if (sigaction(SIGUSR2, &sa, NULL) == -1)
    {
      perror("Error: cannot handle SIGUSR2"); // Should not happen
    }

    // Stop or Terminal Exits
    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
      perror("Error: cannot handle SIGHUP"); // Should not happen
    }

    // Get filepath and verify it
    if (use_music_library)
    {
      strncpy(filepath, wav_paths[index], PATH_MAX);
    }
    else
    {
      strncpy(filepath, argv[optind], PATH_MAX);
    }

    if (!VerifyFilepath(filepath))
    {
      PrintUsage(stderr, 1, argv[0]);
    }

    // Attempt playing the file
    pid_t this_pid = getpid();
    if (WritePIDRecord(this_pid))
    {

      // Enter main program
      rc = play(filepath, print_info_only);

      DeletePIDRecord();
    }
  }

  return rc;
}
