#include "pb_llist.h"

#include <stdbool.h>
#include <string.h>

#define HEAD list->next[0]
#define TAIL list->prev[0]


pb_llist_retval_t pb_llist_init(pb_llist_t* list)
{
    return PB_LLIST_SUCCESS;
}


pb_llist_retval_t pb_llist_reset(pb_llist_t* list)
{
    for (int i = 0; i <= list->max_nodes; i++)
    {
        list->next[i] = 0;
        list->prev[i] = 0;
    }

    list->space_used         = 0;
    list->space_search_index = 1;

    return PB_LLIST_SUCCESS;
}


uint16_t pb_llist_get_space_used(pb_llist_t* list)
{
    return list->space_used;
}


uint16_t pb_llist_get_capacity(pb_llist_t* list)
{
    return list->max_nodes;
}


uint16_t pb_llist_get_content_size(pb_llist_t* list)
{
    return list->content_size;
}


pb_llist_retval_t pb_llist_head_add(pb_llist_t* list, uint8_t* content, uint16_t content_length, uint16_t* insert_index)
{
    if (list->space_used == list->max_nodes)
    {
        return PB_LLIST_INSUFFICIENT_MEMORY;
    }

    bool     found_space    = false;
    uint16_t new_elem_index = 0;
    uint16_t counter        = list->max_nodes;
    while (found_space == false && counter > 0)
    {
        if (list->next[list->space_search_index] == 0 &&
            list->prev[list->space_search_index] == 0)
        {   
            found_space    = true;
            new_elem_index = list->space_search_index;
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

    if (found_space == true)
    {
        uint16_t copy_length = (list->content_size < content_length ? \
                                list->content_size : content_length);
        memcpy(&(list->content[new_elem_index]), content, copy_length);
        *insert_index = new_elem_index;

        if (list->space_used == 0)
        {
            list->next[new_elem_index] = 0;
            list->prev[new_elem_index] = 0;
            TAIL = new_elem_index;
            HEAD = new_elem_index;
        }
        else
        {
            list->next[new_elem_index] = HEAD;
            list->prev[new_elem_index] = 0;
            list->prev[HEAD] = new_elem_index;
            HEAD = new_elem_index;
        }
        
        list->space_used++;

        return PB_LLIST_SUCCESS;
    }
    else
    {
        return PB_LLIST_FAILURE;
    }
}


pb_llist_retval_t pb_llist_tail_add(pb_llist_t* list, uint8_t* content, uint16_t content_length, uint16_t* insert_index)
{
    if (list->space_used == list->max_nodes)
    {
        return PB_LLIST_INSUFFICIENT_MEMORY;
    }

    bool found_space        = false;
    uint16_t new_elem_index = 0;
    uint16_t counter        = list->max_nodes;
    while (found_space == false && counter > 0)
    {
        if (list->next[list->space_search_index] == 0 &&
            list->prev[list->space_search_index] == 0)
        {   
            found_space    = true;
            new_elem_index = list->space_search_index;
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

    if (found_space == true)
    {
        uint16_t copy_length = (list->content_size < content_length ? \
                                list->content_size : content_length);
        memcpy(&(list->content[new_elem_index]), content, copy_length);
        *insert_index = new_elem_index;

        if (list->space_used == 0)
        {
            list->next[new_elem_index] = 0;
            list->prev[new_elem_index] = 0;
            HEAD = new_elem_index;
            TAIL = new_elem_index;
        }
        else
        {
            list->next[new_elem_index] = 0;
            list->prev[new_elem_index] = TAIL;
            list->next[TAIL] = new_elem_index;
            TAIL = new_elem_index;
        }
        
        list->space_used++;

        return PB_LLIST_SUCCESS;
    }
    else
    {
        return PB_LLIST_FAILURE;
    }
}


pb_llist_retval_t pb_llist_head_remove(pb_llist_t* list, uint8_t* content, uint16_t content_length)
{
    if (list->space_used == 0)
    {
        return PB_LLIST_LIST_IS_EMPTY;
    }

    uint16_t elem_to_remove_index = HEAD;

    uint16_t copy_length = (list->content_size < content_length ? \
                            list->content_size : content_length);
    memcpy(content, &(list->content[elem_to_remove_index]), copy_length);

    if (list->space_used == 1)
    {
        HEAD = 0;
        TAIL = 0;
        list->next[elem_to_remove_index] = 0;
        list->prev[elem_to_remove_index] = 0;
    }
    else
    {
        HEAD = list->next[elem_to_remove_index];
        list->prev[HEAD] = 0;
        list->next[elem_to_remove_index] = 0;
        list->prev[elem_to_remove_index] = 0;
    }

    list->space_used--;

    return PB_LLIST_SUCCESS;
}


pb_llist_retval_t pb_llist_tail_remove(pb_llist_t* list, uint8_t* content, uint16_t content_length)
{
    if (list->space_used == 0)
    {
        return PB_LLIST_LIST_IS_EMPTY;
    }

    uint16_t elem_to_remove_index = TAIL;

    uint16_t copy_length = (list->content_size < content_length ? \
                            list->content_size : content_length);
    memcpy(content, &(list->content[elem_to_remove_index]), copy_length);

    if (list->space_used == 1)
    {
        HEAD = 0;
        TAIL = 0;
        list->next[elem_to_remove_index] = 0;
        list->prev[elem_to_remove_index] = 0;
    }
    else
    {
        TAIL = list->prev[elem_to_remove_index];
        list->next[TAIL] = 0;
        list->next[elem_to_remove_index] = 0;
        list->prev[elem_to_remove_index] = 0;
    }

    list->space_used--;

    return PB_LLIST_SUCCESS;
}

pb_llist_retval_t pb_llist_index_remove(pb_llist_t* list, uint16_t remove_index, uint8_t* content, uint16_t content_length)
{
    if (list->space_used == 0)
    {
        return PB_LLIST_LIST_IS_EMPTY;
    }

    uint16_t elem_to_remove_index = remove_index;

    if (list->next[elem_to_remove_index] == 0 &&
        list->prev[elem_to_remove_index] == 0)
    {
        return PB_LLIST_INDEX_NOT_IN_LIST;
    }

    uint16_t copy_length = (list->content_size < content_length ? \
                            list->content_size : content_length);
    memcpy(content, &(list->content[elem_to_remove_index]), copy_length);

    if (list->space_used == 1)
    {
        HEAD = 0;
        TAIL = 0;
        list->next[elem_to_remove_index] = 0;
        list->prev[elem_to_remove_index] = 0;
    }
    else
    {
        list->next[list->prev[elem_to_remove_index]] = list->next[elem_to_remove_index];
        list->prev[list->next[elem_to_remove_index]] = list->prev[elem_to_remove_index];
        list->next[elem_to_remove_index] = 0;
        list->prev[elem_to_remove_index] = 0;
    }

    list->space_used--;

    return PB_LLIST_SUCCESS;
}

/*=============================================================================*/
/*  if (list->prev[removed_node_index] != 0)
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
*/
/*=============================================================================*/

pb_llist_retval_t pb_llist_content_get(pb_llist_t* list, uint16_t element_index, uint8_t* content, uint16_t content_length)
{
    uint16_t copy_length = (list->content_size < content_length ? \
                            list->content_size : content_length);
    memcpy(content, &(list->content[element_index]), copy_length);

    return PB_LLIST_SUCCESS;
}
