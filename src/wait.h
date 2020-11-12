#ifndef WAIT_H
#define WAIT_H

using namespace std;
#include <functional>

void wait(unsigned int milliseconds, std::function<void()> innerLoop);

#endif