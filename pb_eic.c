#include "pb_eic.h"

#include "samd21.h"
#include "eic.h"
#include "pb_critical_sections.h"

#include <string.h>

#define PB_EIC_SENSE_MASK    0b0111
#define PB_EIC_SENSE_OFFSET  0
#define PB_EIC_FILTER_MASK   0b0001
#define PB_EIC_FILTER_OFFSET 3
#define PB_EIC_WAKEUP_MASK   0x1

/*=====================================*/
#include <samd21.h>
void delay_led_eic(int n)
{
    int i;

    for (;n >0; n--)
    {
        for (i=0;i<100;i++)
            __asm("nop");
    }
}

void debug_led_eic(void)
{
    for (int i = 0; i < 10; i++)
    {
        REG_PORT_OUT1 &= ~(1<<2);
        delay_led_eic(2);
        REG_PORT_OUT1 |= (1<<2);
        delay_led_eic(2);
        REG_PORT_OUT1 &= ~(1<<2);
        delay_led_eic(2);
        REG_PORT_OUT1 |= (1<<2);
        delay_led_eic(2);
    }

    return;
}
/*=====================================*/

typedef struct
{
    bool            enabled;
    uint8_t         pin;
    pb_eic_config_t cfg;
    pb_eic_cb_t     app_int_handler;
    void*           context;
} pb_eic_instance_t;



pb_eic_instance_t m_eic[16];
bool              m_eic_initialized = false;
bool              m_enabled         = false;



static uint8_t pin2eicnum(uint8_t pin)
{
    uint8_t eic_num = 0xFF;
    switch (pin)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 7:
        case 8:
            eic_num = pin - 1;
            break;

        case 9:
        case 10:
        case 11:
        case 12:
        case 14:
        case 15:
        case 16:
            eic_num = pin - 5;
            break;

        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
            eic_num = pin - 9;
            break;

        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
            eic_num = pin - 25;
            break;

        case 33:
        case 34:
            eic_num = pin - 21;
            break;

        case 37:
        case 38:
            eic_num = pin - 31;
            break;

        case 39:
            eic_num = pin - 24;
            break;

        case 41:
            eic_num = pin - 33;
            break;

        case 45:
        case 46:
            eic_num = pin - 35;
            break;

        case 47:
        case 48:
            eic_num = pin - 45;
            break;

        default:
            break;
    }
    return eic_num;
}



static void init_pb_eic()
{
    for (int8_t i = 0; i < 16; i++)
    {
        m_eic[i].enabled         = false;
        m_eic[i].pin             = 0;
        memset(&m_eic[i].cfg, 0, sizeof(pb_eic_config_t));
        m_eic[i].app_int_handler = NULL;
        m_eic[i].context         = NULL;
    }

    return;
}



pb_eic_retval_t pb_eic_enable()
{   
    if (m_enabled == true)
    {
        return PB_EIC_SUCCESS;
    }

    // First time called -> initialize local data
    if (m_eic_initialized == false)
    {
        init_pb_eic();
        m_eic_initialized = true;
    }

    GCLK->GENDIV.bit.ID      = 4;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };
    GCLK->GENDIV.bit.DIV     = 16;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };

    GCLK->GENCTRL.bit.ID     = 4;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };
    GCLK->GENCTRL.bit.SRC    = GCLK_GENCTRL_SRC_OSC8M_Val; // No bit access here in example /* OSC8M internal 8MHz, XOSC external 32MHz */
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };
    GCLK->GENCTRL.bit.GENEN  = 1;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };

    GCLK->CLKCTRL.bit.ID    = GCLK_CLKCTRL_ID_EIC_Val; // No bit access here in example
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };
    GCLK->CLKCTRL.bit.GEN   = 4;    // Example does not use bit access here
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };
    GCLK->CLKCTRL.bit.CLKEN = 1;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };

    NVIC_EnableIRQ(EIC_IRQn);

    EIC->CTRL.bit.ENABLE = 1;
    while (EIC->STATUS.bit.SYNCBUSY)
    {
        // Wait/do nothing
    }

    return PB_EIC_SUCCESS;
}



pb_eic_retval_t pb_eic_disable()
{
    if (m_enabled == false)
    {
        return PB_EIC_SUCCESS;
    }

    EIC->CTRL.bit.ENABLE = 0;
    while (EIC->STATUS.bit.SYNCBUSY)
    {
        //Wait/do nothing
    }

    return PB_EIC_SUCCESS;
}



pb_eic_retval_t pb_eic_pin_interrupt_enable(uint8_t pin, pb_eic_config_t ext_int_config, pb_eic_cb_t callback, void* context)
{
    uint8_t eic_num = pin2eicnum(pin);
    if (eic_num == 0xFF)
    {
        return PB_EIC_FAILURE;
    }

    if (m_eic[eic_num].enabled == true)
    {
        return PB_EIC_CHANNEL_ALLREADY_ENABLED;
    }

    // Copy configurations to module internal memory
    m_eic[eic_num].enabled         = true;
    m_eic[eic_num].pin             = pin;
    m_eic[eic_num].cfg             = ext_int_config;
    m_eic[eic_num].app_int_handler = callback;
    m_eic[eic_num].context         = context;

    // Configure trigger type and majority filter
    uint32_t cfg_n      = (eic_num >> 3);
    uint32_t cfg_offset = (eic_num & 0b0111) << 2;
    uint32_t cfg_mask   = (0x0F << cfg_offset);
    uint32_t cfg_value_to_write =
        ((ext_int_config.trigger_type    & PB_EIC_SENSE_MASK ) << PB_EIC_SENSE_OFFSET) |
        ((ext_int_config.mjr_filt_enable & PB_EIC_FILTER_MASK) << PB_EIC_FILTER_OFFSET);
    uint32_t config_reg = EIC->CONFIG[cfg_n].reg;
    config_reg = (config_reg & ~cfg_mask) | ((cfg_value_to_write << cfg_offset) & cfg_mask);
    EIC->CONFIG[cfg_n].reg = config_reg;

    // Configure Wake-Up
    uint32_t wakeup_reg = EIC->WAKEUP.vec.WAKEUPEN;
    wakeup_reg = (wakeup_reg & ~(PB_EIC_WAKEUP_MASK << eic_num)) |
        ((ext_int_config.wake_from_sleep & PB_EIC_WAKEUP_MASK) << eic_num);
    EIC->WAKEUP.vec.WAKEUPEN = wakeup_reg;

    // Enable pin external interrupt
    EIC->INTENSET.reg = (1 << eic_num);

    return PB_EIC_SUCCESS;
}



pb_eic_retval_t pb_eic_pin_interrupt_disable(uint8_t pin)
{
    uint8_t eic_num = pin2eicnum(pin);
    if (eic_num == 0xFF)
    {
        return PB_EIC_FAILURE;
    }
    
    m_eic[eic_num].enabled   = false;
    EIC->INTENCLR.reg = (1 << eic_num);
    
    return PB_EIC_SUCCESS;
}



void EIC_Handler()
{   
    pb_critical_section_enter();

    // Get external interrupt flags
    uint16_t intflag = EIC->INTFLAG.vec.EXTINT;    

    // Call application handlers
    for (uint8_t i = 0; i < 16; i++)
    {   
        if (m_eic[i].enabled == true && (intflag >> i & 0x1) == 1)
        {
            pb_eic_cb_t cb = (pb_eic_cb_t)(m_eic[i].app_int_handler);
            uint8_t pin    = m_eic[i].pin;
            void* context  = m_eic[i].context;
            cb(pin, context);
        }
    }
    
    pb_critical_section_leave();
    
    // Clear external interrupt flags in EIC
    EIC->INTFLAG.reg = 0xFFFF;
    
    // Clear EIC interrupt in NVIC
    //NVIC_ClearPendingIRQ(EIC_IRQn);
}
