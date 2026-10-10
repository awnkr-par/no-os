#ifndef NOOS_PANIC_H
#define NOOS_PANIC_H

const char *panic_reason(void);
_Noreturn void panic(const char *reason);
_Noreturn void halt_forever(void);

#endif
