#ifndef TYPES_H
#define TYPES_H

#include <vector>

#include <limits.h>
#include <float.h>
#include <stdint.h>

typedef uint32_t   vertex_t;
typedef float      weight_t;
typedef unsigned   color_t;
typedef uint32_t   colorset_t;

#define MAX_COLORS (sizeof (colorset_t) * CHAR_BIT - 1) // one bit is used up in PTree::Node
#define WEIGHT_MAX FLT_MAX

#endif // TYPES_H
