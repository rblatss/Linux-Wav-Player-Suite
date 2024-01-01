#ifndef PAUSE
#define PAUSE

#include <stdio.h>
#include <signal.h>

// Get process id from pid.txt, kill process, delete pid.txt
pid_t Pause()
{
  char filename[] = "/tmp/pid.txt";
  char c;
  FILE *pid_file;
  int len;
  pid_t pid;

  if (!(pid_file = fopen(filename, "rb")))
    return -1;

  fscanf(pid_file, "%d", &pid);
  fclose(pid_file);

  kill(pid, 10);

  return pid;
}

#endif // PAUSE