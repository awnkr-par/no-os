#ifndef NOOS_PLATFORM_H
#define NOOS_PLATFORM_H

#include <stdbool.h>

void platform_early_init(void);
bool platform_debug_init(void);
void platform_debug_putc(char c);
_Noreturn void platform_idle(void);
const char *platform_name(void);

#endif
