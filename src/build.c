#include <stdio.h>
#include <stdlib.h>

#ifndef PCRE2_STATIC
    #define PCRE2_STATIC
#endif
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "libbloom/bloom.h"

#include "sth/sth.c"
#include "regexp.c"
#include "libbloom/bloom.c"
#include "main.c"
