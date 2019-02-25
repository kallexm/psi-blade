#ifndef PB_LLIST_ITERATOR_H__
#define PB_LLIST_ITERATOR_H__

#include "pb_llist.h"

typedef enum
{
    PB_LLIST_ITERATOR_SUCCESS,
    PB_LLIST_ITERATOR_FAILURE,
    PB_LLIST_ITERATOR_INSUFFICIENT_MEMORY,
    PB_LLIST_ITERATOR_LIST_IS_EMPTY,
    PB_LLIST_ITERATOR_END_OF_LIST,
    PB_LLIST_ITERATOR_START_OF_LIST,
} pb_llist_iterator_retval_t;


typedef struct
{
    pb_llist_t*  list;
    uint16_t     current_index;
} pb_llist_iterator_t;


#define PB_LLIST_ITERATOR_DEF(name_, llist_) \
    pb_llist_iterator_t (name_) = {          \
        &(llist_), 0                         \
    }                                        \


pb_llist_iterator_retval_t pb_llist_iterator_init(pb_llist_iterator_t* iterator);

pb_llist_iterator_retval_t pb_llist_iterator_content_get(pb_llist_iterator_t* iterator, uint8_t* content, uint16_t content_length);
pb_llist_iterator_retval_t pb_llist_iterator_remove_current(pb_llist_iterator_t* iterator, uint8_t* content, uint16_t content_length);

pb_llist_iterator_retval_t pb_llist_iterator_insert_before(pb_llist_iterator_t* iterator, uint8_t* content, uint16_t content_length, uint16_t* insert_index);
pb_llist_iterator_retval_t pb_llist_iterator_insert_after(pb_llist_iterator_t* iterator, uint8_t* content, uint16_t content_length, uint16_t* insert_index);

pb_llist_iterator_retval_t pb_llist_iterator_move_forward(pb_llist_iterator_t* iterator);
pb_llist_iterator_retval_t pb_llist_iterator_move_backward(pb_llist_iterator_t* iterator);

pb_llist_iterator_retval_t pb_llist_iterator_reset2head(pb_llist_iterator_t* iterator);
pb_llist_iterator_retval_t pb_llist_iterator_reset2tail(pb_llist_iterator_t* iterator);

#endif /* PB_LLIST_ITERATOR_H__ */
