#ifndef MHNDR_CIRCULAR_BUFFER_H__
#define MHNDR_CIRCULAR_BUFFER_H__

#include <stdint.h>

#define MHNDR_CRCLRBUF_MAX_CIRCULAR_BUFFERS 1


typedef struct mhndr_crclrbuf* mhndr_crclrbuf_handle;


typedef enum
{
    MHNDR_CRCLRBUF_SUCCESS,
    MHNDR_CRCLRBUF_FAILURE,
    MHNDR_CRCLRBUF_BUFFER_FULL,
    MHNDR_CRCLRBUF_BUFFER_EMPTY,
    MHNDR_CRCLRBUF_INVALID_HANDLE,
} mhndr_crclrbuf_retval_t;


typedef struct
{
    uint8_t* circular_buffer_mem_ptr;
    uint8_t   size_of_elements;
    uint8_t   num_of_elements;
} mhndr_crclrbuf_config_t;


#define MHNDR_CRCLRBUF_CONFIG_MEM_DEF(cfg_name_, num_elem_, size_elem_) \
    uint8_t (cfg_name_ ## _buffer)[(num_elem_)*(size_elem_)] = {0};     \
    mhndr_crclrbuf_config_t (cfg_name_) = {                             \
        (uint8_t*)(cfg_name_ ## _buffer),                               \
        (size_elem_),                                                   \
        (num_elem_)                                                     \
    }                                                                   \


mhndr_crclrbuf_handle mhndr_crclrbuf_create(mhndr_crclrbuf_config_t init_config);


mhndr_crclrbuf_retval_t mhndr_crclrbuf_dequeue(mhndr_crclrbuf_handle circular_buffer, void* element);

mhndr_crclrbuf_retval_t mhndr_crclrbuf_enqueue(mhndr_crclrbuf_handle circular_buffer, void* element);

#endif // MHNDR_CIRCULAR_BUFFER_H__
