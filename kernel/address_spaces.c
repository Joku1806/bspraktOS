#include <kernel/address_spaces.h>
#include <kernel/lib/kassertions.h>

static k_node address_spaces[ADDRESS_SPACE_COUNT] = {
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
};

k_node *get_address_space_list(size_t address_space) {
  VERIFY(address_space < ADDRESS_SPACE_COUNT);

  return &address_spaces[address_space];
}

bool find_first_empty_address_space(size_t *index) {
  for (size_t i = 0; i < ADDRESS_SPACE_COUNT; i++) {
    if (k_is_list_empty(&address_spaces[i])) {
      *index = i;
      return true;
    }
  }

  return false;
}