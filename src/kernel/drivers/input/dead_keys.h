#pragma once

#include <stddef.h>

typedef enum {
    DEAD_NONE = 0,
    DEAD_ACUTE,     // ´
    DEAD_DIERESIS,  // ¨
    DEAD_GRAVE,     // `
    DEAD_TILDE,     // ~
    DEAD_CIRCUMFLEX // ^
} DeadKeyType;

typedef struct {
    DeadKeyType type;
    char base_char;
    char result_char;
} DeadKeyComposition;

typedef struct {
    DeadKeyType current_state;
    const DeadKeyComposition* compositions;
    size_t composition_count;
} DeadKeyProcessor;

void dead_key_processor_init(DeadKeyProcessor* processor);
char dead_key_processor_handle(DeadKeyProcessor* processor, char input_char, uint8_t dead_key_scancode, KeyModifierState modifiers);
void dead_key_processor_reset(DeadKeyProcessor* processor);