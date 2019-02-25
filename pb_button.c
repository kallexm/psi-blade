#include "pb_button.h"

#include "pb_eic.h"
#include "pb_gpio.h"
#include "pb_observer.h"
#include "pb_circular_buffer.h"

#include <stdbool.h>
#include <stdlib.h>


/*=====================================*/
#include <samd21.h>
void delay_led(int n)
{
    int i;

    for (;n >0; n--)
    {
        for (i=0;i<100;i++)
            __asm("nop");
    }
}

void debug_led(void)
{
    for (int i = 0; i < 10; i++)
    {
        REG_PORT_OUT1 &= ~(1<<2);
        delay_led(2);
        REG_PORT_OUT1 |= (1<<2);
        delay_led(2);
        REG_PORT_OUT1 &= ~(1<<2);
        delay_led(2);
        REG_PORT_OUT1 |= (1<<2);
        delay_led(2);
    }

    return;
}

/*=====================================*/


bool         m_button_module_initialized    = false;
uint8_t      m_num_bttns                    = 0;
pb_button_t* m_bttns[PB_BUTTON_MAX_BUTTONS] = {0};



pb_button_retval_t pb_button_module_init()
{
    pb_eic_retval_t eic_err = pb_eic_enable();
    if (eic_err != PB_EIC_SUCCESS)
    {
        return PB_BUTTON_FAILURE;
    }
    return PB_BUTTON_SUCCESS;
}


void button_triggered(uint8_t pin, void* context)
{
    pb_button_t* bttn = (pb_button_t*)context;

    // A random value to be pushed to the queue. The existance of the element is the important thing.
    uint8_t sentinel = 42;

    pb_crclrbuf_retval_t queue_err = pb_crclrbuf_enqueue(bttn->p_queue, &sentinel);
    if (queue_err != PB_CRCLRBUF_SUCCESS)
    {
        // Indicate enque error in some way. Can not return because of return type void.
    }

    return;
}


pb_button_retval_t pb_button_init(pb_button_t* bttn, uint8_t bttn_pin, pb_button_trigger_t trigger_type)
{
    if (m_button_module_initialized == false)
    {
        // This is executed ones
        pb_button_retval_t bttn_err = pb_button_module_init();
        if (bttn_err != PB_BUTTON_SUCCESS)
        {
            return bttn_err;
        }
        m_button_module_initialized = true;
    }

    bool bttn_allready_registered = false;
    for (int i = 0; i < PB_BUTTON_MAX_BUTTONS; i++)
    {
        if (bttn == m_bttns[i])
        {
            bttn_allready_registered = true;
            break;
        }
    }

    if (bttn_allready_registered == false)
    {
        if (m_num_bttns >= PB_BUTTON_MAX_BUTTONS)
        {
            return PB_BUTTON_FAILURE;
        }
        else
        {
            for (int i = 0; i < PB_BUTTON_MAX_BUTTONS; i++)
            {
                if (m_bttns[i] == 0)
                {
                    m_bttns[i] = bttn;
                }
            }
            m_num_bttns += 1;
        }
    }
    
    bttn->button_pin   = bttn_pin;
    bttn->trigger_type = trigger_type;

    pb_observer_retval_t obs_err = pb_observer_init(bttn->p_observers);
    if (obs_err != PB_OBSERVER_SUCCESS)
    {
        return PB_BUTTON_FAILURE;
    }

    pb_gpio_retval_t gpio_err1 = pb_gpio_pin_dir(bttn->button_pin, DIRECTION_IN);
    if (gpio_err1 != PB_GPIO_SUCCESS)
    {
        return PB_BUTTON_FAILURE;
    }

    pb_gpio_retval_t gpio_err2 = pb_gpio_pin_periph_func_set(bttn->button_pin, PERIPH_FUNC_A);
    if (gpio_err2 != PB_GPIO_SUCCESS)
    {
        return PB_BUTTON_FAILURE;
    }

    pb_eic_sense_t tgr_type;
    switch (bttn->trigger_type)
    {
        case PB_BUTTON_TRIGGER_FALLING_EDGE:
            tgr_type = PB_EIC_FALL;
            break;

        case PB_BUTTON_TRIGGER_RISING_EDGE:
            tgr_type = PB_EIC_RISE;
            break;

        default:
            return PB_BUTTON_FAILURE;
    }
    pb_eic_config_t eic_cfg = {tgr_type, true, true};

    pb_eic_retval_t eic_err = pb_eic_pin_interrupt_enable(bttn->button_pin, eic_cfg, (pb_eic_cb_t)button_triggered, (void*)bttn);
    if (eic_err != PB_EIC_SUCCESS)
    {
        return PB_BUTTON_FAILURE;
    }

    return PB_BUTTON_SUCCESS;
}


pb_button_retval_t pb_button_subscribe(pb_button_t* bttn, pb_button_on_evt_cb_t cb, void* context)
{
    pb_observer_retval_t obs_err = pb_observer_subscribe(bttn->p_observers, (pb_observer_cb_t)cb, context);
    if (obs_err != PB_OBSERVER_SUCCESS)
    {
        return PB_BUTTON_FAILURE;
    }
    return PB_BUTTON_SUCCESS;
}


pb_button_retval_t pb_button_unsubscribe(pb_button_t* bttn, pb_button_on_evt_cb_t cb)
{
    pb_observer_retval_t obs_err = pb_observer_unsubscribe(bttn->p_observers, (pb_observer_cb_t)cb);
    if (obs_err != PB_OBSERVER_SUCCESS)
    {
        return PB_BUTTON_FAILURE;
    }
    return PB_BUTTON_SUCCESS;
}


uint32_t pb_button_process(pb_button_t* bttn, uint32_t num_evt_to_process)
{
    uint8_t  sentinel = 0;

    uint32_t count = 0;
    while (count < num_evt_to_process)
    {
        pb_crclrbuf_retval_t queue_err = pb_crclrbuf_dequeue(bttn->p_queue, &sentinel);
        if (queue_err == PB_CRCLRBUF_BUFFER_EMPTY)
        {   
            break;
        }
        count++;
        pb_observer_notify(bttn->p_observers);
    }

    uint16_t num_remaining = pb_crclrbuf_space_used(bttn->p_queue);
    return num_remaining;    
}


uint32_t pb_button_process_all_buttons(uint32_t num_evt_to_process)
{   
    uint8_t    sentinel             = 0;
    static int i                    = 0;
    uint32_t   to_process           = num_evt_to_process;
    int        times_empty_in_a_row = 0;

    while (to_process > 0 && times_empty_in_a_row < PB_BUTTON_MAX_BUTTONS)
    {
        if (m_bttns[i] != NULL)
        {
            pb_crclrbuf_retval_t queue_err = pb_crclrbuf_dequeue(m_bttns[i]->p_queue, &sentinel);
            if (queue_err == PB_CRCLRBUF_SUCCESS)
            {   
                to_process = to_process - 1;
                times_empty_in_a_row = 0;
            }
            else
            {
                times_empty_in_a_row++;
            }
        }

        if (i == PB_BUTTON_MAX_BUTTONS-1)
        {
            i = i + 1;
        }
        else
        {
            i = 0;
        }
    }

    return to_process;
}