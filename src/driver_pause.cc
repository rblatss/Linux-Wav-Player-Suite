#include "../include/pause.h"

#include <stdio.h>
#include <stdlib.h>

void PrintUsage(FILE* stream, const char* program_name)
{
  fprintf(stream, "Usage: %s\n", program_name);
  fprintf(stream,
          "Pause PCM audio that is currently running. Resume audio that was previously paused.");
  exit(0);
}

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    PrintUsage(stderr, argv[0]);
  }
  return pause();
}