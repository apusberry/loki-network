#ifndef LLARP_LOGIC_H
#define LLARP_LOGIC_H
#include <llarp/mem.h>
#include <llarp/threadpool.h>
#include <llarp/timer.h>

struct llarp_logic
{
  struct llarp_threadpool* thread;
  struct llarp_timer_context* timer;
};

struct llarp_logic*
llarp_init_logic();

/// single threaded mode logic event loop
struct llarp_logic*
llarp_init_single_process_logic(struct llarp_threadpool* tp);

/// single threaded tick
void
llarp_logic_tick(struct llarp_logic* logic, llarp_time_t now);

/// isolated tick
void
llarp_logic_tick_async(struct llarp_logic* logic, llarp_time_t now);

void
llarp_free_logic(struct llarp_logic** logic);

void
llarp_logic_queue_job(struct llarp_logic* logic, struct llarp_thread_job job);

uint32_t
llarp_logic_call_later(struct llarp_logic* logic, struct llarp_timeout_job job);

void
llarp_logic_cancel_call(struct llarp_logic* logic, uint32_t id);

void
llarp_logic_remove_call(struct llarp_logic* logic, uint32_t id);

void
llarp_logic_stop_timer(struct llarp_logic* logic);

void
llarp_logic_stop(struct llarp_logic* logic);

void
llarp_logic_mainloop(struct llarp_logic* logic);

#endif
