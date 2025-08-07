#pragma once
#include "keyboard_types.h"
#include "keymap.h"
#include "dead_keys.h"
#include "input_buffer.h"

#define KEYBOARD_RAW_BUFFER_SIZE 64
#define KEYBOARD_PROCESSED_BUFFER_SIZE 32

typedef struct {
    // Hardware interface
    uint16_t data_port;
    uint16_t status_port;
    
    // Layout and processing
    const KeyboardLayout* current_layout;
    DeadKeyProcessor dead_key_processor;
    
    // State tracking
    KeyModifierState modifiers;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
    bool extended_key_pending;
    
    // Buffers
    CircularBuffer raw_events;
    CircularBuffer processed_events;
    
    // Storage for buffers
    RawKeyEvent raw_buffer_storage[KEYBOARD_RAW_BUFFER_SIZE];
    ProcessedKeyEvent processed_buffer_storage[KEYBOARD_PROCESSED_BUFFER_SIZE];
    
    // Statistics
    uint32_t total_key_events;
    uint32_t dropped_events;
} KeyboardDevice;

// Public interface
bool keyboard_device_init(KeyboardDevice* device, uint16_t data_port, uint16_t status_port);
void keyboard_device_set_layout(KeyboardDevice* device, const KeyboardLayout* layout);
void keyboard_device_interrupt_handler(KeyboardDevice* device);
bool keyboard_device_get_event(KeyboardDevice* device, ProcessedKeyEvent* event);
bool keyboard_device_peek_event(KeyboardDevice* device, ProcessedKeyEvent* event);
void keyboard_device_flush(KeyboardDevice* device);

// State queries
bool keyboard_device_is_caps_lock_on(const KeyboardDevice* device);
bool keyboard_device_is_shift_pressed(const KeyboardDevice* device);
KeyModifierState keyboard_device_get_modifiers(const KeyboardDevice* device);
