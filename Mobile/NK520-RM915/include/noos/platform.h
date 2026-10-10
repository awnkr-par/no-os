#ifndef NOOS_PLATFORM_H
#define NOOS_PLATFORM_H

void platform_early_init(void);
_Noreturn void platform_idle(void);
const char *platform_name(void);

#endif
