#ifndef NOOS_CONSOLE_H
#define NOOS_CONSOLE_H

#include <stddef.h>
#include <stdint.h>

typedef void (*console_sink_fn)(char c);

void console_init(void);
void console_set_sink(console_sink_fn sink);
void console_putc(char c);
void console_write(const char *text);
void console_write_hex32(uint32_t value);
void console_write_dec32(uint32_t value);
const char *console_buffer(void);
size_t console_buffer_length(void);

#endif
