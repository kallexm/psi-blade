#ifndef PB_TIMER_H__
#define PB_TIMER_H__

#include <stdint.h>
#include <stdbool.h>

#define PB_TIMER_MAX_NUM_TIMERS 10

typedef void (*pb_on_timeout_cb)(void* context);

typedef enum
{
    PB_TIMER_SUCCESS,
    PB_TIMER_FAILURE,
    PB_TIMER_STOP_TIMER_FIRST,
} pb_timer_retval_t;


typedef enum
{
    PB_TIMER_ONESHOT,
    PB_TIMER_PERIODIC,
} pb_timer_type_t;


typedef struct
{
    pb_timer_type_t  type;
    pb_on_timeout_cb callback;
    void*            cb_context;
    uint32_t         expire_time;
    uint32_t         periodic_time;
    bool             active;
} pb_timer_t;



pb_timer_retval_t pb_timer_init();

pb_timer_retval_t pb_timer_create(uint16_t* timer_index, pb_timer_type_t type, pb_on_timeout_cb cb);
pb_timer_retval_t pb_timer_destroy(uint16_t* timer_index);

pb_timer_retval_t pb_timer_start(uint16_t* timer_index, uint32_t time, void* cb_context);
pb_timer_retval_t pb_timer_stop(uint16_t* timer_index);

#endif // PB_TIMER_H__
