#include "../include/stop.h"

#include <stdio.h>
#include <stdlib.h>

void PrintUsage(FILE* stream, const char* program_name)
{
  fprintf(stream, "Usage: %s\n", program_name);
  fprintf(stream, "Stop PCM audio that is currently running. If no audio running, do nothing.");
  exit(0);
}

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    PrintUsage(stderr, argv[0]);
  }
  return stop();
}
