#ifndef SCALES_H
#define SCALES_H

#include <vector>

using namespace std;

vector<int> MAJOR_SCALE = {
    0, 2, 4, 5, 7, 9, 11,
};

vector<int> PENTATONIC_SCALE = {
    0, 2, 4, 7, 9,
};

vector<int> MINOR_SCALE = {0, 2, 3, 5, 7, 8, 10};
vector<int> WHOLE_TONE_SCALE = {0, 2, 4, 6, 8, 10};
vector<int> MAJOR_CHORD = {0, 4, 7};
vector<int> MINOR_CHORD = {0, 3, 7};
vector<int> MAJOR_7TH_CHORD = {0, 4, 7, 11};
vector<int> DOMINANT_7TH_CHORD = {0, 4, 7, 10};
vector<int> CHROMATIC_SCALE = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

#endif SCALES_H