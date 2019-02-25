#ifndef PB_CIRCULAR_BUFFER_H__
#define PB_CIRCULAR_BUFFER_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    PB_CRCLRBUF_SUCCESS,
    PB_CRCLRBUF_FAILURE,
    PB_CRCLRBUF_BUFFER_FULL,
    PB_CRCLRBUF_BUFFER_EMPTY,
    PB_CRCLRBUF_INVALID_HANDLE,
} pb_crclrbuf_retval_t;


typedef struct
{
    uint8_t*  p_circular_buffer;
    uint8_t   size_of_elements;
    uint8_t   max_elements;
    uint16_t  num_of_elements;
    uint16_t  head;
    uint16_t  tail;
} pb_crclrbuf_t;


#define PB_CRCLRBUF_DEF(name_, num_elem_, size_elem_)           \
    uint8_t (name_ ## _buffer)[(num_elem_)*(size_elem_)] = {0}; \
    pb_crclrbuf_t (name_) = {                                   \
        (uint8_t*)(name_ ## _buffer),                           \
        (size_elem_),                                           \
        (num_elem_),                                            \
        0,                                                      \
        0,                                                      \
        0                                                       \
    }                                                           \
    /* pb_crclrbuf_t* (name_) = &(name_ ## _data)                  \ */


pb_crclrbuf_retval_t pb_crclrbuf_enqueue(pb_crclrbuf_t* circular_buffer, void* element);
pb_crclrbuf_retval_t pb_crclrbuf_dequeue(pb_crclrbuf_t* circular_buffer, void* element);

pb_crclrbuf_retval_t pb_crclrbuf_reset(pb_crclrbuf_t* circular_buffer);

bool     pb_crclrbuf_is_empty(pb_crclrbuf_t* circular_buffer);
bool     pb_crclrbuf_is_full(pb_crclrbuf_t* circular_buffer);
uint16_t pb_crclrbuf_space_used(pb_crclrbuf_t* circular_buffer);
uint16_t pb_crclrbuf_capacity(pb_crclrbuf_t* circular_buffer);
uint16_t pb_crclrbuf_elem_size(pb_crclrbuf_t* circular_buffer);


#endif // PB_CIRCULAR_BUFFER_H__
