#include <noos/scheduler.h>
#include <noos/memory.h>

static struct noos_task g_tasks[NOOS_MAX_TASKS];
static struct scheduler_stats g_stats;
static uint32_t g_next_id;
static uint32_t g_cursor;

static void recount(void)
{
    g_stats.task_count = 0;
    g_stats.ready_count = 0;
    g_stats.sleeping_count = 0;

    for (uint32_t i = 0; i < NOOS_MAX_TASKS; ++i) {
        if (g_tasks[i].state != NOOS_TASK_UNUSED &&
            g_tasks[i].state != NOOS_TASK_DEAD) {
            g_stats.task_count++;
        }
        if (g_tasks[i].state == NOOS_TASK_READY) {
            g_stats.ready_count++;
        }
        if (g_tasks[i].state == NOOS_TASK_SLEEPING) {
            g_stats.sleeping_count++;
        }
    }
}

void scheduler_init(void)
{
    memset(g_tasks, 0, sizeof(g_tasks));
    memset(&g_stats, 0, sizeof(g_stats));
    g_next_id = 1;
    g_cursor = 0;
}

int32_t scheduler_add(noos_task_entry entry, void *context)
{
    if (entry == 0) {
        return -1;
    }

    for (uint32_t i = 0; i < NOOS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == NOOS_TASK_UNUSED ||
            g_tasks[i].state == NOOS_TASK_DEAD) {
            g_tasks[i].id = g_next_id++;
            g_tasks[i].state = NOOS_TASK_READY;
            g_tasks[i].wake_tick = 0;
            g_tasks[i].entry = entry;
            g_tasks[i].context = context;
            recount();
            return (int32_t)g_tasks[i].id;
        }
    }

    return -1;
}

static struct noos_task *find_task(uint32_t id)
{
    for (uint32_t i = 0; i < NOOS_MAX_TASKS; ++i) {
        if (g_tasks[i].id == id &&
            g_tasks[i].state != NOOS_TASK_UNUSED &&
            g_tasks[i].state != NOOS_TASK_DEAD) {
            return &g_tasks[i];
        }
    }
    return 0;
}

bool scheduler_set_state(uint32_t id, enum noos_task_state state)
{
    struct noos_task *task = find_task(id);
    if (task == 0 || state == NOOS_TASK_UNUSED) {
        return false;
    }

    task->state = state;
    recount();
    return true;
}

bool scheduler_sleep_until(uint32_t id, uint32_t wake_tick)
{
    struct noos_task *task = find_task(id);
    if (task == 0) {
        return false;
    }

    task->wake_tick = wake_tick;
    task->state = NOOS_TASK_SLEEPING;
    recount();
    return true;
}

void scheduler_tick(void)
{
    g_stats.ticks++;

    for (uint32_t i = 0; i < NOOS_MAX_TASKS; ++i) {
        if (g_tasks[i].state == NOOS_TASK_SLEEPING &&
            (int32_t)(g_stats.ticks - g_tasks[i].wake_tick) >= 0) {
            g_tasks[i].state = NOOS_TASK_READY;
        }
    }

    recount();
}

int32_t scheduler_pick_next(void)
{
    for (uint32_t checked = 0; checked < NOOS_MAX_TASKS; ++checked) {
        const uint32_t index = (g_cursor + checked) % NOOS_MAX_TASKS;
        if (g_tasks[index].state == NOOS_TASK_READY) {
            g_cursor = (index + 1u) % NOOS_MAX_TASKS;
            g_stats.switches_planned++;
            return (int32_t)g_tasks[index].id;
        }
    }

    return -1;
}

const struct noos_task *scheduler_task(uint32_t id)
{
    return find_task(id);
}

const struct scheduler_stats *scheduler_get_stats(void)
{
    return &g_stats;
}
