#include "hal.h"
#include <arch/i686/cpu/gdt.h>
#include <arch/i686/cpu/idt.h>
#include <arch/i686/cpu/isr.h>
#include <arch/i686/cpu/irq.h>
#include <arch/i686/io/vga_text.h>

void HAL_Initialize()
{
    VGA_clrscr();
    i686_GDT_Initialize();
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
}