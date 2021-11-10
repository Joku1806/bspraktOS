// Inspiriert von
// https://github.com/SerenityOS/serenity/blob/master/Kernel/Assertions.h

#ifndef ASSERTIONS_H
#define ASSERTIONS_H

#include "debug.h"

#define VERIFY(expression)                                                     \
  do {                                                                         \
    if (!(expression)) {                                                       \
      warnln("ASSERTION FAILED: %s\n%s:%u in %s\n", #expression, __FILE__,     \
             __LINE__, __PRETTY_FUNCTION__);                                   \
    }                                                                          \
  } while (0)

#define VERIFY_NOT_REACHED() VERIFY(false)

#endif