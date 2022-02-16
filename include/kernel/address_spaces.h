#ifndef ADDRESS_SPACES_H
#define ADDRESS_SPACES_H

#include <kernel/lib/kintrusive_list.h>
#include <stdbool.h>
#include <stddef.h>

#define ADDRESS_SPACE_COUNT 8
#define THREADS_PER_ADDRESS_SPACE 4

k_node *get_address_space_list(size_t address_space);
bool find_first_empty_address_space(size_t *index);

#endif