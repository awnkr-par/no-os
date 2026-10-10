#include <noos/framebuffer.h>
#include <stddef.h>
#include <stdint.h>

static struct noos_framebuffer g_fb;
static bool g_available;

static uint32_t bytes_per_pixel(enum noos_pixel_format format)
{
    switch (format) {
    case NOOS_PIXEL_XRGB8888:
    case NOOS_PIXEL_ARGB8888:
        return 4;
    case NOOS_PIXEL_RGB565:
        return 2;
    default:
        return 0;
    }
}

static uint16_t rgb565(uint32_t rgb)
{
    const uint32_t r = (rgb >> 16) & 0xFFu;
    const uint32_t g = (rgb >> 8) & 0xFFu;
    const uint32_t b = rgb & 0xFFu;
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

static void put_pixel(uint32_t x, uint32_t y, uint32_t rgb)
{
    if (!g_available || x >= g_fb.width || y >= g_fb.height) {
        return;
    }

    volatile uint8_t *row = (volatile uint8_t *)(uintptr_t)
        (g_fb.address + y * g_fb.stride);

    if (g_fb.format == NOOS_PIXEL_RGB565) {
        volatile uint16_t *pixels = (volatile uint16_t *)row;
        pixels[x] = rgb565(rgb);
    } else {
        volatile uint32_t *pixels = (volatile uint32_t *)row;
        pixels[x] = rgb & 0x00FFFFFFu;
    }
}

static uint8_t glyph_row(char c, uint32_t row)
{
    static const uint8_t blank[7] = {0,0,0,0,0,0,0};
    static const uint8_t A[7] = {14,17,17,31,17,17,17};
    static const uint8_t B[7] = {30,17,17,30,17,17,30};
    static const uint8_t C[7] = {14,17,16,16,16,17,14};
    static const uint8_t D[7] = {30,17,17,17,17,17,30};
    static const uint8_t E[7] = {31,16,16,30,16,16,31};
    static const uint8_t F[7] = {31,16,16,30,16,16,16};
    static const uint8_t G[7] = {14,17,16,23,17,17,15};
    static const uint8_t I[7] = {31,4,4,4,4,4,31};
    static const uint8_t L[7] = {16,16,16,16,16,16,31};
    static const uint8_t M[7] = {17,27,21,21,17,17,17};
    static const uint8_t N[7] = {17,25,21,19,17,17,17};
    static const uint8_t O[7] = {14,17,17,17,17,17,14};
    static const uint8_t P[7] = {30,17,17,30,16,16,16};
    static const uint8_t R[7] = {30,17,17,30,20,18,17};
    static const uint8_t S[7] = {15,16,16,14,1,1,30};
    static const uint8_t T[7] = {31,4,4,4,4,4,4};
    static const uint8_t U[7] = {17,17,17,17,17,17,14};
    static const uint8_t X[7] = {17,17,10,4,10,17,17};
    static const uint8_t zero[7] = {14,17,19,21,25,17,14};
    static const uint8_t one[7] = {4,12,4,4,4,4,14};
    static const uint8_t two[7] = {14,17,1,2,4,8,31};
    static const uint8_t three[7] = {30,1,1,14,1,1,30};
    static const uint8_t four[7] = {2,6,10,18,31,2,2};
    static const uint8_t five[7] = {31,16,16,30,1,1,30};
    static const uint8_t six[7] = {14,16,16,30,17,17,14};
    static const uint8_t seven[7] = {31,1,2,4,8,8,8};
    static const uint8_t eight[7] = {14,17,17,14,17,17,14};
    static const uint8_t nine[7] = {14,17,17,15,1,1,14};
    static const uint8_t dash[7] = {0,0,0,31,0,0,0};
    const uint8_t *glyph = blank;

    switch (c) {
    case 'A': glyph = A; break;
    case 'B': glyph = B; break;
    case 'C': glyph = C; break;
    case 'D': glyph = D; break;
    case 'E': glyph = E; break;
    case 'F': glyph = F; break;
    case 'G': glyph = G; break;
    case 'I': glyph = I; break;
    case 'L': glyph = L; break;
    case 'M': glyph = M; break;
    case 'N': glyph = N; break;
    case 'O': glyph = O; break;
    case 'P': glyph = P; break;
    case 'R': glyph = R; break;
    case 'S': glyph = S; break;
    case 'T': glyph = T; break;
    case 'U': glyph = U; break;
    case 'X': glyph = X; break;
    case '0': glyph = zero; break;
    case '1': glyph = one; break;
    case '2': glyph = two; break;
    case '3': glyph = three; break;
    case '4': glyph = four; break;
    case '5': glyph = five; break;
    case '6': glyph = six; break;
    case '7': glyph = seven; break;
    case '8': glyph = eight; break;
    case '9': glyph = nine; break;
    case '-': glyph = dash; break;
    default: break;
    }

    return row < 7 ? glyph[row] : 0;
}

bool framebuffer_init(const struct noos_framebuffer *fb)
{
    g_available = false;

    if (fb == 0 || !fb->valid || fb->address == 0 ||
        fb->width == 0 || fb->height == 0 || fb->stride == 0) {
        return false;
    }

    const uint32_t bpp = bytes_per_pixel(fb->format);
    if (bpp == 0 || fb->stride < fb->width * bpp) {
        return false;
    }

    const uint64_t required = (uint64_t)fb->stride * (uint64_t)fb->height;
    if (required == 0 || required > UINT32_MAX) {
        return false;
    }

    if (fb->size != 0 && required > fb->size) {
        return false;
    }

    g_fb = *fb;
    g_available = true;
    return true;
}

bool framebuffer_available(void)
{
    return g_available;
}

void framebuffer_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t rgb)
{
    if (!g_available || x >= g_fb.width || y >= g_fb.height) {
        return;
    }

    if (width > g_fb.width - x) {
        width = g_fb.width - x;
    }
    if (height > g_fb.height - y) {
        height = g_fb.height - y;
    }

    for (uint32_t py = 0; py < height; ++py) {
        for (uint32_t px = 0; px < width; ++px) {
            put_pixel(x + px, y + py, rgb);
        }
    }
}

void framebuffer_clear(uint32_t rgb)
{
    framebuffer_fill_rect(0, 0, g_fb.width, g_fb.height, rgb);
}

void framebuffer_draw_text(uint32_t x, uint32_t y, const char *text, uint32_t rgb, uint32_t scale)
{
    if (!g_available || text == 0 || scale == 0) {
        return;
    }

    uint32_t cursor_x = x;

    while (*text != '\0') {
        if (*text == ' ') {
            cursor_x += 6u * scale;
            text++;
            continue;
        }

        for (uint32_t row = 0; row < 7; ++row) {
            const uint8_t bits = glyph_row(*text, row);
            for (uint32_t col = 0; col < 5; ++col) {
                if ((bits & (1u << (4u - col))) != 0) {
                    framebuffer_fill_rect(cursor_x + col * scale,
                                          y + row * scale,
                                          scale, scale, rgb);
                }
            }
        }

        cursor_x += 6u * scale;
        text++;
    }
}

void framebuffer_draw_hex32(uint32_t x, uint32_t y, uint32_t value, uint32_t rgb, uint32_t scale)
{
    static const char digits[] = "0123456789ABCDEF";
    char text[11];

    text[0] = '0';
    text[1] = 'X';
    for (uint32_t i = 0; i < 8; ++i) {
        const uint32_t shift = 28u - i * 4u;
        text[2 + i] = digits[(value >> shift) & 0xFu];
    }
    text[10] = '\0';

    framebuffer_draw_text(x, y, text, rgb, scale);
}

void framebuffer_show_boot_splash(bool selftests_ok)
{
    if (!g_available) {
        return;
    }

    framebuffer_clear(0x101820u);

    const uint32_t band_height = g_fb.height / 8u;
    framebuffer_fill_rect(0, 0, g_fb.width, band_height, 0x26364Au);
    framebuffer_fill_rect(0, g_fb.height - band_height, g_fb.width, band_height,
                          selftests_ok ? 0x166534u : 0x7F1D1Du);

    uint32_t scale = g_fb.width / 90u;
    if (scale < 2u) {
        scale = 2u;
    }
    if (scale > 6u) {
        scale = 6u;
    }

    framebuffer_draw_text(g_fb.width / 12u, g_fb.height / 3u,
                          "NOOS", 0xF8FAFCu, scale + 1u);
    framebuffer_draw_text(g_fb.width / 12u, g_fb.height / 2u,
                          "RM-915", 0xCBD5E1u, scale);
    framebuffer_draw_text(g_fb.width / 12u, (g_fb.height * 2u) / 3u,
                          selftests_ok ? "SELFTEST PASS" : "SELFTEST FAIL",
                          selftests_ok ? 0x86EFACu : 0xFCA5A5u, scale);
}

void framebuffer_show_runtime(const struct noos_boot_info *boot,
                              const struct pmm_stats *pmm,
                              struct selftest_result tests)
{
    if (!g_available || boot == 0 || pmm == 0) {
        return;
    }

    framebuffer_clear(0x0B1220u);
    framebuffer_fill_rect(0, 0, g_fb.width, 72u, 0x1E293Bu);

    const uint32_t label = 0x94A3B8u;
    const uint32_t value = 0xF8FAFCu;
    const uint32_t good = 0x86EFACu;
    const uint32_t bad = 0xFCA5A5u;
    const uint32_t x = 28u;
    const uint32_t vx = 150u;
    const uint32_t scale = 2u;

    framebuffer_draw_text(x, 20u, "NOOS RM-915", value, 3u);

    framebuffer_draw_text(x, 105u, "BOOT", label, scale);
    framebuffer_draw_text(vx, 105u,
                          boot->protocol == NOOS_BOOT_DTB ? "DTB" :
                          boot->protocol == NOOS_BOOT_ATAGS ? "ATAGS" : "FAIL",
                          value, scale);

    framebuffer_draw_text(x, 155u, "RAM", label, scale);
    framebuffer_draw_hex32(vx, 155u, boot->total_memory_bytes, value, scale);

    framebuffer_draw_text(x, 205u, "FREE", label, scale);
    framebuffer_draw_hex32(vx, 205u, pmm->free_pages, value, scale);

    framebuffer_draw_text(x, 255u, "PERM", label, scale);
    framebuffer_draw_hex32(vx, 255u, pmm->permanent_pages, value, scale);

    framebuffer_draw_text(x, 305u, "FB", label, scale);
    framebuffer_draw_hex32(vx, 305u, boot->framebuffer.address, value, scale);

    framebuffer_draw_text(x, 380u, "TEST", label, scale);
    framebuffer_draw_text(vx, 380u,
                          tests.failed == 0 ? "PASS" : "FAIL",
                          tests.failed == 0 ? good : bad, scale);

    framebuffer_draw_text(x, 430u, "PASS", label, scale);
    framebuffer_draw_hex32(vx, 430u, tests.passed, value, scale);

    framebuffer_draw_text(x, 480u, "FAIL", label, scale);
    framebuffer_draw_hex32(vx, 480u, tests.failed,
                           tests.failed == 0 ? value : bad, scale);

    framebuffer_fill_rect(0, g_fb.height - 54u, g_fb.width, 54u,
                          tests.failed == 0 ? 0x14532Du : 0x7F1D1Du);
}
