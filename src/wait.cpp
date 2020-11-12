#include "wait.h"

#include <Arduino.h>

typedef void (*voidFunc)();

void wait(unsigned int milliseconds, voidFunc innerLoop) {
  elapsedMillis msec = 0;

  while (msec <= milliseconds) {
    innerLoop();
  }
}