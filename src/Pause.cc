#include <stdio.h>
#include "../include/Pause.h"

int main(){
  if(Pause() == -1)
    printf("No music to pause!\n"); 
  return 0;
}