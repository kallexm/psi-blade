#include "pb_critical_sections.h"

#include <samd21.h>

#include <stdint.h>
#include <stdbool.h>



static volatile bool       m_interrupt_enabled        = true;
static volatile bool       m_prev_interrupt_state;
static volatile uint32_t   m_critical_section_counter = 0;



void pb_critical_section_enter(void)
{
    if (m_critical_section_counter == 0)
    {
        if (__get_PRIMASK() == 0) /* is the irq enabled? */
        {
            __ASM volatile ("cpsid i" : : : "memory");
            __DMB();
            m_interrupt_enabled = false;

            m_prev_interrupt_state = true;
        }
        else 
        {
            m_prev_interrupt_state = false;
        }
    }
    
    m_critical_section_counter += 1;

    return;
}


void pb_critical_section_leave(void)
{
    m_critical_section_counter -= 1;

    if ((m_critical_section_counter == 0) && (m_prev_interrupt_state == true))
    {
        m_interrupt_enabled = true;
        __DMB();
        __ASM volatile ("cpsie i" : : : "memory");
    }

    return;
}