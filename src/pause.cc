#include "../include/pause.h"

#include <stdio.h>
#include <stdlib.h>

pid_t pause()
{
  char filename[] = "/tmp/pid.txt";
  char c;
  FILE* pid_file;
  int len;
  pid_t pid;
  int rc;

  // Extract the PID from the temp file.
  // Return if the file is empty
  pid_file = fopen(filename, "rb");
  if (!pid_file)
  {
    printf("No music to pause!\n");
    return -1;
  }

  rc = fscanf(pid_file, "%d", &pid);
  fclose(pid_file);

  // Signal the play application
  if(rc != 0)
  {
    kill(pid, SIGUSR1);
  }

  return pid;
}
