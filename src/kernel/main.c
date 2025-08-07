#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include <hal/hal.h>
#include <arch/i686/cpu/irq.h>
#include <arch/i686/io/io.h>
#include <debug.h>
#include <boot/bootparams.h>
#include <drivers/input/keyboard_device.h>
#include <arch/i686/io/vga_text.h>


extern void _init();

void crash_me();

static KeyboardDevice g_keyboard;

void keyboard_interrupt_handler_wrapper(Registers* regs) {
    keyboard_device_interrupt_handler(&g_keyboard);
}

void keyboard_init_refactored(void) {
    if (!keyboard_device_init(&g_keyboard, 0x60, 0x64)) {
        log_err("Keyboard", "Failed to initialize keyboard device");
        return;
    }
    
    i686_IRQ_RegisterHandler(1, keyboard_interrupt_handler_wrapper);
    log_info("Keyboard", "Keyboard driver initialized successfully");
}

void keyboard_process_events(void) {
    ProcessedKeyEvent event;
    
    while (keyboard_device_get_event(&g_keyboard, &event)) {
        if (!event.is_special) {
            insert_char_multiline(event.character);
        } else {
            handle_special_key(event.special_code);
        }
    }
}

void timer(Registers* regs)
{
    printf(".");
}

void start(BootParams* bootParams)
{   
    _init();

    HAL_Initialize();

    keyboard_init_refactored();  // nueva init
    screen_init();
    
    i686_EnableInterrupts();
    
    log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) 
    {
        log_debug("Main", "MEM: start=0x%llx length=0x%llx type=%x", 
            bootParams->Memory.Regions[i].Begin,
            bootParams->Memory.Regions[i].Length,
            bootParams->Memory.Regions[i].Type);
    }

    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warning msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");
    printf("OS MiqOSoft v0.16.1\n");
    printf("This operating system is under construction.\n");

    g_ScreenY = 2;
    g_ScreenX = 0;
    redraw_input_line();

    while (1) {
        keyboard_process_events();  // nuevo procesamiento
    }

end:
    for (;;);
}