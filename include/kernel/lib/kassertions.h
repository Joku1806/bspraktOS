// Inspiriert von
// https://github.com/SerenityOS/serenity/blob/master/Kernel/Assertions.h

#ifndef KASSERTIONS_H
#define KASSERTIONS_H

#include <kernel/lib/kdebug.h>
#include <stdbool.h>

#define VERIFY(expression)                                                 \
  do {                                                                     \
    if (!(expression)) {                                                   \
      kpanicln("ASSERTION FAILED: %s\n%s:%u in %s", #expression, __FILE__, \
               __LINE__, __PRETTY_FUNCTION__);                             \
    }                                                                      \
  } while (0)

#define VERIFY_NOT_REACHED() VERIFY(false)

#endif