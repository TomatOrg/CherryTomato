#pragma once

#include <util/defs.h>
// actually called euclidean division and remainder
// TODO: for now remainder must be positive
static inline int floordiv(int a, int b) { return (a / b) - (a % b != 0 && a < 0); }
static inline int floormod(int a, int b) { return (a % b) + ((a % b != 0 && a < 0) ? b : 0); }
