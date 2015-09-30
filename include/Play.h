#include <alsa/asoundlib.h>
#include <dirent.h>
#include <linux/limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "../include/Wav_File.h"

// Globals
const char *PCM_DEVICE = "default";

extern snd_pcm_t *pcm_handle;

const char *program_name;
snd_pcm_t *pcm_handle;

int ppause = 0;

std::vector<char *> wav_names;
std::vector<char *> wav_paths;

int ConfigureALSAAudio(snd_pcm_t *device, snd_pcm_hw_params_t *&hw_params, int channels, int sample_rate, int sample_size) {
  int                 err;
  int                 tmp;
  snd_pcm_uframes_t   frames;
  int                 fragments = 2;
  int                 frame_size;
  int                 buffer_size = 512; // request this size
  
  /* allocate memory for hardware parameter structure */ 
  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) { 
    fprintf (stderr, "cannot allocate parameter structure (%s)\n",
             snd_strerror(err));
    return 1;
  }
 
  /* fill structure from current audio parameters */
  if ((err = snd_pcm_hw_params_any(device, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize parameter structure (%s)\n",
             snd_strerror(err));
    return 1;
  }
  
   /* Set access and format */
  if(channels == 2) {
    if (err = snd_pcm_hw_params_set_access(pcm_handle, hw_params,
                                       SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
      fprintf(stderr, "ERROR: Can't set interleaved mode. %s\n", snd_strerror(err));
      return 1;
    }
    if (err = snd_pcm_hw_params_set_format(pcm_handle, hw_params,
 				 SND_PCM_FORMAT_S16_LE) < 0){
      fprintf(stderr, "ERROR: Can't set format. %s\n", snd_strerror(err));
      return 1;
    }
  }
  else if(channels == 1){
    if (err = snd_pcm_hw_params_set_format(pcm_handle, hw_params,
 				  SND_PCM_FORMAT_U8) < 0){
      fprintf(stderr, "ERROR: Can't set format. %s\n", snd_strerror(err));
      return 1;
    }
  }
  else {
    fprintf(stderr, "%d channels not supported.\n", channels); 
    return 1;
  }

  /* Check if hardware supports pausing */
  if(!snd_pcm_hw_params_can_pause(hw_params)){
    fprintf(stderr, "ERROR: ALSA hardware device doesn't permit pausing.\n");
    snd_pcm_close(pcm_handle);
    return 1;
  }
  
  /* Set number of channels */
  if (err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, channels) < 0){
    fprintf(stderr, "ERROR: Can't set channels number. %s\n", snd_strerror(err));
    return 1;
  }
    
  
  /* Set sample rate */
  tmp = sample_rate;    
  if (err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, (unsigned int *)&tmp, 0) < 0) {
    fprintf(stderr, "ERROR: Can't set rate. %s\n", snd_strerror(err));
    return 1;
  }
  if (tmp != sample_rate) {
    fprintf(stderr, "Could not set requested sample rate, asked for %d got %d\n", sample_rate, tmp);
    sample_rate = tmp;
  }
  
  /* Set the parameters */
  if ((err = snd_pcm_hw_params(device, hw_params)) < 0) {
    fprintf(stderr, "Error setting HW params: %s\n",
            snd_strerror(err));
    return 1;
  }
 
  return 0;
}

// Open Wav file, setup/play via ALSA API
int Play(char *filename, bool display_info){

  /* ************ Wav File ************** */
  
  WavFile theWavFile(filename);
  
  if( !theWavFile.Open() )
    return 1;
  
  if(display_info){
    theWavFile.DisplayWavInfo(); 
    return 0;
  }
  
  unsigned int pcm, tmp;
  int rate, channels, data_length, sample_size;
  double seconds;
  snd_pcm_hw_params_t *params;
  snd_pcm_uframes_t frames;
  char *buff, *data;
  int i;
  int buff_size, err, index, loops;
  
  // Get Wav info
  channels = theWavFile.GetNumberChannels();
  data_length = theWavFile.GetDataLength();
  rate = theWavFile.GetSampleRate();
  sample_size = theWavFile.GetSampleSize();
  seconds = (double)theWavFile.GetDataLength()/theWavFile.GetBytesPerSecond();
  
  /* Open the PCM device in playback mode */
  if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
  			SND_PCM_STREAM_PLAYBACK, 0) < 0) 
    fprintf(stderr, "ERROR: Can't open \"%s\" PCM device. %s\n",
	    PCM_DEVICE, snd_strerror(pcm));

  if(ConfigureALSAAudio(pcm_handle, params, channels, rate, sample_size) != 0) return 1;
  
#ifdef DEBUG
  printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));
  printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));
  
  snd_pcm_hw_params_get_channels(params, &tmp);
  printf("channels: %i ", tmp);
  
  snd_pcm_hw_params_get_rate(params, &tmp, 0);
  
  if (tmp == 1)
    printf("(mono)\n");
  else if (tmp == 2)
    printf("(stereo)\n");
  printf("rate: %d bps\n", tmp);  
  printf("seconds: %f\n", seconds);	
#endif
  
  /* Allocate buffer to hold single period */
  snd_pcm_hw_params_get_period_size(params, &frames, 0);
  
  buff_size = frames * channels * sample_size; // 1024*2*2
  
  buff = (char *) malloc(buff_size);
  
  snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
  
  for (loops = 1000000*seconds / tmp + 1, index = 0; loops >= 0; loops--, index += buff_size) {
    
    buff_size = buff_size > data_length - index ? data_length - index : buff_size;
    
    for(i = index; i < index + buff_size; i++)
      buff[i - index] = theWavFile[i];
    
    //memcpy(buff, data + index, buff_size);
    
    if (pcm = snd_pcm_writei(pcm_handle, buff, frames) == -EPIPE) {
      printf("XRUN.\n");
      snd_pcm_prepare(pcm_handle);
    }
    else if (pcm < 0) {
      printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
    }
  }

  // Play wav
  snd_pcm_start(pcm_handle);
  
  // Loop used to resume play if interrupted by signal
  do 
  {
  
    err = snd_pcm_wait(pcm_handle, -1);
  
  } while(err != 1);
  
  snd_pcm_drain(pcm_handle);
  
  // Clean up
  snd_pcm_close(pcm_handle);
  free(buff);
   
  return 0;
}

// Handle signal
void HandleSignal(int signal) {
   
  // Critical Section to perform i/o
  sigset_t newMask, oldMask;
  sigemptyset(&newMask);
  sigemptyset(&oldMask);
   
  sigaddset(&newMask, signal);
  sigprocmask(SIG_BLOCK, &newMask, &oldMask);
     
  const char *signal_name;
  sigset_t pending;

  switch (signal) {
    
    // Stop or Terminal closes
    case SIGHUP:
      signal_name = "SIGHUP";
      snd_pcm_drop(pcm_handle);
      snd_pcm_close(pcm_handle);
      
      break;
      
    // Pause
    case SIGUSR1:
      signal_name = "SIGUSR1";
      ppause = 1 - ppause;
      snd_pcm_pause(pcm_handle, ppause);
   
      break;
      
    // ff or rewind??
    case SIGUSR2:
      signal_name = "SIGUSR2";
      
      break;
    case SIGINT:
      printf("Caught SIGINT, exiting now\n");
      exit(0);
    default:
      fprintf(stderr, "Caught wrong signal: %d\n", signal);
      return;
  }  
  
#ifdef DEBUG
  printf("Caught %s. Cleaned up.\n", signal_name);
#endif
  
  // End of critical section    
  sigprocmask(SIG_SETMASK, &oldMask, NULL);
 
}

// Print usage
void PrintUsage(FILE *stream, int exit_code){
 
  fprintf(stream, "Usage: %s [options] filename.wav\n", program_name);
  fprintf(stream,
	  " -h --help           Display usage information.\n"
	  " -m --music-library  Use file from user's music directory.\n"
	  " -p --print          List available wav files.\n"
	  " -i --info           Print wav file information.\n");
  
  exit(exit_code);
}

// Write PID of Play to pid.txt
bool WritePIDRecord(pid_t pid){
  
  FILE *pid_file = fopen("/tmp/pid.txt", "wb"); 
  
  if(!pid_file) {
    printf("Uh oh .. something went wrong writing to pid.txt! %s", strerror(errno));
    return false;
  }
  
  fprintf(pid_file, "%d", pid);
  
  fclose(pid_file);
  return true;
}

// Delete pid.txt after play
bool DeletePIDRecord(){
  
  char pid_file[] = "/tmp/pid.txt"; 
 
  if( remove(pid_file) != 0 ){
    perror("Error");
    return false;  
  }
   
  return true;
}

void FindWavs(const char *curr_pwd, unsigned int plen){
  
  DIR *contents;
  dirent* entry;
  int filename_len;
  int i;
  char new_pwd[PATH_MAX + 1];
  
  
  // Ensure # characters of path below PATH_MAX limit
  if(plen > PATH_MAX){
    printf("Error reading path (PATH_MAX reached).\n");
    return;
  }
  
  // Iterate thru entries in directory and process if DT_DIR or DT_REG
  contents = opendir(curr_pwd);
  if(contents == NULL) {
    printf("Error opening directory.\n");
    return;
  }
 
  while( entry = readdir(contents) ){
    
    if(entry->d_name[0] == '.') // skip current ('.'), parent ('..') and hidden folders/files
      continue;
    
    if(entry->d_type == DT_DIR){ // If directory, recurse
      
      strcpy(new_pwd, curr_pwd);
      new_pwd[plen] = '/';
      strcat(new_pwd, entry->d_name);
	
      FindWavs(new_pwd, plen + 1 + strlen(entry->d_name));
      
    }
    
    else if(entry->d_type == DT_REG) { // check if wav
      
      filename_len = strlen(entry->d_name);
      if(strcmp( &entry->d_name[filename_len - 4], ".wav") == 0){
	
	wav_names.push_back(entry->d_name);
	
	char *temp = new char[plen + 1 + filename_len];
	
        strcpy(temp, curr_pwd);
        temp[plen] = '/';
        strcat(temp, entry->d_name);

	wav_paths.push_back(temp);

      }
    }
  }
  
  closedir(contents); 
}

// Return path at index of vector element that corresponds to filename argument
int GetWavPath(char *filename){
  std::vector<char *>::iterator itr = wav_names.begin();
  int index = 0;
  
  for(itr; itr != wav_names.end(); itr++, index++){
      
    if(strcmp(*itr, filename) == 0)
      return index;
  }
  
  return -1;
}

void PrintListOfWavs(){
  
  std::vector<char *>::iterator itr = wav_names.begin();
  for(itr; itr != wav_names.end(); itr++){
    printf("%s\n", *itr);
  }
}

void GetMusicDir(char *music_dir){
  
  if ( getenv("HOME") == NULL) 
    strcpy(music_dir, getpwuid(getuid())->pw_dir);
  else
    strcpy(music_dir, getenv("HOME"));
  
  strcat((char *)music_dir, "/Music");   
}

// Check filename (needs ".wav" extension)
bool ValidateFilename(char *arg){
  
  int len = strlen(arg);
  
  if( len < 5 ) return false; // ".wav" is invalid
  else if( strcmp( &arg[len - 4], ".wav") != 0) return false;
  
  return true;  
}