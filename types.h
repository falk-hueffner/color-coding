#ifndef TYPES_H
#define TYPES_H

#include <float.h>
#include <limits.h>
#include <stdint.h>

#include "util.h"

COMPILE_TIME_ASSERT(CHAR_BIT == 8);

typedef unsigned      vertex_t;
typedef int16_t       small_vertex_t;
typedef float         weight_t;
typedef unsigned      color_t;
typedef unsigned long colorset_t;

#define MAX_COLORS (sizeof (colorset_t) * CHAR_BIT - 1) // one bit is used up in PTree::Node
#define DELETED_VERTEX 32767
#define MAX_VERTEX (DELETED_VERTEX - 1)
#define WEIGHT_MAX FLT_MAX

#endif // TYPES_H
