#ifndef PB_OBERSERVER_H__
#define PB_OBERSERVER_H__

#include <stdint.h>

typedef enum
{
    PB_OBSERVER_SUCCESS,
    PB_OBSERVER_FAILURE,
} pb_observer_retval_t;

typedef void (*pb_observer_cb_t)(void* context);

typedef struct
{
    pb_observer_cb_t* callbacks;
    void**            context;
    uint8_t           subscribers_max_num;
} pb_observer_t;


#define PB_OBSERVER_DEF(observer_name_, subscribers_max_num_)                \
    pb_observer_cb_t (observer_name_ ## _callbacks)[(subscribers_max_num_)]; \
    void*            (observer_name_ ## _contexts)[(subscribers_max_num_)];  \
    pb_observer_t (observer_name_) = {                                       \
        (observer_name_ ## _callbacks),                                      \
        (observer_name_ ## _contexts),                                       \
        (subscribers_max_num_)                                               \
    }                                                                        \
    /* pb_observer_t* (observer_name_) = &(observer_name_ ## _data)             \ */


pb_observer_retval_t pb_observer_init(pb_observer_t* observer);

pb_observer_retval_t pb_observer_notify(pb_observer_t* observer);

pb_observer_retval_t pb_observer_subscribe(pb_observer_t* observer, pb_observer_cb_t cb, void* context);
pb_observer_retval_t pb_observer_unsubscribe(pb_observer_t* observer, pb_observer_cb_t cb);

#endif /* PB_OBERSERVER_H__ */
