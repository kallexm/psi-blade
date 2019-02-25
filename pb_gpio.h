#ifndef PB_GPIO_H__
#define PB_GPIO_H__

#include <stdint.h>

typedef enum
{
    PB_GPIO_SUCCESS,
    PB_GPIO_FAILURE,
} pb_gpio_retval_t;


typedef enum
{
    PERIPH_FUNC_A        = 0x0,
    PERIPH_FUNC_B        = 0x1,
    PERIPH_FUNC_C        = 0x2,
    PERIPH_FUNC_D        = 0x3,
    PERIPH_FUNC_E        = 0x4,
    PERIPH_FUNC_F        = 0x5,
    PERIPH_FUNC_G        = 0x6,
    PERIPH_FUNC_H        = 0x7,
    PERIPH_FUNC_I        = 0x8,
    PERIPH_FUNC_DISABLE  = 0x9,
} pb_gpio_periph_func_t;


typedef enum
{
    DIRECTION_OUT,
    DIRECTION_IN,
} pb_gpio_dir_t;


typedef enum
{
    VALUE_LOW,
    VALUE_HIGH,
    VALUE_TOGGLE,
} pb_gpio_value_t;


typedef enum
{
    DRIVE_STRENGTH_NORMAL,
    DRIVE_STRENGTH_STRONG,
} pb_gpio_drvstr_t;


pb_gpio_retval_t pb_gpio_pin_periph_func_set(uint8_t pin, pb_gpio_periph_func_t peripheral_function);


pb_gpio_retval_t pb_gpio_pin_dir(uint8_t pin, pb_gpio_dir_t direction);


pb_gpio_retval_t pb_gpio_pin_write(uint8_t pin, pb_gpio_value_t value);


uint8_t       pb_gpio_pin_read(uint8_t pin);


pb_gpio_retval_t pb_gpio_pin_strength(uint8_t pin, pb_gpio_drvstr_t drive_strength);


pb_gpio_retval_t pb_gpio_pin_pullup(uint8_t pin, bool pullup_value);


#endif // PB_GPIO_H__