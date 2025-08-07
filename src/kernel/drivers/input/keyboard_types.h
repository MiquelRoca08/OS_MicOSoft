#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    KEY_EVENT_PRESS,
    KEY_EVENT_RELEASE,
    KEY_EVENT_REPEAT
} KeyEventType;

typedef enum {
    KEY_STATE_NORMAL,
    KEY_STATE_SHIFT,
    KEY_STATE_CTRL,
    KEY_STATE_ALT,
    KEY_STATE_ALTGR
} KeyModifierState;

typedef struct {
    uint8_t scancode;
    KeyEventType type;
    KeyModifierState modifiers;
    uint32_t timestamp;
} RawKeyEvent;

typedef struct {
    char character;
    bool is_special;
    uint32_t special_code;  // For arrow keys, function keys, etc.
    KeyModifierState modifiers;
    uint32_t timestamp;
} ProcessedKeyEvent;