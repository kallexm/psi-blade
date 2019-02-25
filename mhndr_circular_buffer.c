#include "mhndr_circular_buffer.h"

#include <stdbool.h>
#include <string.h>

#include <stdio.h>

void debug_count1()
{
    static int counter = 0;
    counter++;
    printf("crc: %i\n", counter);
    return;
}

struct mhndr_crclrbuf
{
    uint8_t*  crclrbuf_mem_pool;
    uint8_t   size_of_elements;
    uint8_t   num_of_elements;
    uint8_t   head;
    uint8_t   tail;
};


struct mhndr_crclrbuf_mem_pool
{
    struct mhndr_crclrbuf  data_pool[MHNDR_CRCLRBUF_MAX_CIRCULAR_BUFFERS];
    bool                   data_used[MHNDR_CRCLRBUF_MAX_CIRCULAR_BUFFERS];
} p_pool;



mhndr_crclrbuf_handle mhndr_crclrbuf_create(mhndr_crclrbuf_config_t init_config)
{
    if (init_config.circular_buffer_mem_ptr == NULL ||
        init_config.size_of_elements == 0           ||
        init_config.num_of_elements == 0             )
    {
        return NULL;
    }

    for (int i = 0; i < MHNDR_CRCLRBUF_MAX_CIRCULAR_BUFFERS; i++)
    {
        if (p_pool.data_used[i] == false)
        {
            p_pool.data_used[i] = true;
            memset(&(p_pool.data_pool[i]), 0, sizeof(p_pool.data_pool[i]));

            p_pool.data_pool[i].crclrbuf_mem_pool = init_config.circular_buffer_mem_ptr;
            p_pool.data_pool[i].size_of_elements = init_config.size_of_elements;
            p_pool.data_pool[i].num_of_elements = init_config.num_of_elements;

            return &(p_pool.data_pool[i]);
        }
        
    }
    return NULL;
}



mhndr_crclrbuf_retval_t mhndr_crclrbuf_enqueue(mhndr_crclrbuf_handle circular_buffer, void* element)
{
    if (element == NULL)
    {
        return MHNDR_CRCLRBUF_INVALID_HANDLE;
    }

    if (circular_buffer->tail == circular_buffer->head + 1                                    ||
        circular_buffer->tail == circular_buffer->num_of_elements && circular_buffer->head == 0  )
    {
        return MHNDR_CRCLRBUF_BUFFER_FULL;
    }

    memcpy((void*)&(circular_buffer->crclrbuf_mem_pool[circular_buffer->tail]), (void*)element, circular_buffer->size_of_elements);
    if (circular_buffer->tail != circular_buffer->num_of_elements-1)
    {
        (circular_buffer->tail)++;
    }
    else
    {
        circular_buffer->tail = 0;
    }

    return MHNDR_CRCLRBUF_SUCCESS;
}



mhndr_crclrbuf_retval_t mhndr_crclrbuf_dequeue(mhndr_crclrbuf_handle circular_buffer, void* element)
{
    if (circular_buffer->head == circular_buffer->tail)
    {
        memset(element, 0, circular_buffer->size_of_elements);
        return MHNDR_CRCLRBUF_BUFFER_EMPTY;
    }

    memcpy(element, (void*)&(circular_buffer->crclrbuf_mem_pool[circular_buffer->head]), circular_buffer->size_of_elements);
    if (circular_buffer->head != circular_buffer->num_of_elements-1)
    {
        (circular_buffer->head)++;
    }
    else
    {
        circular_buffer->head = 0;
    }
    return MHNDR_CRCLRBUF_SUCCESS;
}
