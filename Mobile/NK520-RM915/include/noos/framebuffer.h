#ifndef NOOS_FRAMEBUFFER_H
#define NOOS_FRAMEBUFFER_H

#include <noos/boot.h>
#include <noos/pmm.h>
#include <noos/selftest.h>
#include <stdbool.h>
#include <stdint.h>

bool framebuffer_init(const struct noos_framebuffer *fb);
bool framebuffer_available(void);
void framebuffer_clear(uint32_t rgb);
void framebuffer_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t rgb);
void framebuffer_draw_text(uint32_t x, uint32_t y, const char *text, uint32_t rgb, uint32_t scale);
void framebuffer_draw_hex32(uint32_t x, uint32_t y, uint32_t value, uint32_t rgb, uint32_t scale);
void framebuffer_show_boot_splash(bool selftests_ok);
void framebuffer_show_runtime(const struct noos_boot_info *boot,
                              const struct pmm_stats *pmm,
                              struct selftest_result tests);

#endif
