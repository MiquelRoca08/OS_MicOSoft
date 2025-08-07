#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    void* data;
    size_t element_size;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
} CircularBuffer;

bool circular_buffer_init(CircularBuffer* buffer, void* storage, size_t element_size, size_t capacity);
bool circular_buffer_push(CircularBuffer* buffer, const void* element);
bool circular_buffer_pop(CircularBuffer* buffer, void* element);
bool circular_buffer_is_empty(const CircularBuffer* buffer);
bool circular_buffer_is_full(const CircularBuffer* buffer);
size_t circular_buffer_count(const CircularBuffer* buffer);
void circular_buffer_clear(CircularBuffer* buffer);