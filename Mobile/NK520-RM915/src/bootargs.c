#include <noos/bootargs.h>
#include <stddef.h>

static bool text_equal_n(const char *a, const char *b, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

static size_t text_length(const char *text)
{
    size_t length = 0;
    while (text != 0 && text[length] != '\0') {
        length++;
    }
    return length;
}

static const char *find_value(const char *command_line, const char *key, size_t *value_length)
{
    if (command_line == 0 || key == 0) {
        return 0;
    }

    const size_t key_length = text_length(key);
    const char *cursor = command_line;

    while (*cursor != '\0') {
        while (*cursor == ' ') {
            cursor++;
        }

        const char *token = cursor;
        while (*cursor != '\0' && *cursor != ' ') {
            cursor++;
        }

        const size_t token_length = (size_t)(cursor - token);
        if (token_length > key_length &&
            text_equal_n(token, key, key_length) &&
            token[key_length] == '=') {
            const char *value = token + key_length + 1u;
            *value_length = token_length - key_length - 1u;
            return value;
        }
    }

    return 0;
}

bool bootargs_get_bool(const char *command_line, const char *key, bool fallback)
{
    size_t length = 0;
    const char *value = find_value(command_line, key, &length);
    if (value == 0) {
        return fallback;
    }

    if ((length == 1 && value[0] == '1') ||
        (length == 2 && value[0] == 'o' && value[1] == 'n') ||
        (length == 4 && text_equal_n(value, "true", 4)) ||
        (length == 3 && text_equal_n(value, "yes", 3))) {
        return true;
    }

    if ((length == 1 && value[0] == '0') ||
        (length == 3 && text_equal_n(value, "off", 3)) ||
        (length == 5 && text_equal_n(value, "false", 5)) ||
        (length == 2 && text_equal_n(value, "no", 2))) {
        return false;
    }

    return fallback;
}

uint32_t bootargs_get_u32(const char *command_line, const char *key, uint32_t fallback)
{
    size_t length = 0;
    const char *value = find_value(command_line, key, &length);
    if (value == 0 || length == 0) {
        return fallback;
    }

    uint32_t base = 10;
    size_t index = 0;

    if (length >= 3 && value[0] == '0' &&
        (value[1] == 'x' || value[1] == 'X')) {
        base = 16;
        index = 2;
    }

    uint32_t result = 0;
    for (; index < length; ++index) {
        uint32_t digit;
        const char c = value[index];

        if (c >= '0' && c <= '9') {
            digit = (uint32_t)(c - '0');
        } else if (base == 16 && c >= 'a' && c <= 'f') {
            digit = (uint32_t)(c - 'a') + 10u;
        } else if (base == 16 && c >= 'A' && c <= 'F') {
            digit = (uint32_t)(c - 'A') + 10u;
        } else {
            return fallback;
        }

        if (digit >= base) {
            return fallback;
        }

        if (base == 10u) {
            if (result > 429496729u ||
                (result == 429496729u && digit > 5u)) {
                return fallback;
            }
            result = result * 10u + digit;
        } else {
            if (result > 0x0FFFFFFFu) {
                return fallback;
            }
            result = (result << 4) | digit;
        }
    }

    return result;
}

void bootargs_defaults(struct noos_boot_options *options)
{
    if (options == 0) {
        return;
    }

    options->splash = true;
    options->selftest = true;
    options->mmu_plan = true;
    options->scheduler_test = true;
    options->timer_irq = false;
    options->log_level = 2u;
}

void bootargs_parse(struct noos_boot_options *options, const char *command_line)
{
    if (options == 0) {
        return;
    }

    bootargs_defaults(options);
    options->splash = bootargs_get_bool(command_line, "noos.splash", options->splash);
    options->selftest = bootargs_get_bool(command_line, "noos.selftest", options->selftest);
    options->mmu_plan = bootargs_get_bool(command_line, "noos.mmu-plan", options->mmu_plan);
    options->scheduler_test = bootargs_get_bool(command_line, "noos.sched-test", options->scheduler_test);
    options->timer_irq = bootargs_get_bool(command_line, "noos.timer-irq", options->timer_irq);
    options->log_level = bootargs_get_u32(command_line, "noos.loglevel", options->log_level);
}
