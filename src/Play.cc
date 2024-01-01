/* ***********************************************
 *                                               *
 *                     Play                      *
 *                Robert Blatner                 *
 *                                               *
 * ******************************************** */

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../include/Play.h"
#include "../include/Stop.h"

// Entry Point
int main(int argc, char *argv[])
{

  int next_option;
  bool display_info = false;
  bool from_music = false;
  const char *short_opts = "imph";

  // For -m option
  char filename[PATH_MAX];
  int index;

  const struct option long_opts[] = {
      {"info", 0, NULL, 'i'},
      {"music", 0, NULL, 'm'},
      {"print", 0, NULL, 'p'},
      {"help", 0, NULL, 'h'},
      {NULL, 0, NULL, 0}};

  program_name = argv[0];

  if (argc < 2)
    PrintUsage(stderr, 1);

  do
  {
    next_option = getopt_long(argc, argv, short_opts, long_opts, NULL);

    switch (next_option)
    {
    case 'h':
      PrintUsage(stdout, 0);

    case 'i':
      display_info = true;
      break;

    case 'm':
      from_music = true;

      GetMusicDir(filename);
      FindWavs(filename, strlen(filename));
      index = GetWavPath(argv[optind]);
      if (index == -1)
      {
        printf("Can't find the file. Sure you spelled it correctly?\n\n");
        PrintUsage(stderr, 1);
      }

      break;

    case 'p':

      GetMusicDir(filename);
      printf("%s\n", filename);
      FindWavs(filename, strlen(filename));
      PrintListOfWavs();
      break;

    case '?':
      PrintUsage(stderr, 1);

    case -1:
      break;

    default:
      abort();
    }

  } while (next_option != -1);

  int rc = 0;
  int pid = fork();

  // Main Program executes as child process
  if (pid == 0)
  {

    // Check if already playing, if so replace it
    if (access("/tmp/pid.txt", F_OK != 0))
      Stop();

    // Setup for signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &HandleSignal;

    // Pause
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
      perror("Error: cannot handle SIGUSR1"); // Should not happen

    // ff or rewind
    if (sigaction(SIGUSR2, &sa, NULL) == -1)
      perror("Error: cannot handle SIGUSR2"); // Should not happen

    // Stop or Terminal Exits
    if (sigaction(SIGHUP, &sa, NULL) == -1)
      perror("Error: cannot handle SIGHUP"); // Should not happen

    if (WritePIDRecord(getpid()))
    {

      if (from_music)
        strcpy(filename, wav_paths[index]);
      else
        strcpy(filename, argv[optind]);

      if (!ValidateFilename(filename))
        PrintUsage(stderr, 1);

      rc = Play(filename, display_info); // main program

      DeletePIDRecord();
    }
  }

  return rc;
}
