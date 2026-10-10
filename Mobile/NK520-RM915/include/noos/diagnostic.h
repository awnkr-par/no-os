#ifndef NOOS_DIAGNOSTIC_H
#define NOOS_DIAGNOSTIC_H

#include <noos/boot.h>
#include <noos/bootargs.h>
#include <noos/cpu.h>
#include <noos/irq.h>
#include <noos/klog.h>
#include <noos/mmu.h>
#include <noos/pmm.h>
#include <noos/qgic.h>
#include <noos/scheduler.h>
#include <noos/selftest.h>
#include <noos/timer.h>
#include <noos/uartdm.h>
#include <stdint.h>

#define NOOS_DIAG_MAGIC 0x4E444941u
#define NOOS_DIAG_VERSION 7u
#define NOOS_DIAG_PANIC_BYTES 64u
#define NOOS_DIAG_LOG_BYTES 256u

struct noos_diagnostic_record {
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    uint32_t checksum;

    uint32_t stage;
    uint32_t detail;

    uint32_t boot_protocol;
    uint32_t boot_data_address;
    uint32_t boot_data_size;
    uint32_t total_memory_bytes;
    uint32_t reserved_range_count;

    uint32_t boot_option_flags;
    uint32_t boot_log_level;

    uint32_t cpu_midr;
    uint32_t cpu_cpsr;
    uint32_t cpu_sctlr;

    uint32_t pmm_total_pages;
    uint32_t pmm_free_pages;
    uint32_t pmm_permanent_pages;
    uint32_t pmm_allocated_pages;

    uint32_t mmu_table_address;
    uint32_t mmu_mapped_sections;
    uint32_t mmu_first_section;
    uint32_t mmu_last_section;
    uint32_t mmu_validated;

    uint32_t uart_base;
    uint32_t uart_bytes_sent;
    uint32_t uart_tx_timeouts;
    uint32_t uart_flags;

    uint32_t framebuffer_address;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_stride;
    uint32_t framebuffer_format;

    uint32_t selftests_passed;
    uint32_t selftests_failed;

    uint32_t klog_bytes_written;
    uint32_t klog_bytes_overwritten;
    uint32_t klog_wraps;
    uint32_t klog_length;

    uint32_t scheduler_ticks;
    uint32_t scheduler_tasks;
    uint32_t scheduler_ready;
    uint32_t scheduler_sleeping;
    uint32_t scheduler_switches_planned;
    uint32_t scheduler_reschedule_requests;
    uint32_t scheduler_context_switches;

    uint32_t irq_registered;
    uint32_t irq_dispatches;
    uint32_t irq_handled;
    uint32_t irq_spurious;
    uint32_t irq_last;

    uint32_t qgic_initialized;
    uint32_t qgic_irq_count;
    uint32_t qgic_last_ack;
    uint32_t qgic_spurious;
    uint32_t qgic_unhandled;

    uint32_t timer_initialized;
    uint32_t timer_interrupts;
    uint32_t timer_last_count;
    uint32_t timer_period_counts;

    char panic_reason[NOOS_DIAG_PANIC_BYTES];
    char log_tail[NOOS_DIAG_LOG_BYTES];
};

void diagnostic_init(void);
void diagnostic_set_stage(uint32_t stage, uint32_t detail);
void diagnostic_capture_boot(const struct noos_boot_info *boot);
void diagnostic_capture_boot_options(const struct noos_boot_options *options);
void diagnostic_capture_cpu(const struct cpu_state *cpu);
void diagnostic_capture_pmm(const struct pmm_stats *pmm);
void diagnostic_capture_mmu(const struct mmu_stats *mmu);
void diagnostic_capture_uart(const struct uartdm_stats *uart);
void diagnostic_capture_selftest(struct selftest_result result);
void diagnostic_capture_scheduler(const struct scheduler_stats *scheduler);
void diagnostic_capture_irq(const struct irq_stats *irq);
void diagnostic_capture_qgic(const struct qgic_stats *qgic);
void diagnostic_capture_timer(const struct rm915_timer_stats *timer);
void diagnostic_capture_panic(const char *reason);
void diagnostic_capture_console(void);
const struct noos_diagnostic_record *diagnostic_get(void);

#endif
