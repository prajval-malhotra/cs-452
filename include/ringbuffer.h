#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "util.h"
#include "rpi.h"

#define BUFFER_SIZE 2048

struct RingBuffer {
    unsigned char data[BUFFER_SIZE];
    int head;
    int tail;
};

void initBuffer(RingBuffer *buffer);
int isBufferFull(const RingBuffer *buffer);
int isBufferEmpty(const RingBuffer *buffer);
void printBuffer(const RingBuffer *buffer);
void push(RingBuffer *buffer, unsigned char value);
unsigned char pop(RingBuffer *buffer);

#endif