#include "pb_spi.h"

#include "pb_gpio.h" // Is this needed? Try to remove and see if errors appear.
#include "pb_sercom_handlers.h"

#include "samd21.h"

#include <string.h>

#define PB_SPI_GCLK_GENERATOR_NUM 5
#define PB_SPI_GCLK_FREQUENCY     8000000


void pb_spi_on_interrupt(void* context)
{
    pb_spi_t* spi = (pb_spi_t*)context;

    uint8_t interrupt_flags = spi->spi_instance->INTFLAG.reg;
    if (interrupt_flags & SERCOM_SPI_INTFLAG_DRE != 0)
    {
        // Data register empty interrupt
        switch (spi->state)
        {
            case PB_SPI_STATE_SENDING:
            case PB_SPI_STATE_SEND_AND_RECEIVE:
                if (spi->send_index < spi->send_data_length)
                {
                    spi->spi_instance->DATA.bit.DATA = spi->send_data_buffer[spi->send_index];
                    spi->send_index++;
                    (spi->transmitting_channel->bytes_transmitted)++;
                }
                break;

            case PB_SPI_STATE_RECEIVING:
                if (spi->receive_index < spi->receive_data_length)
                {
                    spi->spi_instance->DATA.bit.DATA   = 0x00;
                }
                break;

            default:
                // Case idle and a DRE flag is raised: Do Nothing
                break;
        }
    }

    if (interrupt_flags & SERCOM_SPI_INTFLAG_TXC != 0)
    {
        // Transmit complete interrupt
        if (spi->transmitting_channel->ss_type == PB_SPI_CHANNEL_SS_TYPE_ENABLE_LOW)
        {
            pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_HIGH);
        }
        else
        {
            pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_LOW);
        }
        
        switch (spi->state)
        {
            case PB_SPI_STATE_SENDING:
                spi->send_callback(spi->transmitting_channel);
                break;

            case PB_SPI_STATE_RECEIVING:
                spi->receive_callback(spi->transmitting_channel, spi->receive_data_buffer, spi->receive_data_length);
                break;
            case PB_SPI_STATE_SEND_AND_RECEIVE:
                spi->send_and_receive_callback(spi->transmitting_channel, spi->receive_data_buffer, spi->receive_data_length);
                break;

            default:
                // Should not be reached
                break;
        }

        spi->state = PB_SPI_STATE_IDLE;
    }

    if (interrupt_flags & SERCOM_SPI_INTFLAG_RXC != 0)
    {
        // Receive complete interrupt
        uint16_t received_data = spi->spi_instance->DATA.bit.DATA;
        switch (spi->state)
        {
            case PB_SPI_STATE_RECEIVING:
            case PB_SPI_STATE_SEND_AND_RECEIVE:
                if (spi->receive_index < spi->receive_data_length)
                {
                    spi->receive_data_buffer[spi->receive_index] = received_data;
                    spi->receive_index++;
                    (spi->transmitting_channel->bytes_received)++;
                }
                break;

            default:
                // Do not save received data if in state PB_SPI_STATE_SENDING or PB_SPI_STATE_IDLE
                break;
        }
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
    switch ((uint32_t)SERCOMn)
    {
        case (uint32_t)SERCOM0:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM0_CORE_Val;
            break;

        case (uint32_t)SERCOM1:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM1_CORE_Val;
            break;

        case (uint32_t)SERCOM2:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM2_CORE_Val;
            break;

        case (uint32_t)SERCOM3:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM3_CORE_Val;
            break;

        case (uint32_t)SERCOM4:
            sercom_gclk_to_activate = GCLK_CLKCTRL_ID_SERCOM4_CORE_Val;
            break;

        case (uint32_t)SERCOM5:
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
    switch ((uint32_t)SERCOMn)
    {
        case (uint32_t)SERCOM0:
            sercom_interrupt_to_enable = SERCOM0_IRQn;
            break;

        case (uint32_t)SERCOM1:
            sercom_interrupt_to_enable = SERCOM1_IRQn;
            break;

        case (uint32_t)SERCOM2:
            sercom_interrupt_to_enable = SERCOM2_IRQn;
            break;

        case (uint32_t)SERCOM3:
            sercom_interrupt_to_enable = SERCOM3_IRQn;
            break;

        case (uint32_t)SERCOM4:
            sercom_interrupt_to_enable = SERCOM4_IRQn;
            break;

        case (uint32_t)SERCOM5:
            sercom_interrupt_to_enable = SERCOM5_IRQn;
            break;

        default:
            return PB_SPI_FAILURE;
    }

    NVIC_EnableIRQ(sercom_interrupt_to_enable);
    return PB_SPI_SUCCESS;
}


pb_spi_retval_t configure_sercom_pins(Sercom* SERCOMn, pb_spi_pins_multiplex_t pins_multiplex)
{
    uint8_t pins_to_configure[4] = {0};

    switch ((uint32_t)SERCOMn)
    {
        case (uint32_t)SERCOM0:
            switch (pins_multiplex)
            {
                case PB_SPI_PINS_MULTIPLEX_MAIN:
                    pins_to_configure[0] = 13; pins_to_configure[1] = 14;
                    pins_to_configure[2] = 15; pins_to_configure[3] = 16;
                    break;

                case PB_SPI_PINS_MULTIPLEX_ALTERNATIVE:
                    pins_to_configure[0] = 9;  pins_to_configure[1] = 10;
                    pins_to_configure[2] = 11; pins_to_configure[3] = 12;
                    break;

                default:
                    return PB_SPI_FAILURE;
            }
            break;

        case (uint32_t)SERCOM1:
            switch (pins_multiplex)
            {
                case PB_SPI_PINS_MULTIPLEX_MAIN:
                    pins_to_configure[0] = 25; pins_to_configure[1] = 26;
                    pins_to_configure[2] = 27; pins_to_configure[3] = 28;
                    break;

                case PB_SPI_PINS_MULTIPLEX_ALTERNATIVE:
                    pins_to_configure[0] = 1;  pins_to_configure[1] = 2;
                    pins_to_configure[2] = 45; pins_to_configure[3] = 46;
                    break;

                default:
                    return PB_SPI_FAILURE;
            }
            break;

        case (uint32_t)SERCOM2:
            switch (pins_multiplex)
            {
                case PB_SPI_PINS_MULTIPLEX_MAIN:
                    pins_to_configure[0] = 21; pins_to_configure[1] = 22;
                    pins_to_configure[2] = 23; pins_to_configure[3] = 24;
                    break;

                case PB_SPI_PINS_MULTIPLEX_ALTERNATIVE:
                    pins_to_configure[0] = 13; pins_to_configure[1] = 14;
                    pins_to_configure[2] = 15; pins_to_configure[3] = 16;
                    break;

                default:
                    return PB_SPI_FAILURE;
            }
            break;

        case (uint32_t)SERCOM3:
            switch (pins_multiplex)
            {
                case PB_SPI_PINS_MULTIPLEX_MAIN:
                    pins_to_configure[0] = 31; pins_to_configure[1] = 32;
                    pins_to_configure[2] = 33; pins_to_configure[3] = 34;
                    break;

                case PB_SPI_PINS_MULTIPLEX_ALTERNATIVE:
                    pins_to_configure[0] = 25; pins_to_configure[1] = 26;
                    pins_to_configure[2] = 27; pins_to_configure[3] = 28;
                    break;

                default:
                    return PB_SPI_FAILURE;
            }
            break;

        case (uint32_t)SERCOM4:
            switch (pins_multiplex)
            {
                case PB_SPI_PINS_MULTIPLEX_MAIN:
                    pins_to_configure[0] = 0; pins_to_configure[1] = 0;
                    pins_to_configure[2] = 0; pins_to_configure[3] = 0;
                    break;

                case PB_SPI_PINS_MULTIPLEX_ALTERNATIVE:
                    pins_to_configure[0] = 7;  pins_to_configure[1] = 8;
                    pins_to_configure[2] = 19; pins_to_configure[3] = 20;
                    break;

                default:
                    return PB_SPI_FAILURE;
            }
            break;

        case (uint32_t)SERCOM5:
            switch (pins_multiplex)
            {
                case PB_SPI_PINS_MULTIPLEX_MAIN:
                    pins_to_configure[0] = 0;  pins_to_configure[1] = 0;
                    pins_to_configure[2] = 29; pins_to_configure[3] = 30;
                    break;

                case PB_SPI_PINS_MULTIPLEX_ALTERNATIVE:
                    pins_to_configure[0] = 47; pins_to_configure[1] = 48;
                    pins_to_configure[2] = 37; pins_to_configure[3] = 38;
                    break;

                default:
                    return PB_SPI_FAILURE;
            }
            break;

        default:
            return PB_SPI_FAILURE;
    }

    pb_gpio_retval_t gpio_err;
    for (int i = 0; i < 4; i++)
    {
        if (pins_to_configure[i] != 0)
        {
            gpio_err = pb_gpio_pin_periph_func_set(pins_to_configure[i], (pb_gpio_periph_func_t)pins_multiplex);
            if (gpio_err != PB_GPIO_SUCCESS)
            {
                return PB_GPIO_FAILURE;
            }
        }
    }
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

    /* Configure/setup clock source */
    pb_spi_retval_t err1 = enable_clock(SERCOMn);
    if (err1 != PB_SPI_SUCCESS)
    {
        return err1;
    }

    /* Configure the needed gpio pins to work with spi/sercom */
    pb_spi_retval_t spi_err = configure_sercom_pins(SERCOMn, spi->config.pins_multiplex);
    if (spi_err != PB_SPI_SUCCESS)
    {
        return spi_err;
    }

    /* Set hardware registers of spi/sercom */
    spi->spi_instance->CTRLA.bit.MODE     = spi->config.mode;
    spi->spi_instance->CTRLA.bit.DORD     = spi->config.data_order;
    spi->spi_instance->CTRLA.bit.CPOL     = spi->config.clock_polarity;
    spi->spi_instance->CTRLA.bit.CPHA     = spi->config.clock_phase;
    spi->spi_instance->CTRLA.bit.RUNSTDBY = spi->config.run_in_standby;
    spi->spi_instance->CTRLA.bit.DOPO     = spi->config.data_out_pin;
    spi->spi_instance->CTRLA.bit.DIPO     = spi->config.data_in_pin;
    spi->spi_instance->CTRLB.bit.CHSIZE   = 0;
    spi->spi_instance->CTRLB.bit.MSSEN    = spi->config.master_slave_select;
    spi->spi_instance->CTRLB.bit.SSDE     = spi->config.slave_select_low_detect;

    spi->spi_instance->BAUD.reg = ((PB_SPI_GCLK_FREQUENCY)/(2*(spi->config.bit_rate_in_Hz))) - 1;
    
    spi->spi_instance->CTRLB.bit.RXEN   = 1;
    spi->spi_instance->INTENSET.bit.RXC = 1;
    spi->spi_instance->INTENSET.bit.TXC = 1;
    spi->spi_instance->INTENSET.bit.DRE = 1;

    /* Register and enable interrupts for spi instance */
    pb_sercom_handlers_retval_t sc_hnl_err = pb_sercom_handlers_register_callback((Sercom*)(spi->spi_instance), pb_spi_on_interrupt, (void*)spi);
    pb_spi_retval_t err2 = enable_interrupt(SERCOMn);
    if (err2 != PB_SPI_SUCCESS)
    {
        return err2;
    }

    /* Enable SPI instance */
    spi->spi_instance->CTRLA.bit.ENABLE = 1;
    while (spi->spi_instance->SYNCBUSY.bit.ENABLE)
    {
        // Synchronizing
    }

    return PB_SPI_SUCCESS;
}


// Reinskrive denne funksjonen
pb_spi_retval_t pb_spi_channel_init(pb_spi_channel_t* channel, pb_spi_t* connect_to, uint16_t slave_select_pin, pb_spi_channel_ss_type_t slave_select_enable_value)
{
    if (connect_to->enabled == false)
    {
        return PB_SPI_FAILURE;
    }

    channel->spi_transport = connect_to;
    if (channel->spi_transport == NULL)
    {
        return PB_SPI_FAILURE;
    }

    /* Slave select pin config. Output direction and set high or low depending on argument slave_select_enable_value */
    channel->ss_pin = slave_select_pin;
    pb_gpio_retval_t gpio_err = pb_gpio_pin_dir(channel->ss_pin, DIRECTION_OUT);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }
    if (slave_select_enable_value == PB_SPI_CHANNEL_SS_TYPE_ENABLE_LOW)
    {
        gpio_err = pb_gpio_pin_write(channel->ss_pin, VALUE_HIGH);
    }
    else if (slave_select_enable_value == PB_SPI_CHANNEL_SS_TYPE_ENABLE_HIGH)
    {
        gpio_err = pb_gpio_pin_write(channel->ss_pin, VALUE_LOW);
    }
    else
    {
        return PB_SPI_FAILURE;
    }

    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_SPI_FAILURE;
    }

    return PB_SPI_SUCCESS;
}


pb_spi_retval_t pb_spi_ready(pb_spi_t* spi)
{
    if (spi->enabled == false)
    {
        return PB_SPI_FAILURE;
    }

    if (spi->state != PB_SPI_STATE_IDLE)
    {
        return PB_SPI_BUSY;
    }
    return PB_SPI_SUCCESS;
}


pb_spi_retval_t pb_spi_send(pb_spi_channel_t* channel, uint8_t* data, uint32_t data_length, pb_spi_send_complete_cb_t cb)
{
    pb_spi_t* spi = channel->spi_transport;

    pb_spi_retval_t spi_retval = pb_spi_ready(spi);
    if (spi_retval != PB_SPI_SUCCESS)
    {
        return spi_retval;
    }

    if (data_length > spi->send_buffer_size)
    {
        return PB_SPI_FAILURE;
    }

    spi->transmitting_channel = channel;
    spi->state                = PB_SPI_STATE_SENDING;
    spi->send_callback        = (void*)cb;

    spi->send_data_length = data_length;
    memcpy(spi->send_data_buffer, data, data_length);

    // Start transfer
    if (spi->transmitting_channel->ss_type == PB_SPI_CHANNEL_SS_TYPE_ENABLE_LOW)
    {
        pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_LOW);
    }
    else
    {
        pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_HIGH);
    }

    spi->spi_instance->DATA.bit.DATA = spi->send_data_buffer[0];
    spi->send_index = 1;

    return PB_SPI_SUCCESS;
}


pb_spi_retval_t pb_spi_receive(pb_spi_channel_t* channel, uint32_t bytes_to_receive, pb_spi_receive_complete_cb_t cb)
{
    pb_spi_t* spi = channel->spi_transport;

    pb_spi_retval_t spi_retval = pb_spi_ready(spi);
    if (spi_retval != PB_SPI_SUCCESS)
    {
        return spi_retval;
    }

    if (bytes_to_receive > spi->receive_buffer_size)
    {
        return PB_SPI_FAILURE;
    }

    spi->transmitting_channel    = channel;
    spi->state                   = PB_SPI_STATE_RECEIVING;
    spi->receive_callback        = (void*)cb;

    spi->receive_data_length = bytes_to_receive;
    spi->receive_index = 0;

    // Start transfer
    if (spi->transmitting_channel->ss_type == PB_SPI_CHANNEL_SS_TYPE_ENABLE_LOW)
    {
        pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_LOW);
    }
    else
    {
        pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_HIGH);
    }

    spi->spi_instance->DATA.bit.DATA = 0x00;

    return PB_SPI_SUCCESS;
}


pb_spi_retval_t pb_spi_send_and_receive(pb_spi_channel_t* channel, uint8_t* send_data, uint32_t send_data_length, pb_spi_send_and_receive_complete_cb_t cb)
{
    pb_spi_t* spi = channel->spi_transport;

    pb_spi_retval_t spi_retval = pb_spi_ready(spi);
    if (spi_retval != PB_SPI_SUCCESS)
    {
        return spi_retval;
    }

    if (send_data_length > spi->send_buffer_size || send_data_length > spi->receive_buffer_size)
    {
        return PB_SPI_FAILURE;
    }

    spi->transmitting_channel      = channel;
    spi->state                     = PB_SPI_STATE_SEND_AND_RECEIVE;
    spi->send_and_receive_callback = (void*)cb;

    spi->send_data_length = send_data_length;
    memcpy(spi->send_data_buffer, send_data, send_data_length);

    spi->receive_data_length = send_data_length;
    spi->receive_index = 0;

    // Start transfer
    if (spi->transmitting_channel->ss_type == PB_SPI_CHANNEL_SS_TYPE_ENABLE_LOW)
    {
        pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_LOW);
    }
    else
    {
        pb_gpio_pin_write(spi->transmitting_channel->ss_pin, VALUE_HIGH);
    }
    
    spi->spi_instance->DATA.bit.DATA = spi->send_data_buffer[0];
    spi->send_index = 1;

    return PB_SPI_SUCCESS;
}
