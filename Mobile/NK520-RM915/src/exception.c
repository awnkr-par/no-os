#include <noos/console.h>
#include <noos/exception.h>
#include <noos/memory.h>
#include <noos/panic.h>
#include <noos/qgic.h>

static struct noos_fault_state g_last_fault;

static uint32_t read_dfsr(void)
{
    uint32_t value;
    __asm__ volatile ("mrc p15, 0, %0, c5, c0, 0" : "=r"(value));
    return value;
}

static uint32_t read_ifsr(void)
{
    uint32_t value;
    __asm__ volatile ("mrc p15, 0, %0, c5, c0, 1" : "=r"(value));
    return value;
}

static uint32_t read_dfar(void)
{
    uint32_t value;
    __asm__ volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r"(value));
    return value;
}

static uint32_t read_ifar(void)
{
    uint32_t value;
    __asm__ volatile ("mrc p15, 0, %0, c6, c0, 2" : "=r"(value));
    return value;
}

static uint32_t exception_pc(enum noos_exception_type type, uint32_t lr)
{
    switch (type) {
    case NOOS_EXC_DATA_ABORT:
        return lr - 8u;
    case NOOS_EXC_PREFETCH_ABORT:
    case NOOS_EXC_IRQ:
    case NOOS_EXC_FIQ:
        return lr - 4u;
    default:
        return lr;
    }
}

static const char *exception_name(enum noos_exception_type type)
{
    switch (type) {
    case NOOS_EXC_UNDEFINED: return "undefined instruction";
    case NOOS_EXC_SVC: return "supervisor call";
    case NOOS_EXC_PREFETCH_ABORT: return "prefetch abort";
    case NOOS_EXC_DATA_ABORT: return "data abort";
    case NOOS_EXC_RESERVED: return "reserved vector";
    case NOOS_EXC_IRQ: return "IRQ";
    case NOOS_EXC_FIQ: return "FIQ";
    default: return "unknown";
    }
}

_Noreturn void exception_handle(uint32_t raw_type, uint32_t spsr,
                                const struct noos_exception_frame *frame)
{
    const enum noos_exception_type type = (enum noos_exception_type)raw_type;

    g_last_fault.type = type;
    g_last_fault.spsr = spsr;
    memcpy(&g_last_fault.frame, frame, sizeof(g_last_fault.frame));
    g_last_fault.fault_pc = exception_pc(type, frame->lr);
    g_last_fault.dfsr = read_dfsr();
    g_last_fault.dfar = read_dfar();
    g_last_fault.ifsr = read_ifsr();
    g_last_fault.ifar = read_ifar();

    console_write("\nEXCEPTION: ");
    console_write(exception_name(type));
    console_write("\nPC: ");
    console_write_hex32(g_last_fault.fault_pc);
    console_write(" LR: ");
    console_write_hex32(frame->lr);
    console_write(" SPSR: ");
    console_write_hex32(spsr);
    console_write("\nDFSR: ");
    console_write_hex32(g_last_fault.dfsr);
    console_write(" DFAR: ");
    console_write_hex32(g_last_fault.dfar);
    console_write("\nIFSR: ");
    console_write_hex32(g_last_fault.ifsr);
    console_write(" IFAR: ");
    console_write_hex32(g_last_fault.ifar);
    console_write("\n");

    panic("unhandled CPU exception");
}

void exception_irq_handle(uint32_t spsr,
                          const struct noos_exception_frame *frame)
{
    if (qgic_handle_one()) {
        return;
    }

    exception_handle(NOOS_EXC_IRQ, spsr, frame);
}

void arm_irq_enable(void)
{
    __asm__ volatile ("cpsie i" ::: "memory");
}

void arm_irq_disable(void)
{
    __asm__ volatile ("cpsid i" ::: "memory");
}

const struct noos_fault_state *exception_last_fault(void)
{
    return &g_last_fault;
}
