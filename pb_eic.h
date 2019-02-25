#ifndef PB_EIC_H__
#define PB_EIC_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    PB_EIC_SUCCESS,
    PB_EIC_FAILURE,
    PB_EIC_CHANNEL_ALLREADY_ENABLED,
} pb_eic_retval_t;

typedef enum
{
    PB_EIC_NONE = 0x0,
    PB_EIC_RISE = 0x1,
    PB_EIC_FALL = 0x2,
    PB_EIC_BOTH = 0x3,
    PB_EIC_HIGH = 0x4,
    PB_EIC_LOW  = 0x5,
} pb_eic_sense_t;

typedef struct
{
    pb_eic_sense_t  trigger_type;
    bool            mjr_filt_enable;
    bool            wake_from_sleep;
} pb_eic_config_t;

typedef void (*pb_eic_cb_t)(uint8_t pin, void* context);



pb_eic_retval_t pb_eic_enable();

pb_eic_retval_t pb_eic_disable();

pb_eic_retval_t pb_eic_pin_interrupt_enable(uint8_t pin, pb_eic_config_t ext_int_config, pb_eic_cb_t callback, void* cb_context);

pb_eic_retval_t pb_eic_pin_interrupt_disable(uint8_t pin);

#endif /* PB_EIC_H__ */