#ifndef NOOS_SELFTEST_H
#define NOOS_SELFTEST_H

#include <noos/boot.h>
#include <noos/bootargs.h>
#include <stdbool.h>
#include <stdint.h>

struct selftest_result {
    uint32_t passed;
    uint32_t failed;
};

struct selftest_result selftest_run(const struct noos_boot_info *boot,
                                    const struct noos_boot_options *options);
bool selftest_ok(struct selftest_result result);

#endif
