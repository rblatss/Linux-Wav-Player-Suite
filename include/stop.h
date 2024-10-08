/* ***********************************************
 *                                               *
 *                     Stop                      *
 *                Robert Blatner                 *
 *                                               *
 * ******************************************** */
#ifndef STOP
#define STOP

#include <stdlib.h>

/*
 * stop:
 *
 * Stop the audio that is currently running.
 *
 * Returns:
 *   The pid number on success
 *   -1 on error
 */
extern "C"
{
  pid_t stop();
}

#endif // STOP