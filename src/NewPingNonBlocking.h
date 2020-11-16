#ifndef PING_AVERAGE_H
#define PING_AVERAGE_H

#include <NewPing.h>

#include <functional>
#include <vector>

#define ITERATIONS 5

class NewPingNonBlocking : public NewPing {
  using NewPing::NewPing;

 public:
  unsigned long ping_average(unsigned int max_cm_distance = 0);

 private:
  int _echoTimesToAverage[5]{0};
  int _echoIndexToOverwrite{0};
  int _lastPingAverage{0};
};

#endif