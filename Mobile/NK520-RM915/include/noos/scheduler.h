#ifndef NOOS_SCHEDULER_H
#define NOOS_SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>

#define NOOS_MAX_TASKS 16u

enum noos_task_state {
    NOOS_TASK_UNUSED = 0,
    NOOS_TASK_READY = 1,
    NOOS_TASK_RUNNING = 2,
    NOOS_TASK_SLEEPING = 3,
    NOOS_TASK_BLOCKED = 4,
    NOOS_TASK_DEAD = 5
};

typedef void (*noos_task_entry)(void *context);

struct noos_task {
    uint32_t id;
    enum noos_task_state state;
    uint32_t wake_tick;
    noos_task_entry entry;
    void *context;
};

struct scheduler_stats {
    uint32_t ticks;
    uint32_t task_count;
    uint32_t ready_count;
    uint32_t sleeping_count;
    uint32_t switches_planned;
    uint32_t reschedule_requests;
    uint32_t context_switches;
};

void scheduler_init(void);
int32_t scheduler_add(noos_task_entry entry, void *context);
bool scheduler_set_state(uint32_t id, enum noos_task_state state);
bool scheduler_sleep_until(uint32_t id, uint32_t wake_tick);
void scheduler_tick(void);
int32_t scheduler_pick_next(void);
void scheduler_request_reschedule(void);
bool scheduler_reschedule_pending(void);
void scheduler_clear_reschedule(void);
void scheduler_note_context_switch(void);
const struct noos_task *scheduler_task(uint32_t id);
const struct scheduler_stats *scheduler_get_stats(void);

#endif
