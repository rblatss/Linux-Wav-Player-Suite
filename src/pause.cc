#include "../include/pause.h"

#include <signal.h>
#include <stdio.h>

pid_t pause()
{
  const char filename[] = "/tmp/pid.txt";
  FILE* pid_file_hdl;
  pid_t pid;
  int scan_cnt;

  // Extract the PID from the temp file.
  // Return if the file is empty
  pid_file_hdl = fopen(filename, "rb");
  if (!pid_file_hdl)
  {
    printf("No music to pause!\n");
    return -1;
  }
  else
  {
    scan_cnt = fscanf(pid_file_hdl, "%d", &pid);
    fclose(pid_file_hdl);

    // Signal the play application
    if (scan_cnt > 0)
    {
      kill(pid, SIGUSR1);
    }

    return pid;
  }
}
