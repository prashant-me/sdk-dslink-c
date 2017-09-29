#include <stdlib.h>

#include "dslink/mem/mem.h"
#include "dslink/col/ringbuffer.h"

#include <string.h>


int rb_init(Ringbuffer* rb, uint32_t size, size_t element_size)
{
    if(!rb) {
        return -1;
    }
    rb->data = dslink_malloc(size*element_size);
    if(!rb->data) {
        return -1;
    }

    rb->element_size = element_size;
    rb->size = size;
    rb->current = 0;
    rb->count = 0;

    return 0;
}

int rb_count(const Ringbuffer* rb)
{
    if(!rb) {
        return -1;
    }

    return rb->count;
}

int rb_push(Ringbuffer* rb, void* data)
{
    if(!rb) {
        return -1;
    }

    int res = 0;
    if(rb->count == rb->size) {
        res = 1;
    }

    size_t offset = rb->current * rb->element_size;
    memcpy((char*)rb->data + offset, data, rb->element_size);
    ++rb->current;
    if(rb->current == rb->size) {
        rb->current = 0;
    }
    ++rb->count;
    if(rb->count > rb->size) {
        rb->count = rb->size;
    }

    return res;
}

void* rb_front(const Ringbuffer* rb)
{
    if(!rb || rb->count == 0) {
        return NULL;
    }

    uint32_t index = 0;
    if(rb->current >= rb->count) {
        index = rb->current - rb->count;
    } else {
        index = rb->size - (rb->count - rb->current);
    }

    return (char*)rb->data + (index * rb->element_size);
}

int rb_pop(Ringbuffer* rb)
{
    if(!rb) {
        return -1;
    }

    if(rb->count > 0) {
        --rb->count;
    } else {
        return -1;
    }

    return 0;
}

int rb_free(Ringbuffer* rb)
{
    if(!rb) {
        return -1;
    }

    dslink_free(rb->data);

    return 0;
}
