#include "keyboard_device.h"
#include <arch/i686/io/io.h>
#include <memory.h>

// Hardware constants
#define KEYBOARD_STATUS_OUTPUT_BUFFER_FULL  0x01
#define KEYBOARD_STATUS_INPUT_BUFFER_FULL   0x02

// Scan code constants
#define SCANCODE_EXTENDED_KEY               0xE0
#define SCANCODE_KEY_RELEASE                0x80
#define SCANCODE_SHIFT_L                    0x2A
#define SCANCODE_SHIFT_R                    0x36
#define SCANCODE_CTRL_L                     0x1D
#define SCANCODE_ALT_L                      0x38
#define SCANCODE_CAPS_LOCK                  0x3A

static uint8_t keyboard_read_scancode(KeyboardDevice* device) {
    while (!(i686_inb(device->status_port) & KEYBOARD_STATUS_OUTPUT_BUFFER_FULL)) {
        // Wait for data
    }
    return i686_inb(device->data_port);
}

static void update_modifier_state(KeyboardDevice* device, uint8_t scancode, bool is_press) {
    switch (scancode) {
        case SCANCODE_SHIFT_L:
        case SCANCODE_SHIFT_R:
            if (is_press) {
                device->modifiers |= KEY_STATE_SHIFT;
            } else {
                device->modifiers &= ~KEY_STATE_SHIFT;
            }
            break;
            
        case SCANCODE_CTRL_L:
            if (is_press) {
                device->modifiers |= KEY_STATE_CTRL;
            } else {
                device->modifiers &= ~KEY_STATE_CTRL;
            }
            break;
            
        case SCANCODE_ALT_L:
            if (device->extended_key_pending) {
                // AltGr
                if (is_press) {
                    device->modifiers |= KEY_STATE_ALTGR;
                } else {
                    device->modifiers &= ~KEY_STATE_ALTGR;
                }
            } else {
                // Regular Alt
                if (is_press) {
                    device->modifiers |= KEY_STATE_ALT;
                } else {
                    device->modifiers &= ~KEY_STATE_ALT;
                }
            }
            break;
            
        case SCANCODE_CAPS_LOCK:
            if (is_press) {
                device->caps_lock = !device->caps_lock;
            }
            break;
    }
}

static void process_raw_event(KeyboardDevice* device, const RawKeyEvent* raw_event) {
    // Skip modifier-only events for processed output
    if (raw_event->scancode == SCANCODE_SHIFT_L || 
        raw_event->scancode == SCANCODE_SHIFT_R ||
        raw_event->scancode == SCANCODE_CTRL_L ||
        raw_event->scancode == SCANCODE_ALT_L) {
        return;
    }
    
    // Only process key presses for character generation
    if (raw_event->type != KEY_EVENT_PRESS) {
        return;
    }
    
    char character = keymap_get_character(device->current_layout, 
                                        raw_event->scancode, 
                                        device->modifiers);
    
    if (character != 0) {
        // Handle dead keys
        if (keymap_is_dead_key(device->current_layout, raw_event->scancode)) {
            // This is a dead key press, just update the processor state
            dead_key_processor_handle(&device->dead_key_processor, 0, 
                                    raw_event->scancode, device->modifiers);
            return;
        }
        
        // Process through dead key processor
        char final_char = dead_key_processor_handle(&device->dead_key_processor, 
                                                  character, 0, device->modifiers);
        
        if (final_char != 0) {
            ProcessedKeyEvent processed_event = {
                .character = final_char,
                .is_special = false,
                .special_code = 0,
                .modifiers = device->modifiers,
                .timestamp = raw_event->timestamp
            };
            
            if (!circular_buffer_push(&device->processed_events, &processed_event)) {
                device->dropped_events++;
            }
        }
    }
}

bool keyboard_device_init(KeyboardDevice* device, uint16_t data_port, uint16_t status_port) {
    if (!device) return false;
    
    // Clear the device structure
    memset(device, 0, sizeof(KeyboardDevice));
    
    // Initialize hardware interface
    device->data_port = data_port;
    device->status_port = status_port;
    
    // Initialize buffers
    if (!circular_buffer_init(&device->raw_events, 
                             device->raw_buffer_storage,
                             sizeof(RawKeyEvent),
                             KEYBOARD_RAW_BUFFER_SIZE)) {
        return false;
    }
    
    if (!circular_buffer_init(&device->processed_events,
                             device->processed_buffer_storage,
                             sizeof(ProcessedKeyEvent),
                             KEYBOARD_PROCESSED_BUFFER_SIZE)) {
        return false;
    }
    
    // Initialize processors
    dead_key_processor_init(&device->dead_key_processor);
    
    // Set default layout
    device->current_layout = &LAYOUT_ES_ES;
    
    // Clear keyboard buffer
    while (i686_inb(device->status_port) & KEYBOARD_STATUS_OUTPUT_BUFFER_FULL) {
        i686_inb(device->data_port);
    }
    
    return true;
}

void keyboard_device_interrupt_handler(KeyboardDevice* device) {
    if (!device) return;
    
    uint8_t scancode = keyboard_read_scancode(device);
    device->total_key_events++;
    
    // Handle extended key sequence
    if (scancode == SCANCODE_EXTENDED_KEY) {
        device->extended_key_pending = true;
        return;
    }
    
    // Determine if this is a key press or release
    bool is_release = (scancode & SCANCODE_KEY_RELEASE) != 0;
    if (is_release) {
        scancode &= ~SCANCODE_KEY_RELEASE;
    }
    
    // Update modifier state
    update_modifier_state(device, scancode, !is_release);
    
    // Create raw event
    RawKeyEvent raw_event = {
        .scancode = scancode,
        .type = is_release ? KEY_EVENT_RELEASE : KEY_EVENT_PRESS,
        .modifiers = device->modifiers,
        .timestamp = device->total_key_events  // Simple timestamp
    };
    
    // Add to raw buffer
    if (!circular_buffer_push(&device->raw_events, &raw_event)) {
        device->dropped_events++;
    }
    
    // Process into character events
    process_raw_event(device, &raw_event);
    
    // Reset extended key flag
    device->extended_key_pending = false;
}

bool keyboard_device_get_event(KeyboardDevice* device, ProcessedKeyEvent* event) {
    if (!device || !event) return false;
    return circular_buffer_pop(&device->processed_events, event);
}

bool keyboard_device_peek_event(KeyboardDevice* device, ProcessedKeyEvent* event) {
    if (!device || !event) return false;
    
    if (circular_buffer_is_empty(&device->processed_events)) {
        return false;
    }
    
    // Peek at the next event without removing it
    size_t tail = device->processed_events.tail;
    *event = device->processed_buffer_storage[tail];
    return true;
}

void keyboard_device_flush(KeyboardDevice* device) {
    if (!device) return;
    
    circular_buffer_clear(&device->raw_events);
    circular_buffer_clear(&device->processed_events);
    dead_key_processor_reset(&device->dead_key_processor);
}

// State query functions
bool keyboard_device_is_caps_lock_on(const KeyboardDevice* device) {
    return device ? device->caps_lock : false;
}

bool keyboard_device_is_shift_pressed(const KeyboardDevice* device) {
    return device ? (device->modifiers & KEY_STATE_SHIFT) != 0 : false;
}

KeyModifierState keyboard_device_get_modifiers(const KeyboardDevice* device) {
    return device ? device->modifiers : KEY_STATE_NORMAL;
}