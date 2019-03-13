#include "pb_oled.h"

#include "pb_gpio.h"
#include "pb_spi.h"
#include "pb_critical_sections.h"

#include "pb_oled_symbol_tables.h"

//#include <stdlib.h>
#include <string.h>

#define CMD_SET_MEM_ADDR_MODE 0x20
#define ARG_PAGE_ADDR_MODE    0x02

#define CMD_SET_CONTRAST      0x81
#define ARG_DEFAULT_CONTRAST  0x64

#define CMD_SET_DISPLAY_ON    0xAF
#define CMD_SET_DISPLAY_OFF   0xAE

#define CMD_SET_LOW_COLUMN_START_ADDR     0x00
#define CMD_SET_HIGH_COLUMN_START_ADDR    0x10
#define ARG_LOW_COLUMN_START_ADDR_MASK    0x0F
#define ARG_HIGH_COLUMN_START_ADDR_OFFSET 0x04

#define CMD_SET_PAGE_START_ADDR           0xB0
#define ARG_PAGE_START_ADDR_MASK          0x07


pb_oled_retval_t write_command(pb_oled_t* oled, uint8_t* command, uint8_t command_length)
{
    bool is_ready = pb_spi_is_ready(oled->spi_channel);
    if (is_ready == false)
    {
        return PB_OLED_FAILURE;
    }

    pb_gpio_retval_t gpio_err = pb_gpio_pin_write(oled->data_inv_command_pin, VALUE_LOW);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_OLED_FAILURE;
    }

    pb_spi_retval_t spi_err = pb_spi_send(oled->spi_channel, command, command_length, NULL);
    if (spi_err != PB_SPI_SUCCESS)
    {
        return PB_OLED_FAILURE;
    }
    return PB_OLED_SUCCESS;
}


pb_oled_retval_t pb_oled_init(pb_oled_t* oled, pb_oled_config_t* config)
{
    oled->data_inv_command_pin = config->data_inv_command_pin;
    oled->inv_reset_pin        = config->inv_reset_pin;
    oled->spi_channel          = config->spi_channel;

    oled->page_index     = 0;
    oled->column_index   = 0;
    oled->wrap_enabled   = false;

    pb_gpio_retval_t gpio_err;

    /* Initialize data/command pin and set low (command mode) */
    gpio_err = pb_gpio_pin_dir(oled->data_inv_command_pin, DIRECTION_OUT);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_OLED_FAILURE;
    }
    gpio_err = pb_gpio_pin_write(oled->data_inv_command_pin, VALUE_LOW);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_OLED_FAILURE;
    }

    /* Initialize reset pin and set low to trigger a reset and then pull high */
    gpio_err = pb_gpio_pin_dir(oled->inv_reset_pin, DIRECTION_OUT);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_OLED_FAILURE;
    }
    gpio_err = pb_gpio_pin_write(oled->inv_reset_pin, VALUE_LOW);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_OLED_FAILURE;
    }
    gpio_err = pb_gpio_pin_write(oled->inv_reset_pin, VALUE_HIGH);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_OLED_FAILURE;
    }

    pb_spi_channel_t* channel    = oled->spi_channel;
    pb_oled_retval_t oled_retval = PB_OLED_FAILURE;

    /* Set Address mode */
    uint8_t addr_mode_cmd[2] = {CMD_SET_MEM_ADDR_MODE, ARG_PAGE_ADDR_MODE};
    do
    {
        oled_retval = write_command(oled, addr_mode_cmd, sizeof(addr_mode_cmd));
    }
    while (oled_retval != PB_OLED_SUCCESS);

    /* Set contrast */
    uint8_t contrast_cmd[2] = {CMD_SET_CONTRAST, ARG_DEFAULT_CONTRAST};
    do
    {
        oled_retval = write_command(oled, contrast_cmd, sizeof(contrast_cmd));
    }
    while (oled_retval != PB_OLED_SUCCESS);
    
    /* Turn on display */
    uint8_t display_on_cmd[1] = {CMD_SET_DISPLAY_ON};
    do
    {
        oled_retval = write_command(oled, display_on_cmd, sizeof(display_on_cmd));
    }
    while (oled_retval != PB_SPI_SUCCESS);

    return PB_OLED_SUCCESS;
}


pb_oled_retval_t pb_oled_reset(pb_oled_t* oled)
{
    pb_oled_config_t config = {oled->data_inv_command_pin, oled->inv_reset_pin, oled->spi_channel};
    pb_oled_retval_t oled_retval = pb_oled_init(oled, &config);
    return oled_retval;
}


pb_oled_retval_t pb_oled_display_on(pb_oled_t* oled)
{
    pb_oled_retval_t oled_retval = PB_OLED_FAILURE;

    uint8_t display_on_cmd[1] = {CMD_SET_DISPLAY_ON};
    do
    {
        oled_retval = write_command(oled, display_on_cmd, sizeof(display_on_cmd));
    }
    while (oled_retval != PB_OLED_SUCCESS);

    return PB_OLED_SUCCESS;
}


pb_oled_retval_t pb_oled_display_off(pb_oled_t* oled)
{
    pb_oled_retval_t oled_retval = PB_OLED_FAILURE;

    uint8_t display_off_cmd[1] = {CMD_SET_DISPLAY_OFF};
    do
    {
        oled_retval = write_command(oled, display_off_cmd, sizeof(display_off_cmd));
    }
    while (oled_retval != PB_OLED_SUCCESS);

    return PB_OLED_SUCCESS;
}


pb_oled_retval_t pb_oled_set_contrast(pb_oled_t* oled, uint8_t value)
{
    pb_oled_retval_t oled_retval = PB_OLED_FAILURE;

    uint8_t contrast_cmd[2] = {CMD_SET_CONTRAST, value};
    do
    {
        oled_retval = write_command(oled, contrast_cmd, sizeof(contrast_cmd));
    }
    while (oled_retval != PB_OLED_SUCCESS);

    return PB_OLED_SUCCESS;
}


pb_oled_retval_t pb_oled_cursor_set(pb_oled_t* oled, uint8_t page, uint8_t column)
{
    pb_oled_retval_t oled_retval = PB_OLED_FAILURE;

    uint8_t set_low_col_addr_cmd = CMD_SET_LOW_COLUMN_START_ADDR | (column & ARG_LOW_COLUMN_START_ADDR_MASK);
    do
    {
        oled_retval = write_command(oled, &set_low_col_addr_cmd, sizeof(set_low_col_addr_cmd));
    }
    while (oled_retval != PB_OLED_SUCCESS);

    uint8_t set_high_col_addr_cmd = CMD_SET_HIGH_COLUMN_START_ADDR | (column >> ARG_HIGH_COLUMN_START_ADDR_OFFSET);
    do
    {
        oled_retval = write_command(oled, &set_high_col_addr_cmd, sizeof(set_high_col_addr_cmd));
    }
    while (oled_retval != PB_OLED_SUCCESS);
    
    oled->column_index = (column & 0x7F);

    uint8_t set_page_addr_cmd = CMD_SET_PAGE_START_ADDR | (page & ARG_PAGE_START_ADDR_MASK);
    do
    {
        oled_retval = write_command(oled, &set_page_addr_cmd, sizeof(set_page_addr_cmd));
    }
    while (oled_retval != PB_OLED_SUCCESS);

    oled->page_index = (page & 0x07);

    return PB_OLED_SUCCESS;
}
  

pb_oled_retval_t pb_oled_cursor_get(pb_oled_t* oled, uint8_t* page, uint8_t* column)
{
    if (page != NULL)
    {
        *page   = oled->page_index;
    }
    
    if (column != NULL)
    {
        *column = oled->column_index;
    }
    
    return PB_OLED_SUCCESS;
}


pb_oled_retval_t pb_oled_end_wrap_enable(pb_oled_t* oled)
{
    oled->wrap_enabled = true;
    return PB_OLED_SUCCESS;
}


pb_oled_retval_t pb_oled_end_wrap_disable(pb_oled_t* oled)
{
    oled->wrap_enabled = false;
    return PB_OLED_SUCCESS;
}

// Need to add code that increases and decreases page_index and column_index correctly and
// add the feature of character wrapping if character wrapping is enabled.

pb_oled_retval_t pb_oled_write_array(pb_oled_t* oled, uint8_t* data, uint16_t data_length)
{
    bool is_ready = pb_spi_is_ready(oled->spi_channel);
    if (is_ready == false)
    {
        return PB_OLED_FAILURE;
    }

    pb_gpio_retval_t gpio_err = pb_gpio_pin_write(oled->data_inv_command_pin, VALUE_HIGH);
    if (gpio_err != PB_GPIO_SUCCESS)
    {
        return PB_OLED_FAILURE;
    }

    /* ========== */
    uint16_t written_data = 0;
    pb_spi_retval_t spi_err;

    while (written_data == data_length)
    {
        if ( (PB_OLED_COLUMN_TOP - oled->column_index) > (data_length - written_data) )
        {
            do
            {
                spi_err = pb_spi_send(oled->spi_channel, &(data[written_data]), data_length - written_data, NULL);
            }
            while (spi_err != PB_SPI_SUCCESS);
            oled->column_index = oled->column_index + (data_length - written_data);
            written_data = data_length;
        }
        else
        {
            do
            {
                spi_err = pb_spi_send(oled->spi_channel, &(data[written_data]), PB_OLED_COLUMN_TOP - oled->column_index, NULL);
            }
            while (spi_err != PB_SPI_SUCCESS);
            written_data = PB_OLED_COLUMN_TOP - oled->column_index;
            oled->column_index = 0;
            if (oled->wrap_enabled == true)
            {
                if (oled->page_index < PB_OLED_PAGE_TOP)
                {
                    oled->page_index = oled->page_index + 1;
                }
                else
                {
                    oled->page_index = 0;
                }
            }
        }
    }
    return PB_OLED_SUCCESS;
    /* ========== */
}


pb_oled_retval_t pb_oled_write_byte(pb_oled_t* oled, uint8_t byte)
{
    pb_oled_retval_t oled_retval = pb_oled_write_array(oled, &byte, 1);
    return oled_retval;
}

#define SYMBOL_BUFFER_SIZE 16

pb_oled_retval_t pb_oled_write_string(pb_oled_t* oled, char* string, uint8_t string_length)
{
    uint8_t data_to_write[SYMBOL_BUFFER_SIZE*CHAR_COLUMN_SIZE];
    uint16_t written_symbols = 0;

    while (written_symbols == string_length)
    {
        uint16_t num_symbols_to_write;
        if ( SYMBOL_BUFFER_SIZE < (string_length - written_symbols) )
        {
            num_symbols_to_write = SYMBOL_BUFFER_SIZE;
        }
        else
        {
            num_symbols_to_write = string_length - written_symbols;
        }

        for (int i = 0; i < num_symbols_to_write; i++)
        {
            uint8_t symbol_index = (uint8_t)(string[written_symbols + i]) - CHAR_VALUE_TO_INDEX_OFFSET;
            memcpy(&(data_to_write[i*CHAR_COLUMN_SIZE]), char_table[symbol_index], CHAR_COLUMN_SIZE);
        }
        
        pb_oled_retval_t oled_retval;
        do
        {
            oled_retval = pb_oled_write_array(oled, data_to_write, num_symbols_to_write*CHAR_COLUMN_SIZE);
        }
        while (oled_retval != PB_OLED_SUCCESS);
        written_symbols = written_symbols + num_symbols_to_write;
    }
    return PB_OLED_SUCCESS;

    /* ========== */
    /*uint8_t data_to_write[SYMBOL_BUFFER_SIZE*CHAR_COLUMN_SIZE];

    uint16_t written_symbols = 0;
    pb_oled_retval_t oled_err;

    while (written_symbols == string_length)
    {
        if ( SYMBOL_BUFFER_SIZE < (string_length - written_symbols) )
        {
            for (int i = 0; i < SYMBOL_BUFFER_SIZE; i++)
            {
                uint8_t symbol_index = (uint8_t)(string[written_symbols + i]) - CHAR_VALUE_TO_INDEX_OFFSET;
                memcpy(&(data_to_write[i*CHAR_COLUMN_SIZE]), char_table[symbol_index], CHAR_COLUMN_SIZE);
            }
            do
            {
                oled_retval = pb_oled_write_array(oled, &data_to_write, sizeof(data_to_write));
            }
            while (oled_retval != PB_OLED_SUCCESS);
            written_symbols = written_symbols + SYMBOL_BUFFER_SIZE;
        }
        else
        {
            for (int i = 0; i < (string_length - written_symbols); i++)
            {
                uint8_t symbol_index = (uint8_t)(string[written_symbols + i]) - CHAR_VALUE_TO_INDEX_OFFSET;
                memcpy(&(data_to_write[i*CHAR_COLUMN_SIZE]), char_table[symbol_index], CHAR_COLUMN_SIZE);
            }
            do
            {
                oled_retval = pb_oled_write_array(oled, &data_to_write, (string_length - written_symbols)*CHAR_COLUMN_SIZE);
            }
            while (oled_retval != PB_OLED_SUCCESS);
            written_symbols = string_length;
        }
    }
    return PB_OLED_SUCCESS;*/
    /* ========== */
}


pb_oled_retval_t pb_oled_write_char(pb_oled_t* oled, char character)
{
    pb_oled_retval_t oled_retval = pb_oled_write_string(oled, &character, 1);
    return oled_retval;
}


pb_oled_retval_t pb_oled_write_special(pb_oled_t* oled, pb_oled_special_sign_t* signs, uint8_t signs_length)
{
    return PB_OLED_SUCCESS;
}
