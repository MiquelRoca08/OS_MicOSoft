#pragma once
#include "keyboard_types.h"
#include <stddef.h>

typedef struct {
    uint8_t scancode;
    char normal;
    char shift;
    char altgr;
    bool is_dead_key;
} KeyMapping;

typedef struct {
    const KeyMapping* mappings;
    size_t mapping_count;
    const char* layout_name;
} KeyboardLayout;

// Spanish keyboard layout
extern const KeyboardLayout LAYOUT_ES_ES;

char keymap_get_character(const KeyboardLayout* layout, uint8_t scancode, KeyModifierState modifiers);
bool keymap_is_dead_key(const KeyboardLayout* layout, uint8_t scancode);
