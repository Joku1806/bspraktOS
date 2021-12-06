#include <kernel/bounded_thread_list.h>
#include <kernel/scheduler.h>

void thread_cleanup(){

  transfer_node_to_list(get_thread_list_head(running), get_thread_list_head(finished));
  schedule_next_thread();
  
}