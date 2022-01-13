// Inspiriert von
// https://github.com/SerenityOS/serenity/blob/master/Kernel/Assertions.h

#ifndef ASSERTIONS_H
#define ASSERTIONS_H

#include <stdbool.h>
#include <user/lib/debug.h>

#define VERIFY(expression)                                                \
  do {                                                                    \
    if (!(expression)) {                                                  \
      panicln("ASSERTION FAILED: %s\n%s:%u in %s", #expression, __FILE__, \
              __LINE__, __PRETTY_FUNCTION__);                             \
    }                                                                     \
  } while (0)

#define VERIFY_NOT_REACHED() VERIFY(false)

#endif