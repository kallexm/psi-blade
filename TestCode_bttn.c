#include <samd21.h>

#include "pb_button.h"
#include "pb_gpio.h"

#include <stdlib.h>

void delay(int n)
{
    int i;

    for (;n >0; n--)
    {
        for (i=0;i<100;i++)
            __asm("nop");
    }
}

void debug_(void)
{
    for (int i = 0; i < 10; i++)
    {
        REG_PORT_OUT1 &= ~(1<<2);
        delay(20);
        REG_PORT_OUT1 |= (1<<2);
        delay(5);
        REG_PORT_OUT1 &= ~(1<<2);
        delay(5);
        REG_PORT_OUT1 |= (1<<2);
        delay(20);
    }

    return;
}

void set_test_pin_high(void* context)
{
    //REG_PORT_OUT1 |= (1<<2);
    pb_gpio_pin_write(47, VALUE_HIGH);
    return;
}


void set_test_pin_low(void* context)
{
    //REG_PORT_OUT1 &= ~(1<<2);
    pb_gpio_pin_write(47, VALUE_LOW);
    return;
}


PB_BUTTON_DEF(bttn0, 1, 1);
PB_BUTTON_DEF(bttn1, 1, 1);

int main()
{   
    /*REG_PORT_DIR1 |= (1<<2); */
    pb_gpio_retval_t gpio_err1 = pb_gpio_pin_dir(47, DIRECTION_OUT);
    //pb_gpio_retval_t gpio_err2 = pb_gpio_pin_dir(41, DIRECTION_IN);

    pb_button_retval_t err1 = pb_button_init(&bttn0, 41, PB_BUTTON_TRIGGER_RISING_EDGE);
    if (err1 != PB_BUTTON_SUCCESS)
    {
        debug_();
    }

    pb_button_retval_t err2 = pb_button_init(&bttn1, 39, PB_BUTTON_TRIGGER_RISING_EDGE);
    if (err2 != PB_BUTTON_SUCCESS)
    {
        debug_();
    }

    pb_button_retval_t err3 = pb_button_subscribe(&bttn0, set_test_pin_high, NULL);
    if (err3 != PB_BUTTON_SUCCESS)
    {
        debug_();
    }

    pb_button_retval_t err4 = pb_button_subscribe(&bttn1, set_test_pin_low,  NULL);
    if (err4 != PB_BUTTON_SUCCESS)
    {
        debug_();
    }



    while (1)
    {
        pb_button_process(&bttn0, 1);
        pb_button_process(&bttn1, 1);

        /*if (pb_gpio_pin_read(41) > 0)
        {
            debug_();
        }
        else
        {
            if (true)
            {
                pb_gpio_retval_t gpio_err2 = pb_gpio_pin_write(47, VALUE_LOW);
                delay(10);
                gpio_err2 = pb_gpio_pin_write(47, VALUE_HIGH);
                delay(10);
            }
            else
            {
                REG_PORT_OUT1 &= ~(1<<2);
                delay(20);
                REG_PORT_OUT1 |= (1<<2);
                delay(10);
            }
        
        }*/

        pb_gpio_retval_t gpio_err2 = pb_gpio_pin_write(47, VALUE_TOGGLE);
        delay(10);
        gpio_err2 = pb_gpio_pin_write(47, VALUE_TOGGLE);
        delay(20);
    }
    return 0;
}