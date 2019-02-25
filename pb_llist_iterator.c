#include "pb_llist_iterator.h"

#include <stdbool.h>
#include <string.h>

#define HEAD next[0]
#define TAIL prev[0]


pb_llist_iterator_retval_t pb_llist_iterator_init(pb_llist_iterator_t* iterator)
{
    return PB_LLIST_ITERATOR_SUCCESS;
}


pb_llist_iterator_retval_t pb_llist_iterator_content_get(pb_llist_iterator_t* iterator, uint8_t* content, uint16_t content_length)
{
    pb_llist_t* list = iterator->list;

    uint16_t copy_length = (list->content_size < content_length ? \
                            list->content_size : content_length);
    memcpy(&(list->content[iterator->current_index]), content, copy_length);

    return PB_LLIST_ITERATOR_SUCCESS;
}


pb_llist_iterator_retval_t pb_llist_iterator_remove_current(pb_llist_iterator_t* iterator, uint8_t* content, uint16_t content_length)
{
    pb_llist_t* list = iterator->list;

    uint16_t removed_node_index = iterator->current_index;
    iterator->current_index = 0;

    if (list->prev[removed_node_index] != 0)
    {
        uint16_t prev_node_index = list->prev[removed_node_index];

        list->next[prev_node_index] = list->next[removed_node_index];
        list->prev[removed_node_index] = 0;

        iterator->current_index = prev_node_index;
    }
    else
    {
        list->HEAD = list->next[removed_node_index];
        list->prev[removed_node_index] = 0;
    }

    if (list->next[removed_node_index] != 0)
    {
        uint16_t next_node_index = list->next[removed_node_index];

        list->prev[next_node_index] = list->prev[removed_node_index];
        list->next[removed_node_index] = 0;

        iterator->current_index = next_node_index;
    }
    else
    {
        list->TAIL = list->prev[removed_node_index];
        list->next[removed_node_index] = 0;
    }

    list->space_used = list->space_used - 1;

    uint16_t copy_length = (list->content_size < content_length ? \
                            list->content_size : content_length);
    memcpy(content, &(list->content[removed_node_index]), copy_length);

    return PB_LLIST_ITERATOR_SUCCESS;
}


pb_llist_iterator_retval_t pb_llist_iterator_insert_before(pb_llist_iterator_t* iterator, uint8_t* content, uint16_t content_length, uint16_t* insert_index)
{
    pb_llist_t* list = iterator->list;

    /* Check if sufficient space in list and return if that is not the case */
    if (list->space_used == list->max_nodes)
    {
        return PB_LLIST_ITERATOR_INSUFFICIENT_MEMORY;
    }

    /* Find free space */
    bool     found_space    = false;
    uint16_t new_node_index = 0;
    uint16_t counter        = list->max_nodes;
    while (found_space == false && counter > 0)
    {
        if (list->next[list->space_search_index] == 0 &&
            list->prev[list->space_search_index] == 0)
        {   
            found_space    = true;
            new_node_index = list->space_search_index;
        }
        
        if (list->space_search_index != list->max_nodes)
        {
            list->space_search_index++;
        }
        else
        {
            list->space_search_index = 1;
        }
        counter--;
    }

    /* Check if free space was found. Return failure if not */
    if (found_space == false)
    {
        return PB_LLIST_ITERATOR_FAILURE;
    }

    /* Copy content to the free space */
    uint16_t copy_length = (list->content_size < content_length ? \
                            list->content_size : content_length);
    memcpy(&(list->content[new_node_index]), content, copy_length);
    *insert_index = new_node_index;

    /* Insert node in the correct spot in the list */
    if (iterator->current_index == 0)
    {
        if (list->HEAD == 0)
        {
            list->HEAD                 = new_node_index;
            list->prev[new_node_index] = 0;

            list->TAIL                 = new_node_index;
            list->next[new_node_index] = 0;
        }
        else
        {
            uint16_t head_node_index = list->HEAD;

            list->prev[head_node_index] = new_node_index;
            list->next[new_node_index]  = head_node_index;

            list->HEAD                  = new_node_index;
            list->prev[new_node_index]  = 0;
        }

        iterator->current_index = new_node_index;
    }
    else
    {
        uint16_t curr_node = iterator->current_index;
        if (list->next[curr_node] != 0)
        {
            uint16_t prev_node_index    = list->prev[curr_node];

            list->next[prev_node_index] = new_node_index;
            list->prev[new_node_index]  = prev_node_index;

            list->prev[curr_node]       = new_node_index;
            list->next[new_node_index]  = curr_node;
        }
        else
        {
            list->HEAD                 = new_node_index;
            list->prev[new_node_index] = 0;

            list->prev[curr_node]      = new_node_index;
            list->next[new_node_index] = curr_node;
        }
    }
    list->space_used = list->space_used + 1;

    return PB_LLIST_ITERATOR_SUCCESS;
}


pb_llist_iterator_retval_t pb_llist_iterator_insert_after(pb_llist_iterator_t* iterator, uint8_t* content, uint16_t content_length, uint16_t* insert_index)
{
    pb_llist_t* list = iterator->list;

    /* Check if sufficient space in list and return if that is not the case */
    if (list->space_used == list->max_nodes)
    {
        return PB_LLIST_ITERATOR_INSUFFICIENT_MEMORY;
    }

    /* Find free space */
    bool     found_space    = false;
    uint16_t new_node_index = 0;
    uint16_t counter        = list->max_nodes;
    while (found_space == false && counter > 0)
    {
        if (list->next[list->space_search_index] == 0 &&
            list->prev[list->space_search_index] == 0)
        {   
            found_space    = true;
            new_node_index = list->space_search_index;
        }
        
        if (list->space_search_index != list->max_nodes)
        {
            list->space_search_index++;
        }
        else
        {
            list->space_search_index = 1;
        }
        counter--;
    }

    /* Check if free space was found. Return failure if not */
    if (found_space == false)
    {
        return PB_LLIST_ITERATOR_FAILURE;
    }

    /* Copy content to the free space */
    uint16_t copy_length = (list->content_size < content_length ? \
                            list->content_size : content_length);
    memcpy(&(list->content[new_node_index]), content, copy_length);
    *insert_index = new_node_index;

    /* Insert node in the correct spot in the list */
    if (iterator->current_index == 0)
    {
        if (list->TAIL == 0)
        {
            list->TAIL                 = new_node_index;
            list->next[new_node_index] = 0;
            list->HEAD                 = new_node_index;
            list->prev[new_node_index] = 0;
        }
        else
        {
            uint16_t tail_node_index    = list->TAIL;
            list->next[tail_node_index] = new_node_index;
            list->prev[new_node_index]  = tail_node_index;
            list->TAIL                  = new_node_index;
            list->next[new_node_index]  = 0;
        }
        iterator->current_index = new_node_index;
    }
    else
    {
        uint16_t curr_node = iterator->current_index;
        if (list->next[curr_node] != 0)
        {
            uint16_t next_node_index    = list->next[curr_node];
            list->prev[next_node_index] = new_node_index;
            list->next[new_node_index]  = next_node_index;
            list->next[curr_node]       = new_node_index;
            list->prev[new_node_index]  = curr_node;
        }
        else
        {
            list->TAIL                 = new_node_index;
            list->next[new_node_index] = 0;
            list->next[curr_node]      = new_node_index;
            list->prev[new_node_index] = curr_node;
        }
    }
    list->space_used = list->space_used + 1;

    return PB_LLIST_ITERATOR_SUCCESS;
}


pb_llist_iterator_retval_t pb_llist_iterator_move_forward(pb_llist_iterator_t* iterator)
{
    iterator->current_index = iterator->list->next[iterator->current_index];
    if (iterator->current_index == 0)
    {
        return PB_LLIST_ITERATOR_END_OF_LIST;
    }
    return PB_LLIST_ITERATOR_SUCCESS;
}


pb_llist_iterator_retval_t pb_llist_iterator_move_backward(pb_llist_iterator_t* iterator)
{
    iterator->current_index = iterator->list->prev[iterator->current_index];
    if (iterator->current_index == 0)
    {
        return PB_LLIST_ITERATOR_START_OF_LIST;
    }
    return PB_LLIST_ITERATOR_SUCCESS;
}


pb_llist_iterator_retval_t pb_llist_iterator_reset2head(pb_llist_iterator_t* iterator)
{
    iterator->current_index = iterator->list->HEAD;
    if (iterator->current_index == 0)
    {
        return PB_LLIST_ITERATOR_LIST_IS_EMPTY;
    }
    return PB_LLIST_ITERATOR_SUCCESS;
}


pb_llist_iterator_retval_t pb_llist_iterator_reset2tail(pb_llist_iterator_t* iterator)
{
    iterator->current_index = iterator->list->TAIL;
    if (iterator->current_index == 0)
    {
        return PB_LLIST_ITERATOR_LIST_IS_EMPTY;
    }
    return PB_LLIST_ITERATOR_SUCCESS;
}
