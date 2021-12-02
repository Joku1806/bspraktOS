#include <kernel/bounded_thread_list.h>
#include <lib/assertions.h>
#include <stddef.h>

static const tcb blocks[TCB_LIST_SIZE];

node *ready_head = NULL;
node *waiting_head = NULL;
node *running_head = NULL;
node *finished_head = (node *)&blocks[0];

void thread_list_initialise() {
  for (size_t i = 0; i < TCB_LIST_SIZE; i++) {
    node *current = (node *)&blocks[i];
    if (i > 0) {
      current->previous = (node *)&blocks[i - 1];
    } else {
      current->previous = (node *)&blocks[TCB_LIST_SIZE - 1];
    }

    if (i < TCB_LIST_SIZE - 1) {
      current->next = (node *)&blocks[i + 1];
    } else {
      current->next = (node *)&blocks[0];
    }
  }
}

void transfer_list(node *l1, node **l2) {
  VERIFY(l1 != NULL && l2 != NULL);

  if (l1->previous != NULL) {
    l1->previous->next = l1->next;
  }

  if (l1->next != NULL) {
    l1->next->previous = l1->previous;
  }

  if (*l2 == NULL) {
    *l2 = l1;
    l1->previous = l1;
    l1->next = l1;
  } else {
    l1->previous = (*l2);
    l1->next = (*l2)->next;

    (*l2)->next->previous = l1;
    (*l2)->next = l1;
  }
}