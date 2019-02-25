#include "pb_circular_buffer.h"

#include <string.h>

pb_crclrbuf_retval_t pb_crclrbuf_enqueue(pb_crclrbuf_t* circular_buffer, void* element)
{
    if (element == NULL)
    {
        return PB_CRCLRBUF_INVALID_HANDLE;
    }

    if (circular_buffer->num_of_elements == circular_buffer->max_elements)
    {
        return PB_CRCLRBUF_BUFFER_FULL;
    }

    void* to_memory      = (void*)&(circular_buffer->p_circular_buffer[circular_buffer->tail*circular_buffer->size_of_elements]);
    void* from_memory    = (void*)element;
    uint32_t memory_size = circular_buffer->size_of_elements;
    memcpy(to_memory, from_memory, memory_size);
    (circular_buffer->num_of_elements)++;

    if (circular_buffer->tail != circular_buffer->max_elements-1)
    {
        (circular_buffer->tail)++;
    }
    else
    {
        circular_buffer->tail = 0;
    }

    return PB_CRCLRBUF_SUCCESS;
}



pb_crclrbuf_retval_t pb_crclrbuf_dequeue(pb_crclrbuf_t* circular_buffer, void* element)
{
    if (circular_buffer->num_of_elements == 0)
    {
        memset(element, 0, circular_buffer->size_of_elements);
        return PB_CRCLRBUF_BUFFER_EMPTY;
    }

    void* to_memory      = (void*)element;
    void* from_memory    = (void*)&(circular_buffer->p_circular_buffer[circular_buffer->head*circular_buffer->size_of_elements]);
    uint32_t memory_size = circular_buffer->size_of_elements;
    memcpy(to_memory, from_memory, memory_size);
    (circular_buffer->num_of_elements)--;

    if (circular_buffer->head != circular_buffer->max_elements-1)
    {
        (circular_buffer->head)++;
    }
    else
    {
        circular_buffer->head = 0;
    }

    return PB_CRCLRBUF_SUCCESS;
}



pb_crclrbuf_retval_t pb_crclrbuf_reset(pb_crclrbuf_t* circular_buffer)
{
    circular_buffer->num_of_elements = 0;
    circular_buffer->head            = 0;
    circular_buffer->tail            = 0;
    return PB_CRCLRBUF_SUCCESS;
}



bool pb_crclrbuf_is_empty(pb_crclrbuf_t* circular_buffer)
{
    if (circular_buffer->num_of_elements == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}



bool pb_crclrbuf_is_full(pb_crclrbuf_t* circular_buffer)
{
    if (circular_buffer->num_of_elements == circular_buffer->max_elements)
    {
        return true;
    }
    else
    {
        return false;
    }
}



uint16_t pb_crclrbuf_space_used(pb_crclrbuf_t* circular_buffer)
{
    return circular_buffer->num_of_elements;
}



uint16_t pb_crclrbuf_capacity(pb_crclrbuf_t* circular_buffer)
{
    return circular_buffer->max_elements;
}



uint16_t pb_crclrbuf_elem_size(pb_crclrbuf_t* circular_buffer)
{
    return circular_buffer->size_of_elements;
}
