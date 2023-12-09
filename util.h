#ifndef UTIL_H
#define UTIL_H

#include "remote-char.h"

extern void new_position(int *x, int *y, direction_t direction);
extern void tail_position_calc(int *x, int *y, direction_t direction, char *overflow);

#endif
