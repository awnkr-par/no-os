#include <noos/bootargs.h>
#include <noos/irq.h>
#include <noos/klog.h>
#include <noos/memory.h>
#include <noos/scheduler.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "host-core FAIL: %s\n", message);
        exit(1);
    }
}

static void dummy_task(void *context)
{
    (void)context;
}

static unsigned irq_count;

static void test_irq_handler(uint32_t irq, void *context)
{
    unsigned *count = (unsigned *)context;
    if (irq == 7u) {
        (*count)++;
    }
}

static void test_bootargs(void)
{
    struct noos_boot_options options;
    bootargs_parse(&options,
        "noos.splash=off noos.selftest=yes noos.mmu-plan=0 "
        "noos.sched-test=true noos.loglevel=0x2A");

    require(!options.splash, "splash boolean");
    require(options.selftest, "selftest boolean");
    require(!options.mmu_plan, "mmu-plan boolean");
    require(options.scheduler_test, "sched-test boolean");
    require(options.log_level == 42u, "hex u32");
    require(bootargs_get_u32("x=4294967295", "x", 0) == UINT32_MAX,
            "u32 max decimal");
    require(bootargs_get_u32("x=4294967296", "x", 77) == 77u,
            "u32 overflow rejection");
}

static void test_klog(void)
{
    klog_init();

    for (uint32_t i = 0; i < NOOS_KLOG_CAPACITY + 100u; ++i) {
        klog_putc((char)('A' + (i % 26u)));
    }

    const struct klog_stats *stats = klog_get_stats();
    require(stats->length == NOOS_KLOG_CAPACITY, "ring retained length");
    require(stats->bytes_written == NOOS_KLOG_CAPACITY + 100u,
            "ring written count");
    require(stats->bytes_dropped == 100u, "ring overwrite count");
    require(stats->wraps == 1u, "ring wrap count");

    char tail[33];
    const size_t copied = klog_snapshot(tail, sizeof(tail));
    require(copied == 32u, "ring snapshot tail size");
    require(tail[32] == '\0', "ring snapshot terminator");
}

static void test_scheduler(void)
{
    scheduler_init();

    const int32_t a = scheduler_add(dummy_task, 0);
    const int32_t b = scheduler_add(dummy_task, 0);
    const int32_t c = scheduler_add(dummy_task, 0);

    require(a > 0 && b > 0 && c > 0, "task creation");
    require(scheduler_pick_next() == a, "round robin A");
    require(scheduler_pick_next() == b, "round robin B");
    require(scheduler_pick_next() == c, "round robin C");

    require(scheduler_sleep_until((uint32_t)b, 2u), "sleep task");
    scheduler_tick();
    require(scheduler_task((uint32_t)b)->state == NOOS_TASK_SLEEPING,
            "task remains asleep before deadline");
    scheduler_tick();
    require(scheduler_task((uint32_t)b)->state == NOOS_TASK_READY,
            "task wakes at deadline");

    require(scheduler_set_state((uint32_t)c, NOOS_TASK_DEAD), "kill task");
    require(scheduler_get_stats()->task_count == 2u, "dead task count");
}

static void test_irq(void)
{
    irq_init();
    irq_count = 0;

    require(irq_register(7u, test_irq_handler, &irq_count), "IRQ register");
    require(!irq_register(7u, test_irq_handler, &irq_count),
            "duplicate IRQ reject");
    require(irq_dispatch(7u), "IRQ dispatch");
    require(irq_count == 1u, "IRQ handler called");
    require(!irq_dispatch(8u), "spurious IRQ result");
    require(irq_get_stats()->handled == 1u, "IRQ handled count");
    require(irq_get_stats()->spurious == 1u, "IRQ spurious count");
    require(irq_unregister(7u), "IRQ unregister");
}

static void test_memory(void)
{
    unsigned char a[32];
    unsigned char b[32];

    memset(a, 0x5A, sizeof(a));
    memset(b, 0, sizeof(b));
    memcpy(b, a, sizeof(a));
    require(memcmp(a, b, sizeof(a)) == 0, "memory copy");

    for (unsigned i = 0; i < sizeof(a); ++i) {
        a[i] = (unsigned char)i;
    }

    memmove(a + 4, a, 20);
    require(a[4] == 0 && a[23] == 19, "overlap memmove");
}

int main(void)
{
    test_bootargs();
    test_klog();
    test_scheduler();
    test_irq();
    test_memory();

    puts("host-core: all tests passed");
    return 0;
}
