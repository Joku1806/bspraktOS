#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#define EINVAL 1
#define ERANGE 2
#define EBUSY 3

const char *error_string(int code);

#endif