#ifndef NOOS_SELFTEST_H
#define NOOS_SELFTEST_H

#include <stdbool.h>
#include <stdint.h>

struct selftest_result {
    uint32_t passed;
    uint32_t failed;
};

struct selftest_result selftest_run(void);
bool selftest_ok(struct selftest_result result);

#endif
