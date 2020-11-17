#ifndef LDR_PITCH_BEND_H
#define LDR_PITCH_BEND_H

float read_pitch_bend(unsigned int bend_up_pin, unsigned int bend_down_pin);
int set_ldr_min();
int set_ldr_max();
void first_set_ldr_min(unsigned int bend_up_pin, unsigned int bend_down_pin);

#endif