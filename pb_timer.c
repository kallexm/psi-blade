#include "pb_timer.h"

#include "pb_llist.h"
#include "pb_llist_iterator.h"

#include "samd21.h"
#include "core_cm0plus.h"
#include "rtc.h"
#include "gclk.h"

#include <stdlib.h>

#define GCLK_GENDIV_DIV_100HZ_RTC 41
#define RTC_PRESCALER_100HZ_RTC RTC_MODE0_CTRL_PRESCALER_DIV32_Val
// Have to compensate scheduling by an amount so that deadlines are more exact. Might not be super important.


// rtc_timer. Could be something like a linked list of timers where the one in
// the front is the closest one in time. Then sett the timer/counter compare
// match for rtc timer to the closest point in time. When it goes off the first timer
// in the linked list should have it's callback function called. Then set the compare
// match for the next one in the list. If a new timer is startet then it needs to be
// added at the appropriate place in the list. If in front, the compare match value
// will have to be changed.

// Can add parameter for starting timer for possibility of giving data to the callback
// function as a parameter.

PB_LLIST_DEF(m_expire_list, PB_TIMER_MAX_NUM_TIMERS, sizeof(pb_timer_t));
PB_LLIST_ITERATOR_DEF(m_expire_iterator, m_expire_list);


pb_timer_retval_t pb_timer_init()
{
    /* Peripheral clock (ID) connect to clock generator and enable the clock signal */
    GCLK->CLKCTRL.bit.ID    = GCLK_CLKCTRL_ID_RTC_Val;
    GCLK->CLKCTRL.bit.GEN   = 3;
    GCLK->CLKCTRL.bit.CLKEN = 1;

    /* Clock generator (ID) connect to clock source (SRC), enable generator, output enable for generator, set pre-division factor */
    GCLK->GENCTRL.bit.ID     = 3;
    GCLK->GENCTRL.bit.SRC    = GCLK_GENCTRL_SRC_OSC32K_Val;
    // GCLK->GENCTRL.bit.DIVSEL = GCLK_GENCTRL_DIVSEL;
    GCLK->GENDIV.bit.ID      = 3;
    GCLK->GENDIV.bit.DIV     = GCLK_GENDIV_DIV_100HZ_RTC;
    GCLK->GENCTRL.bit.GENEN  = 1;

    /* Configure RTC clock, set mode, set clear on match, set rtc clock division factor */
    RTC->MODE0.CTRL.bit.MODE      = RTC_MODE0_CTRL_MODE_COUNT32;
    // RTC->MODE0.CTRL.bit.MATCHCLR  = RTC_MODE0_CTRL_MATCHCLR;
    RTC->MODE0.CTRL.bit.PRESCALER = RTC_PRESCALER_100HZ_RTC;
    
    RTC->MODE0.INTENSET.bit.CMP0 = RTC_MODE0_INTENSET_CMP0;
    NVIC_EnableIRQ(RTC_IRQn);

    pb_llist_retval_t llist_err = pb_llist_init(&m_expire_list);
    if (llist_err != PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    pb_llist_iterator_retval_t iter_err = pb_llist_iterator_init(&m_expire_iterator);
    if (iter_err != PB_LLIST_ITERATOR_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    RTC->MODE0.CTRL.bit.ENABLE = 1;
}


pb_timer_retval_t pb_timer_create(uint16_t* timer_index, pb_timer_type_t type, pb_on_timeout_cb cb)
{
    pb_timer_t new_timer;
    new_timer.type        = type;
    new_timer.callback    = cb;
    new_timer.cb_context  = NULL;
    new_timer.expire_time = 0;
    new_timer.active      = false;

    uint16_t new_timer_index = 0;
    pb_llist_retval_t llist_err = pb_llist_tail_add(&m_expire_list, (uint8_t*)&new_timer, sizeof(new_timer), &new_timer_index);
    if (llist_err != PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    *timer_index = new_timer_index;
    return PB_TIMER_SUCCESS;
}


pb_timer_retval_t pb_timer_destroy(uint16_t* timer_index)
{
    pb_timer_t timer_to_destroy = {0};
    pb_llist_retval_t llist_err1 = pb_llist_content_get(&m_expire_list, *timer_index, (uint8_t*)&timer_to_destroy, sizeof(timer_to_destroy));
    if (llist_err1 != PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    if (timer_to_destroy.active == true)
    {
        return PB_TIMER_STOP_TIMER_FIRST;
    }

    pb_llist_retval_t llist_err2 = pb_llist_index_remove(&m_expire_list, *timer_index, NULL, 0);
    if (llist_err2 != PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    return PB_TIMER_SUCCESS;
}


pb_timer_retval_t pb_timer_start(uint16_t* timer_index, uint32_t time_in_10ms, void* cb_context)
{
    pb_timer_t timer_to_start = {0};
    pb_llist_retval_t llist_err1 = pb_llist_content_get(&m_expire_list, *timer_index, (uint8_t*)&timer_to_start, sizeof(timer_to_start));
    if (llist_err1 != PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    if (timer_to_start.active == true)
    {
        return PB_TIMER_SUCCESS;
    }

    pb_llist_retval_t llist_err2 = pb_llist_index_remove(&m_expire_list, *timer_index, NULL, 0);
    if (llist_err2 != PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    timer_to_start.active        = true;
    timer_to_start.cb_context    = cb_context;
    timer_to_start.expire_time   = RTC->MODE0.COUNT.bit.COUNT + time_in_10ms;
    timer_to_start.periodic_time = time_in_10ms;

    pb_llist_iterator_retval_t iter_err1 = pb_llist_iterator_reset2head(&m_expire_iterator);
    if (iter_err1 == PB_LLIST_ITERATOR_LIST_IS_EMPTY)
    {
        pb_llist_retval_t llist_err3 = pb_llist_head_add(&m_expire_list, (uint8_t*)&timer_to_start, sizeof(timer_to_start), timer_index);
        RTC->MODE0.COMP[0].bit.COMP = timer_to_start.expire_time;
        return PB_TIMER_SUCCESS;
    }
    else if (iter_err1 == PB_LLIST_ITERATOR_SUCCESS)
    {
        uint16_t   num_in_list = 0;
        pb_timer_t curr_timer  = {0};
        while (true)
        {           
            pb_llist_iterator_retval_t iter_err2 = pb_llist_iterator_content_get(&m_expire_iterator, (uint8_t*)&curr_timer, sizeof(curr_timer));
            if (iter_err2 != PB_LLIST_ITERATOR_SUCCESS)
            {
                timer_to_start.active = false;
                pb_llist_tail_add(&m_expire_list, (uint8_t*)&timer_to_start, sizeof(timer_to_start), timer_index);
                return PB_TIMER_FAILURE;
            }

            if (curr_timer.active == false || timer_to_start.expire_time <= curr_timer.expire_time)
            {
                pb_llist_iterator_retval_t iter_err3 = pb_llist_iterator_insert_before(&m_expire_iterator, (uint8_t*)&timer_to_start, sizeof(timer_to_start), timer_index);
                if (iter_err3 != PB_LLIST_ITERATOR_SUCCESS)
                {
                    timer_to_start.active = false;
                    pb_llist_tail_add(&m_expire_list, (uint8_t*)&timer_to_start, sizeof(timer_to_start), timer_index);
                    return PB_TIMER_FAILURE;
                }

                if (num_in_list == 0)
                {
                    RTC->MODE0.COMP[0].bit.COMP = timer_to_start.expire_time;
                }
                return PB_TIMER_SUCCESS;
            }

            pb_llist_iterator_retval_t iter_err4 = pb_llist_iterator_move_forward(&m_expire_iterator);
            if (iter_err4 == PB_LLIST_ITERATOR_END_OF_LIST)
            {
                pb_llist_retval_t llist_err4 = pb_llist_tail_add(&m_expire_list, (uint8_t*)&timer_to_start, sizeof(timer_to_start), timer_index);
                if (llist_err4 != PB_LLIST_SUCCESS)
                {
                    return PB_TIMER_FAILURE;
                }
                return PB_TIMER_SUCCESS;
            }
            num_in_list = num_in_list + 1;
        }
    }
    else
    {
        return PB_TIMER_FAILURE;
    }
}


pb_timer_retval_t pb_timer_stop(uint16_t* timer_index)
{   
    pb_timer_t timer_to_stop = {0};
    pb_llist_retval_t llist_err1 = pb_llist_content_get(&m_expire_list, *timer_index, (uint8_t*)&timer_to_stop, sizeof(timer_to_stop));
    if (llist_err1 != PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    if (timer_to_stop.active == false)
    {
        return PB_TIMER_SUCCESS;
    }

    pb_llist_retval_t llist_err2 = pb_llist_index_remove(&m_expire_list, *timer_index, (uint8_t*)&timer_to_stop, sizeof(timer_to_stop));
    if (llist_err2 == PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    timer_to_stop.active = false;

    pb_llist_retval_t llist_err3 = pb_llist_tail_add(&m_expire_list, (uint8_t*)&timer_to_stop, sizeof(timer_to_stop), timer_index);
    if (llist_err3 == PB_LLIST_SUCCESS)
    {
        return PB_TIMER_FAILURE;
    }

    return PB_TIMER_SUCCESS;
}


void RTC_Handler()
{   
    /* Remove the expired timer at the head of the timer list */
    pb_timer_t expired_timer = {0};
    pb_llist_retval_t llist_err1 = pb_llist_head_remove(&m_expire_list, (uint8_t*)&expired_timer, sizeof(expired_timer));

    /* Set rtc comparison value to that of the next timer in the list */
    pb_timer_t* p_next_timer = (pb_timer_t*)&(m_expire_list.content[1]);
    RTC->MODE0.COMP[0].bit.COMP = p_next_timer->expire_time;

    /* Add expired timer to end of timer list */
    expired_timer.active = false;
    uint16_t expired_timer_index = 0;
    pb_llist_retval_t llist_err = pb_llist_tail_add(&m_expire_list, (uint8_t*)&expired_timer, sizeof(expired_timer), &expired_timer_index);

    /* If the expired timer is periodic then start it again */
    if (expired_timer.type == PB_TIMER_PERIODIC)
    {
        pb_timer_start(&expired_timer_index, expired_timer.periodic_time, expired_timer.cb_context);
    }

    /* Call the callback function of the expired timer with context as argument */
    expired_timer.callback(expired_timer.cb_context);

    return;
}