#include <stdlib.h>

#include "dslink/mem/mem.h"
#include "dslink/col/vector.h"

#include <string.h>


int vector_init(Vector* vec, uint32_t initial_size, size_t element_size)
{
    if(!vec) {
        return -1;
    }
    vec->data = dslink_malloc(initial_size*element_size);
    if(!vec->data) {
        return -1;
    }

    vec->element_size = element_size;
    vec->capacity = initial_size;
    vec->size = 0;

    return 0;
}

int vector_count(const Vector* vec)
{
    if(!vec) {
        return 0;
    }

    return vec->size;
}

static int vector_resize(Vector* vec)
{
    if(!vec) {
        return -1;
    }
    if(vec->size >= vec->capacity) {
        uint8_t cap = vec->capacity * 2;
        void** data = dslink_realloc(vec->data, cap*vec->element_size);
        if(!data) {
            return -1;
        }
        vec->capacity = cap;
    }

    return 0;
}

long vector_append(Vector* vec, void* data)
{
    if(!vec) {
        return -1;
    }
    if(vec->size >= vec->capacity) {
        // not enough room left, resize
        if(vector_resize(vec) != 0) {
            return -1;
        }
    }
    size_t offset = vec->size*vec->element_size;
    memcpy((char*)vec->data + offset, data, vec->element_size);
    ++vec->size;

    return vec->size-1;
}

int vector_set(Vector* vec, uint32_t index, void* data)
{
    if(!vec || index >= vec->size) {
        return -1;
    }
    memcpy((char*)vec->data + (index*vec->element_size), data, vec->element_size);

    return 0;
}

void* vector_get(const Vector* vec, uint32_t index)
{
    if(!vec || index >= vec->size) {
        return NULL;
    }

    return (char*)vec->data + (index*vec->element_size);
}

int vector_remove(Vector* vec, uint32_t index)
{
    if(!vec || index >= vec->size) {
        return -1;
    }
    if(index != vec->size-1) {
        //memmove(&vec->data[index], &vec->data[index+1], (vec->size-(index+1))*sizeof(vec->data));

        memmove((char*)vec->data + (index*vec->element_size), (char*)vec->data + ((index+1)*vec->element_size), (vec->size-(index+1))*vec->element_size);
    }
    --(vec->size);

    return 0;
}

int vector_find(const Vector* vec, void* data, vector_comparison_fn_type cmp_fn)
{
    if(!vec || vec->size == 0) {
        return -1;
    }

    for(uint32_t n = 0; n < vec->size; ++n) {
        if(cmp_fn(data, (char*)vec->data + (n*vec->element_size)) == 0) {
            return n;
        }
    }

    return -1;
}

int vector_free(Vector* vec)
{
    if(!vec) {
        return -1;
    }

    dslink_free(vec->data);

    return 0;
}
