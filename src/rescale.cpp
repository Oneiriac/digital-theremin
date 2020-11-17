#include "rescale.h"

/**
 * @brief https://stackoverflow.com/a/5295202
 *
 * @param oldMin
 * @param oldMax
 * @param newMin
 * @param newMax
 * @param x
 * @return double
 */
double rescale(double oldMin, double oldMax, double newMin, double newMax, double x) {
  return (newMax - newMin) * (x - oldMin) / (oldMax - oldMin) + newMin;
}
