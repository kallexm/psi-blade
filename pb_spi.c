#include "pb_spi.h"

#include "pb_gpio.h"

#include "samd21.h"

#define PB_SPI_GCLK_GENERATOR_NUM 5
#define PB_SPI_GCLK_FREQUENCY     8000000


void pb_spi_on_interrupt(void* context)
{
    pb_spi_t* spi = (pb_spi_t*)context;

    uint8_t interrupt_flags = spi->spi_instance->INTFLAG.reg;
    if (interrupt_flags & SERCOM_SPI_INTFLAG_DRE != 0)
    {
        // Data register empty interrupt
        uint8_t data_to_transmit = 0;
        pb_crclrbuf_retval_t queue_err1 = pb_crclrbuf_dequeue(spi->p_out_queue, &data_to_transmit);
        if (queue_err1 == PB_CRCLRBUF_SUCCESS)
        {
            spi->spi_instance->DATA.bit.DATA = data_to_transmit;
            spi->bytes_transmitted = spi->bytes_transmitted + 1;
        }
    }

    if (interrupt_flags & SERCOM_SPI_INTFLAG_TXC != 0)
    {
        // Transmit complete interrupt
        pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_HIGH);

        pb_spi_channel_t* channel = spi->transmitting_channel;

        spi->transmitting_channel->is_transmitting = false;
        spi->transmitting_channel = NULL;
        spi->is_transmitting = false;

        spi->on_transmit_complete(channel);

        /* Check for read requests */
        if (spi->is_transmitting == false)
        {   
            pb_spi_channel_t* receive_channel = NULL;
            for (int i = 0; i < PB_SPI_MAX_CHANNELS_PER_SPI_INSTANCE; i++)
            {
                if (spi->slave_read_request_pending[i] == true)
                {
                    receive_channel = spi->connected_channels[i];
                    spi->slave_read_request_pending = false;
                    break;
                }
            }

            if (receive_channel != NULL)
            {
                spi->is_transmitting = true;
                spi->transmitting_channel = receive_channel;
                receive_channel->is_transmitting = true;
                pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_LOW);
                spi->spi_instance->DATA.bit.DATA = 0xFF;
            }
        }
    }

    if (interrupt_flags & SERCOM_SPI_INTFLAG_RXC != 0)
    {
        // Receive complete interrupt
        uint16_t received_data = spi->spi_instance->DATA.bit.DATA;
        pb_crclrbuf_retval_t queue_err2 = pb_crclrbuf_enqueue(spi->p_in_queue, &received_data);
        if (queue_err2 != PB_CRCLRBUF_SUCCESS)
        {
            spi->bytes_overflowed = spi->bytes_overflowed + 1;// Buffer overflow, have to drop data. Maybe generate some form of error 
        }
        spi->bytes_received = spi->bytes_received + 1;
    }

    if (interrupt_flags & SERCOM_SPI_INTFLAG_SSL != 0)
    {
        // Slave select low interrupt
    }

    if (interrupt_flags & SERCOM_SPI_INTFLAG_ERROR != 0)
    {
        // Error interrupt
    }

    return;
}


pb_spi_retval_t enable_clock(Sercom* SERCOMn)
{
    if (GCLK->GENCTRL.bit.GENEN == 0)
    {
        GCLK->GENDIV.bit.ID      = PB_SPI_GCLK_GENERATOR_NUM;
        while (GCLK->STATUS.bit.SYNCBUSY)
        {
            // Synchronizing
        };
        GCLK->GENDIV.bit.DIV     = 1;
        while (GCLK->STATUS.bit.SYNCBUSY)
        {
            // Synchronizing
        };

        GCLK->GENCTRL.bit.ID     = PB_SPI_GCLK_GENERATOR_NUM;
        while (GCLK->STATUS.bit.SYNCBUSY)
        {
            // Synchronizing
        };
        GCLK->GENCTRL.bit.SRC    = GCLK_GENCTRL_SRC_OSC8M_Val; /* OSC8M internal 8MHz, XOSC external 32MHz */
        while (GCLK->STATUS.bit.SYNCBUSY)
        {
            // Synchronizing
        };
        GCLK->GENCTRL.bit.GENEN  = 1;
        while (GCLK->STATUS.bit.SYNCBUSY)
        {
            // Synchronizing
        };
    }
    
    uint32_t sercom_gclk_to_activate;
    switch (SERCOMn)
    {
        case SERCOM0:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM0_CORE_Val;
            break;

        case SERCOM1:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM1_CORE_Val;
            break;

        case SERCOM2:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM2_CORE_Val;
            break;

        case SERCOM3:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM3_CORE_Val;
            break;

        case SERCOM4:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM4_CORE_Val;
            break;

        case SERCOM5:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM5_CORE_Val;
            break;

        default:
            return PB_SPI_FAILURE;
    }

    GCLK->CLKCTRL.bit.ID    = sercom_gclk_to_activate;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };
    GCLK->CLKCTRL.bit.GEN   = PB_SPI_GCLK_GENERATOR_NUM;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };
    GCLK->CLKCTRL.bit.CLKEN = 1;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
        // Synchronizing
    };

    return PB_SPI_SUCCESS;
}


pb_spi_retval_t enable_interrupt(Sercom* SERCOMn)
{
    uint32_t sercom_interrupt_to_enable;
    switch (SERCOMn)
    {
        case SERCOM0:
            sercom_interrupt_to_enable = SERCOM0_IRQn;
            break;

        case SERCOM1:
            sercom_interrupt_to_enable = SERCOM1_IRQn;
            break;

        case SERCOM2:
            sercom_interrupt_to_enable = SERCOM2_IRQn;
            break;

        case SERCOM3:
            sercom_interrupt_to_enable = SERCOM3_IRQn;
            break;

        case SERCOM4:
            sercom_interrupt_to_enable = SERCOM4_IRQn;
            break;

        case SERCOM5:
            sercom_interrupt_to_enable = SERCOM5_IRQn;
            break;

        default:
            return PB_SPI_FAILURE;
    }

    NVIC_EnableIRQ(sercom_interrupt_to_enable);
    return PB_SPI_SUCCESS;
}   


pb_spi_retval_t pb_spi_transport_init(pb_spi_t* spi, Sercom* SERCOMn, pb_spi_config_t* config)
{
    if (spi == NULL || SERCOMn == NULL || config == NULL)
    {
        return PB_SPI_FAILURE;
    }

    spi->spi_instance = (SercomSpi*)&(SERCOMn->SPI);
    memcpy(&(spi->config), config, sizeof(*config));

    spi->bytes_transmitted = 0;
    spi->bytes_received    = 0;
    spi->bytes_overflowed  = 0;

    pb_spi_retval_t err1 = enable_clock(SERCOMn);
    if (err1 != PB_SPI_SUCCESS)
    {
        return err1;
    }

    // eller spi->spi_instance->CTRLA.bit.......
    spi->spi_instance->CTRLA.bit.MODE     = spi->config.mode;
    spi->spi_instance->CTRLA.bit.DORD     = spi->config.data_order;
    spi->spi_instance->CTRLA.bit.CPOL     = spi->config.clock_polarity;
    spi->spi_instance->CTRLA.bit.CPHA     = spi->config.clock_phase;
    spi->spi_instance->CTRLA.bit.RUNSTDBY = spi->config.run_in_standby;
    spi->spi_instance->CTRLA.bit.DOPO     = spi->config.data_out_pin;
    spi->spi_instance->CTRLA.bit.DIPO     = spi->config.data_in_pin;
    /*spi->spi_instance->CTRLB.bit.CHSIZE   = spi->config.character_size;*/
    spi->spi_instance->CTRLB.bit.CHSIZE   = 0;
    spi->spi_instance->CTRLB.bit.MSSEN    = spi->config.master_slave_select;
    spi->spi_instance->CTRLB.bit.SSDE     = spi->config.slave_selct_low_detect;

    spi->spi_instance.BAUD.reg = ((PB_SPI_GCLK_FREQUENCY)/(2*(spi->config.bit_rate_in_Hz))) - 1;

    // If input queue has size zero, disable input functionality
    if (spi->size_in_queue == 0)
    {
        /* Do not setup receive part */
        spi->spi_instance.CTRLB.bit.RXEN   = 0;
        spi->spi_instance.INTENSET.bit.RXC = 0;
    }
    else
    {
        spi->spi_instance.CTRLB.bit.RXEN   = 1;
        spi->spi_instance.INTENSET.bit.RXC = 1;
    }

    // If output queue has size zero, disable output functionality
    if (spi->size_out_queue == 0)
    {
        spi->spi_instance.INTENSET.bit.TXC = 0;
    }
    else
    {
        spi->spi_instance.INTENSET.bit.TXC = 1;
    }
    spi->spi_instance.INTENSET.bit.DRE = 1; // What to do with this?!

    pb_sercom_handlers_retval_t sc_hnl_err = pb_sercom_handlers_register_callback((Sercom*)(spi->spi_instance), pb_spi_on_interrupt, (void*)spi);

    pb_spi_retval_t err2 = enable_interrupt(SERCOMn);
    if (err2 != PB_SPI_SUCCESS)
    {
        return err2;
    }

    spi->spi_instance.INTENSET.bit.
    spi->spi_instance.INTENCLR.bit.
    spi->spi_instance.INTFLAG.bit.

    spi->spi_instance.DATA.reg

    spi->spi_instance.SYNCBUSY.bit.


    /*
        1. SPI module config 
            a. Set relevant pins to sercom function in PORT module (gpio)
            b. PINCFGn.PULLEN and PINCFGn.DRVSTR is still effective in SPI mode
            c. Receiver pin can be disabled with CTRLB.RXEN = 0
            d. Set BAUD register
            e. Set control mode CTRLA.MODE
            f. Set CTRLA.CPOL and CTRLA.CPHA
            g. Set CTRLA.FORM
            h. Set CTRLA.DIPO (what pad should be used for receiver)
            i. Set CTRLA.DOPO (what pads should be used for input, clock and SS)
            j. Set CTRLB.CHSIZE (character size)
            k. Set CTRLA.DORD (data direction)
            l. Set CTRLB.MSSEN
            m. Enable sercom spi with CTRL.ENABLE = 1
        2. Clock config
            a. Hook SERCOMn's general clock to a clock generator, connect
               the clock generator to a clock source. If the normal clock
               generator is used the setup of the clock generator is not necessary.

        3. Interrupt config
            a. Enable the correct SERCOM interrupt
            b. Set priority

        4. initialize queues
        Does not need initialization

        5. initialize observer
        6. set gpio pins as sercom

     */
    


    pb_gpio_retval_t gpio_err2 = pb_gpio_pin_periph_func_set(bttn->button_pin, (pb_gpio_periph_func_t)(spi->config.pins_multiplex));
    if (gpio_err2 != PB_GPIO_SUCCESS)
    {
        return PB_BUTTON_FAILURE;
    }

    spi->spi_instance.CTRLA.ENABLE = 1;
    while (spi->spi_instance.SYNCBUSY.bit.ENABLE)
    {
        // Synchronizing
    }

    return PB_SPI_SUCCESS;
}


void on_slave_read_request(uint8_t pin, void* context)
{
    pb_spi_channel_t* channel = (pb_spi_channel_t*)context;
    pb_spi_t*         spi     = channel->spi_transport;
    
    if (spi->is_transmitting == true)
    {
        for (int i = 0; i < PB_SPI_MAX_CHANNELS_PER_SPI_INSTANCE; i++)
        {
            if (spi->connected_channels[i] == channel)
            {
                spi->slave_read_request_pending[i] = true;
                break;
            }
        }
    }
    else
    {
        spi->is_transmitting     = true;
        channel->is_transmitting = true;

        spi->transmitting_channel = channel;

        pb_gpio_retval_t gpio_err = pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_LOW);
        if (gpio_err != PB_GPIO_SUCCESS)
        {
            return;
        }

        spi->spi_instance->DATA.bit.DATA = 0xFF;
    }

    return;
}


pb_spi_retval_t pb_spi_channel_init(pb_spi_channel_t* channel, pb_spi_t* spi, pb_spi_channel_config_t* config)
{
    if (spi->enabled == false)
    {
        return PB_SPI_FAILURE;
    }

    for (int i = 0; i < PB_SPI_MAX_CHANNELS_PER_SPI_INSTANCE; i++)
    {
        if (spi->connected_channels[i] == NULL)
        {
            spi->connected_channels[i]         = channel;
            spi->slave_read_request_pending[i] = false;
            channel->spi_transport             = spi;
            break;
        }
    }
    if (channel->spi_transport == NULL)
    {
        return PB_SPI_FAILURE;
    }

    pb_observer_retval_t obs_err = pb_observer_init(channel->p_observer);
    if (obs_err != PB_OBSERVER_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }

    /* Slave select pin config. Output direction and Set high */
    channel->ss_pin = config->slave_select_pin;
    pb_gpio_retval_t gpio_err = pb_gpio_pin_dir(channel->ss_pin, DIRECTION_OUT);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }
    gpio_err = pb_gpio_pin_write(channel->ss_pin, VALUE_HIGH);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }

    /* Slave request pin config. Enable external interrupt, set pin function to external interrupt,
       set external interrupt config, set callback function and enable external interrupt on pin. */
    channel->slave_request_pin->slave_read_request_interrupt_pin;
    if (channel->slave_request_pin != 0)
    {
        pb_eic_retval_t eic_err = pb_eic_enable();
        if (eic_err != PB_EIC_SUCCESS)
        {
            return PB_SPI_FAILURE;
        }

        gpio_err = pb_gpio_pin_periph_func_set(channel->slave_request_pin, PERIPH_FUNC_A);
        if (gpio_err != PB_GPIO_SUCCESS)
        {
            return PB_SPI_FAILURE;
        }

        pb_eic_config_t eic_config;
        eic_config.trigger_type = PB_EIC_RISE;
        eic_config.mjr_filt_enable = true;
        eic_conifg.wake_from_sleep = true;

        eic_err = pb_eic_pin_interrupt_enable(channel->slave_request_pin, eic_config, on_slave_read_request, (void*)channel);
        if (eic_err != PB_EIC_SUCCESS)
        {
            return PB_SPI_FAILURE;
        }
    } 

    return PB_SPI_SUCCESS;
}


pb_spi_retval_t pb_spi_queue_data(pb_spi_channel_t* channel, char char_to_send)
{
    if (channel->is_transmitting == true)
    {
        return PB_SPI_FAILURE;
    }

    pb_crclrbuf_retval_t queue_err = pb_crclrbuf_enqueue(channel->p_out_queue, &char_to_send);
    if (queue_err != PB_CRCLRBUF_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }

    return PB_SPI_SUCCESS;
}


pb_spi_retval_t pb_spi_send_data(pb_spi_channel_t* channel, pb_spi_transmit_complete_cb_t cb)
{
    pb_critical_section_enter();

    if (channel->is_transmitting == true)
    {
        return PB_SPI_SUCCESS;
    }
    if (channel->spi_transport->is_transmitting == true)
    {
        return PB_SPI_FAILURE;
    }

    pb_spi_t* spi = channel->spi_transport;

    channel->is_transmitting = true;
    spi->is_transmitting     = true;

    spi->transmitting_channel = channel;
    spi->on_transmit_complete = cb;

    pb_gpio_retval_t gpio_err = pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_LOW);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }

    spi->spi_instance->DATA.bit.DATA = 0xFF;

    pb_critical_section_leave();
}



pb_spi_retval_t pb_spi_subscribe(pb_spi_channel_t* channel, pb_spi_on_evt_cb_t cb, void* context)
{
    pb_observer_retval_t obs_err = pb_observer_subscribe(channel->p_observer, (pb_observer_cb_t)cb, context);
    if (obs_err != PB_OBSERVER_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }
    return PB_SPI_SUCCESS;
}


pb_spi_retval_t pb_spi_unsubscribe(pb_spi_channel_t* channel, pb_spi_on_evt_cb_t cb)
{
    pb_observer_retval_t obs_err = pb_observer_unsubscribe(channel->p_observer, (pb_observer_cb_t)cb);
    if (obs_err != PB_OBSERVER_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }
    return PB_SPI_SUCCESS;
}


uint32_t        pb_spi_process(pb_spi_channel_t* channel)
{

}

/* 
    Have not managed properly code send function, TXC and DRE interrupt flag responses.
    May need to add some flags to the spi module so the interrupt handler can communicate
    busy_transmitting vs. ready for new data to send.

    Have to redefine send function since the SS EXTINT pins controls who to send to.
    Have to make it intuitive and functional. What I have done until now is sufficient
    for the situation I'm going to use it for, but it is not ok for a general situation.
*/