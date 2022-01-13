#include <user/lib/assertions.h>
#include <user/lib/error.h>

const char *error_string(int code) {
  switch (code) {
    case -EINVAL:
      return "-EINVAL";
    case -ERANGE:
      return "-ERANGE";
    case -EBUSY:
      return "-EBUSY";
  }

  VERIFY_NOT_REACHED();
}