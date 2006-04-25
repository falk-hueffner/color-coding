#ifndef TYPES_H
#define TYPES_H

#include <float.h>
#include <limits.h>

#include "util.h"

typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

COMPILE_TIME_ASSERT(CHAR_BIT == 8);
COMPILE_TIME_ASSERT(sizeof (uint16_t) == 2);
COMPILE_TIME_ASSERT(sizeof (uint32_t) == 4);

typedef unsigned   vertex_t;
typedef uint16_t   small_vertex_t;
typedef float      weight_t;
typedef unsigned   color_t;
typedef uint32_t   colorset_t;

#define MAX_COLORS (sizeof (colorset_t) * CHAR_BIT - 1) // one bit is used up in PTree::Node
#define MAX_VERTEX ((small_vertex_t) -1)
#define WEIGHT_MAX FLT_MAX

#endif // TYPES_H
