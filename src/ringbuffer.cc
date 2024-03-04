#include "ringbuffer.h"

void initBuffer(RingBuffer *buffer) {
    buffer->head = 0;
    buffer->tail = 0;
}

int isBufferFull(const RingBuffer *buffer) {
    return (buffer->head + 1) % BUFFER_SIZE == buffer->tail;
}

int isBufferEmpty(const RingBuffer *buffer) {
    return buffer->head == buffer->tail;
}

void printBuffer(const RingBuffer *buffer) {
    int i = buffer->tail;

    // uart_printf(CONSOLE,"Printing buffer: ");
    while (i != buffer->head) {
        uart_putc(CONSOLE, buffer->data[i]);
        i = (i + 1) % BUFFER_SIZE;
    }
    uart_putc(CONSOLE,'\n');
}

void push(RingBuffer *buffer, unsigned char value) {
    buffer->data[buffer->head] = value;
    buffer->head = (buffer->head + 1) % BUFFER_SIZE;

    if (buffer->head == buffer->tail) {
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE; // Overwrite the oldest value
    }
}

unsigned char pop(RingBuffer *buffer) {
    char value = '\0'; // Default value for an empty buffer
    if (!isBufferEmpty(buffer)) {
        value = buffer->data[buffer->tail];
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
    } else {
        // uart_printf(CONSOLE,"Buffer is empty. Cannot pop.\n");
    }
    return value;
}