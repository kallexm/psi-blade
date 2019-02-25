#ifndef PB_BUTTON_H__
#define PB_BUTTON_H__

#include "pb_observer.h"
#include "pb_circular_buffer.h"

#include <stdint.h>

#define PB_BUTTON_MAX_BUTTONS 4

typedef enum
{
    PB_BUTTON_SUCCESS,
    PB_BUTTON_FAILURE,
} pb_button_retval_t;

typedef enum
{
    PB_BUTTON_TRIGGER_FALLING_EDGE,
    PB_BUTTON_TRIGGER_RISING_EDGE,
} pb_button_trigger_t;

typedef void (*pb_button_on_evt_cb_t)(void* context);


typedef struct
{
    uint8_t                button_pin;
    pb_button_trigger_t    trigger_type;
    pb_observer_t*         p_observers;
    pb_crclrbuf_t*         p_queue;
} pb_button_t;


#define PB_BUTTON_DEF(name_, num_observers_, size_notification_queue_)      \
    PB_OBSERVER_DEF(name_ ## _observer, num_observers_);                    \
    PB_CRCLRBUF_DEF(name_ ## _queue, size_notification_queue_, 1);          \
    pb_button_t (name_) = {0, 0, &(name_ ## _observer), &(name_ ## _queue)} \


pb_button_retval_t pb_button_init(pb_button_t* bttn, uint8_t bttn_pin, pb_button_trigger_t trigger_type);

pb_button_retval_t pb_button_subscribe(pb_button_t* bttn, pb_button_on_evt_cb_t , void* context);
pb_button_retval_t pb_button_unsubscribe(pb_button_t* bttn, pb_button_on_evt_cb_t cb);

uint32_t           pb_button_process(pb_button_t* bttn, uint32_t num_evt_to_process);
uint32_t           pb_button_process_all_buttons(uint32_t num_evt_to_process);


#endif /* PB_BUTTON_H__ */
