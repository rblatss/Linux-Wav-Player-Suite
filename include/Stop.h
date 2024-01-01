#ifndef STOP
#define STOP

#include <stdio.h>
#include <signal.h>

// Get process id from pid.txt, kill process, delete pid.txt
pid_t Stop()
{
  char filename[] = "/tmp/pid.txt";
  char c;
  FILE *pid_file;
  int len;
  pid_t pid;

  if (!(pid_file = fopen(filename, "rb")))
  {
    return -1;
  }

  fscanf(pid_file, "%d", &pid);
  fclose(pid_file);

  kill(pid, 1);

  remove(filename);

  return pid;
}

#endif // STOP