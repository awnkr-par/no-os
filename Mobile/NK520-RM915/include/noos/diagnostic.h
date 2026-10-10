#ifndef NOOS_DIAGNOSTIC_H
#define NOOS_DIAGNOSTIC_H

#include <noos/boot.h>
#include <noos/cpu.h>
#include <noos/pmm.h>
#include <noos/selftest.h>
#include <stdint.h>

#define NOOS_DIAG_MAGIC 0x4E444941u
#define NOOS_DIAG_VERSION 1u
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

    uint32_t cpu_midr;
    uint32_t cpu_cpsr;
    uint32_t cpu_sctlr;

    uint32_t pmm_total_pages;
    uint32_t pmm_free_pages;
    uint32_t pmm_permanent_pages;
    uint32_t pmm_allocated_pages;

    uint32_t framebuffer_address;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_stride;
    uint32_t framebuffer_format;

    uint32_t selftests_passed;
    uint32_t selftests_failed;

    char panic_reason[NOOS_DIAG_PANIC_BYTES];
    char log_tail[NOOS_DIAG_LOG_BYTES];
};

void diagnostic_init(void);
void diagnostic_set_stage(uint32_t stage, uint32_t detail);
void diagnostic_capture_boot(const struct noos_boot_info *boot);
void diagnostic_capture_cpu(const struct cpu_state *cpu);
void diagnostic_capture_pmm(const struct pmm_stats *pmm);
void diagnostic_capture_selftest(struct selftest_result result);
void diagnostic_capture_panic(const char *reason);
void diagnostic_capture_console(void);
const struct noos_diagnostic_record *diagnostic_get(void);

#endif
