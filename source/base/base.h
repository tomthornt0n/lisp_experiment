
//~NOTE(tbt): stdlib headers (these are the only safe ones to include if building without the stdlib)
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>

//~NOTE(tbt): base layer header files
#include "base__context_cracking.h"
#include "base__misc.h"
#include "base__math.h"
#include "base__memory.h"
#include "base__strings.h"
#include "base__time.h"
#include "base__rng.h"
#include "base__sort.h"
