/* ***********************************************
 *                                               *
 *                     Pause                     *
 *                Robert Blatner                 *
 *                                               *
 * ******************************************** */
#ifndef PAUSE
#define PAUSE

#include <stdlib.h>

/*
 * pause:
 *
 * Pause the audio that is currently running, or resume audio that was previously paused. 
 *
 * Returns:
 *   The pid number on success
 *   -1 on error
 */
extern "C"
{
  pid_t pause();
}

#endif // PAUSE