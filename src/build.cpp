#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <cstring>

#ifndef PCRE2_STATIC
    #define PCRE2_STATIC
#endif
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "libbloom/bloom.h"

#include "base/base.cpp"
#include "io/io.cpp"
#include "regexp/regexp.cpp"
#include "libbloom/bloom.c"
#include "main.cpp"
