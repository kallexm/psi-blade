#include <stdint.h>
#include <stdbool.h>

#include "pb_gpio.h"

#include "samd21.h"
#include "core_cm0plus.h"
#include "port.h"

/*=====================================*/
#include <samd21.h>
void delay_led_gpio(int n)
{
    int i;

    for (;n >0; n--)
    {
        for (i=0;i<100;i++)
            __asm("nop");
    }
}

void debug_led_gpio(void)
{
    for (int i = 0; i < 10; i++)
    {
        REG_PORT_OUT1 &= ~(1<<2);
        delay_led_gpio(2);
        REG_PORT_OUT1 |= (1<<2);
        delay_led_gpio(2);
        REG_PORT_OUT1 &= ~(1<<2);
        delay_led_gpio(2);
        REG_PORT_OUT1 |= (1<<2);
        delay_led_gpio(2);
    }

    return;
}

/*=====================================*/

typedef enum
{
    PORT_A,
    PORT_B,
} port_id_t;

typedef struct
{
    port_id_t port;
    uint8_t   position;
} port_position_t;


static pb_gpio_retval_t pin2portbit(uint8_t pin, port_position_t* portbit)
{   
    switch (pin)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            portbit->port     = PORT_A;
            portbit->position = pin - 1;
            break;

        case 7:
        case 8:
            portbit->port     = PORT_B;
            portbit->position = pin + 1;
            break;

        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            portbit->port     = PORT_A;
            portbit->position = pin - 5;
            break;

        case 19:
        case 20:
            portbit->port     = PORT_B;
            portbit->position = pin - 9;
            break;

        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
            portbit->port     = PORT_A;
            portbit->position = pin - 9;
            break;

        case 37:
        case 38:
            portbit->port     = PORT_B;
            portbit->position = pin - 15;
            break;

        case 39:
            portbit->port     = PORT_A;
            portbit->position = pin - 12;
            break;

        case 41:
            portbit->port     = PORT_A;
            portbit->position = pin - 13;
            break;

        case 45:
        case 46:
            portbit->port     = PORT_A;
            portbit->position = pin - 15;
            break;

        case 47:
        case 48:
            portbit->port     = PORT_B;
            portbit->position = pin - 45;
            break;

        default:
            portbit->port     = PORT_A;
            portbit->position = pin;
            return PB_GPIO_FAILURE;
    }
    return PB_GPIO_SUCCESS;
}


pb_gpio_retval_t pb_gpio_pin_periph_func_set(uint8_t pin, pb_gpio_periph_func_t peripheral_function)
{
    port_position_t portbit;
    pb_gpio_retval_t retval = pin2portbit(pin, &portbit);
    if (retval != PB_GPIO_SUCCESS)
    {
        return retval;
    }

    // Calculate positon of relevant settings registers
    uint8_t port_group;
    if (portbit.port == PORT_A)
    {
        port_group = 0;
    }
    else if (portbit.port == PORT_B)
    {
        port_group = 1;
    }
    else
    {
        return PB_GPIO_FAILURE;
    }
    uint8_t pmux_num        = portbit.position >> 1;
    uint8_t pcfg_enable_num = portbit.position;

    // Enable peripheral pin multiplexing and choose peripheral for pin
    if (peripheral_function == PERIPH_FUNC_DISABLE)
    {
        PORT->Group[port_group].PINCFG[pcfg_enable_num].bit.PMUXEN = 0;
    }
    else
    {
        if (portbit.position & 0x01)
        {
            PORT->Group[port_group].PMUX[pmux_num].bit.PMUXO = peripheral_function;
        }
        else
        {
            PORT->Group[port_group].PMUX[pmux_num].bit.PMUXE = peripheral_function;
        }
        
        PORT->Group[port_group].PINCFG[pcfg_enable_num].bit.PMUXEN = 1;
    }

    return PB_GPIO_SUCCESS;
}


pb_gpio_retval_t pb_gpio_pin_dir(uint8_t pin, pb_gpio_dir_t direction)
{
    port_position_t portbit;
    pb_gpio_retval_t retval = pin2portbit(pin, &portbit);
    if (retval != PB_GPIO_SUCCESS)
    {
        return retval;
    }

    uint8_t port_group;
    if (portbit.port == PORT_A)
    {
        port_group = 0;
    }
    else if (portbit.port == PORT_B)
    {
        port_group = 1;
    }
    else
    {
        return PB_GPIO_FAILURE;
    }
    uint8_t pcfg_inen_num = portbit.position;


    

    switch (direction)
    {
        case DIRECTION_IN:
            PORT->Group[port_group].DIRCLR.reg = (1 << portbit.position);
            PORT->Group[port_group].PINCFG[pcfg_inen_num].bit.INEN = 1;
            return PB_GPIO_SUCCESS;

        case DIRECTION_OUT:
            PORT->Group[port_group].DIRSET.reg = (1 << portbit.position);
            return PB_GPIO_SUCCESS;

        default:
            return PB_GPIO_FAILURE;
    }
}


pb_gpio_retval_t pb_gpio_pin_write(uint8_t pin, pb_gpio_value_t value)
{
    port_position_t portbit;
    pb_gpio_retval_t retval = pin2portbit(pin, &portbit);
    if (retval != PB_GPIO_SUCCESS)
    {
        return retval;
    }

    uint8_t port_group;
    if (portbit.port == PORT_A)
    {
        port_group = 0;
    }
    else if (portbit.port == PORT_B)
    {
        port_group = 1;
    }
    else
    {
        return PB_GPIO_FAILURE;
    }

    switch (value)
    {
        case VALUE_LOW:
            PORT->Group[port_group].OUTCLR.reg = (1 << portbit.position);
            return PB_GPIO_SUCCESS;

        case VALUE_HIGH:
            PORT->Group[port_group].OUTSET.reg = (1 << portbit.position);
            return PB_GPIO_SUCCESS;

        case VALUE_TOGGLE:
            PORT->Group[port_group].OUTTGL.reg = (1 << portbit.position);
            return PB_GPIO_SUCCESS;

        default:
            return PB_GPIO_FAILURE;
    }
}


uint8_t pb_gpio_pin_read(uint8_t pin)
{
    port_position_t portbit;
    pb_gpio_retval_t retval = pin2portbit(pin, &portbit);
    if (retval != PB_GPIO_SUCCESS)
    {
        return 0xFF;
    }
    
    uint8_t port_num;
    if (portbit.port == PORT_A)
    {
        port_num = 0;
    }
    else if (portbit.port == PORT_B)
    {
        port_num = 1;
    }
    else
    {
        return 0xFF;
    }
    
    uint32_t in_value = (PORT->Group[port_num].IN.reg & (1UL << portbit.position));
    if (in_value > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    
    return 0xFF;
}


pb_gpio_retval_t pb_gpio_pin_strength(uint8_t pin, pb_gpio_drvstr_t drive_strength)
{
    port_position_t portbit;
    pb_gpio_retval_t retval = pin2portbit(pin, &portbit);
    if (retval != PB_GPIO_SUCCESS)
    {
        return retval;
    }

    uint8_t port_group;
    if (portbit.port == PORT_A)
    {
        port_group = 0;
    }
    else if (portbit.port == PORT_B)
    {
        port_group = 1;
    }
    else
    {
        return PB_GPIO_FAILURE;
    }
    uint8_t pcfg_enable_num = portbit.position;

    switch (drive_strength)
    {
        case DRIVE_STRENGTH_NORMAL:
            PORT->Group[port_group].PINCFG[pcfg_enable_num].bit.DRVSTR = 0;
            return PB_GPIO_SUCCESS;

        case DRIVE_STRENGTH_STRONG:
            PORT->Group[port_group].PINCFG[pcfg_enable_num].bit.DRVSTR = 1;
            return PB_GPIO_SUCCESS;

        default:
            return PB_GPIO_FAILURE;
    }
}


pb_gpio_retval_t pb_gpio_pin_pullup(uint8_t pin, bool pullup_value)
{
    port_position_t portbit;
    pb_gpio_retval_t retval = pin2portbit(pin, &portbit);
    if (retval != PB_GPIO_SUCCESS)
    {
        return retval;
    }

    uint8_t port_group;
    if (portbit.port == PORT_A)
    {
        port_group = 0;
    }
    else if (portbit.port == PORT_B)
    {
        port_group = 1;
    }
    else
    {
        return PB_GPIO_FAILURE;
    }
    uint8_t pcfg_enable_num = portbit.position;

    PORT->Group[port_group].PINCFG[pcfg_enable_num].bit.PULLEN = (uint8_t)pullup_value;

    return PB_GPIO_SUCCESS;
}