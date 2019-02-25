#ifndef PB_LLIST_H__
#define PB_LLIST_H__

#include <stdint.h>

typedef enum
{
    PB_LLIST_SUCCESS,
    PB_LLIST_FAILURE,
    PB_LLIST_INSUFFICIENT_MEMORY,
    PB_LLIST_LIST_IS_EMPTY,
    PB_LLIST_INDEX_NOT_IN_LIST,
} pb_llist_retval_t;


typedef struct
{
    uint16_t* next;
    uint16_t* prev;
    uint8_t*  content;

    uint16_t  space_used;
    uint16_t  max_nodes;
    uint16_t  content_size;
    uint16_t  space_search_index;
} pb_llist_t;


#define PB_LLIST_DEF(name_, max_nodes_, size_node_content_)                    \
    uint16_t (name_ ## _next_node)[(max_nodes_)+1] = {0};                      \
    uint16_t (name_ ## _prev_node)[(max_nodes_)+1] = {0};                      \
    uint8_t  (name_ ## _content)[((max_nodes_)+1)*(size_node_content_)] = {0}; \
    pb_llist_t (name_) = {                                                     \
        (name_ ## _next_node), (name_ ## _prev_node), (name_ ## _content),     \
        0, (max_nodes_), (size_node_content_), 1                               \
    }                                                                          \


pb_llist_retval_t pb_llist_init(pb_llist_t* list);
pb_llist_retval_t pb_llist_reset(pb_llist_t* list);

uint16_t pb_llist_get_space_used(pb_llist_t* list);
uint16_t pb_llist_get_capacity(pb_llist_t* list);
uint16_t pb_llist_get_content_size(pb_llist_t* list);

pb_llist_retval_t pb_llist_head_add(pb_llist_t* list, uint8_t* content, uint16_t content_length, uint16_t* insert_index);
pb_llist_retval_t pb_llist_tail_add(pb_llist_t* list, uint8_t* content, uint16_t content_length, uint16_t* insert_index);
pb_llist_retval_t pb_llist_head_remove(pb_llist_t* list, uint8_t* content, uint16_t content_length);
pb_llist_retval_t pb_llist_tail_remove(pb_llist_t* list, uint8_t* content, uint16_t content_length);

pb_llist_retval_t pb_llist_index_remove(pb_llist_t* list, uint16_t remove_index, uint8_t* content, uint16_t content_length);
pb_llist_retval_t pb_llist_content_get(pb_llist_t* list, uint16_t element_index, uint8_t* content, uint16_t content_length);
//uint8_t* pb_llist_get_content(pb_llist_t* list, uint16_t index);
//pb_llist_retval_t pb_llist_set_content(pb_llist_t* list, uint16_t index, uint8_t* content);



#endif /* PB_LLIST_H__ */
