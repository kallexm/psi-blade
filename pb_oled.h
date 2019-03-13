#ifndef PB_OLED_H__
#define PB_OLED_H__

#include "pb_spi.h"

#include <stdbool.h>
#include <stdint.h>

#define PB_OLED_COLUMN_BOTTOM   0
#define PB_OLED_COLUMN_TOP    127
#define PB_OLED_PAGE_BOTTOM     0
#define PB_OLED_PAGE_TOP        7



typedef enum
{
    PB_OLED_SUCCESS,
    PB_OLED_FAILURE,
} pb_oled_retval_t;

typedef struct
{
    uint8_t             data_inv_command_pin;
    uint8_t             inv_reset_pin;
    pb_spi_channel_t*   spi_channel;
} pb_oled_config_t;

typedef struct
{   
    uint8_t             data_inv_command_pin;
    uint8_t             inv_reset_pin;
    pb_spi_channel_t*   spi_channel;

    uint8_t             page_index;
    uint8_t             column_index;
    bool                wrap_enabled;
} pb_oled_t;

pb_oled_retval_t pb_oled_init(pb_oled_t* oled, pb_oled_config_t* config);
pb_oled_retval_t pb_oled_reset(pb_oled_t* oled);

pb_oled_retval_t pb_oled_display_on(pb_oled_t* oled);
pb_oled_retval_t pb_oled_display_off(pb_oled_t* oled);

pb_oled_retval_t pb_oled_set_contrast(pb_oled_t* oled, uint8_t value);


pb_oled_retval_t pb_oled_cursor_set(pb_oled_t* oled, uint8_t page, uint8_t column);
pb_oled_retval_t pb_oled_cursor_get(pb_oled_t* oled, uint8_t* page, uint8_t* column);
pb_oled_retval_t pb_oled_end_wrap_enable(pb_oled_t* oled);
pb_oled_retval_t pb_oled_end_wrap_disable(pb_oled_t* oled);

pb_oled_retval_t pb_oled_write_byte(pb_oled_t* oled, uint8_t byte);
pb_oled_retval_t pb_oled_write_array(pb_oled_t* oled, uint8_t* data, uint16_t data_length);

pb_oled_retval_t pb_oled_write_char(pb_oled_t* oled, char character);
pb_oled_retval_t pb_oled_write_string(pb_oled_t* oled, char* string, uint8_t string_length);

typedef enum
{
    PB_OLED_SS_STRAIGHT_BOARDER,
    PB_OLED_SS_LEFT_BOARDER,
    PB_OLED_SS_RIGHT_BOARDER,
    /* ... */
} pb_oled_special_sign_t;

pb_oled_retval_t pb_oled_write_special(pb_oled_t* oled, pb_oled_special_sign_t* signs, uint8_t signs_length);

#endif /* PB_OLED_H__ */

/*
    Possible improvements:
        - Currently all command functions can hang given that spi is not working correctly and spi state
          is never set to idle after the command is written.
          Possible solution: Can add a timeout function, using the timer library. So the command breaks free
          from the od{...}while(...); with a returned error if a timer expires (i.e. the function used to long a time to do its job).
*/


/*
    Går for page addressing mode som standard
    
    Should call reset on oled when initializing

    Have to create and add a character table to flash memory that can be used for lookup when
    writing characters and a special signs table for additional figures.
    Put in flash with section attribute

    Så relevante kommandoer:
        - Set Lower Column Start Address for Page Addressing Mode (0x00~0x0F)
        - Set Higher Column Start Address for Page Addressing Mode (0x10~0x1F)
        - Set Memory Addressing Mode (0x20, 0x02)
        - Set Page Start Address for Page Addressing Mode (0xB0~0xB7)
*/

/*
    What has to be configured in init function:
        - Display Clock Divide, and oscillator frequency (Set Display Clock Divide Ratio/Oscillator Frequency)
        - Set display start line
        - Set initial contrast(Set Contrast Control for BANK0)
*/

/* 
    What should be done to control the screen?
    What control functionality do I want?
    What should be done to write to the screen?
    How should the writing be done?

    Addressing modes:
    - The screen is setup with 8 pages (0(top)-7(bottom)) in the vertical direction
    - The screen has 128 columns (0(left)-127(right)) in the horizontal direction
    - When writing or reading, the value of 8 bits/pixels in vertical direction (D0(top)-D7(bottom)) inside a page and inside a column is written or read.
        1. Page addressing mode
            - When display ram is read or written, the column read/write pointer/index is increased by one.
              If it reaches the end, then it resets to 0. If another page should be written then it has to be explicitly set.
        2. Horizontal addressing mode
            - Same movement as page addressing, but when column 127 is reached and written, the ram pointer
              is reset to col 0 of next page. If on the last page, then reset page to 0.
        3. Vertical addressing mode
            - Walks down the columns and increases the column when at the last page. when at the
              last column and and last page, reset to first column, first page.
        Can be set with "Set Memory Addressing Mode" (0x20)

        COL means the graphic display data RAM column.

    Initialize oled to use the whole address space/pixel space with init function. Then
    supply a function that can be called to change lower and upper limit of the columns and pages.

    Supply a function used to change current ram access pointer location. Column and page indices.

    Define upper and lower page and column indices so that they can easily be changed.

    Add function to set contrast of screen. Value from 0 to 255.

    Commands:
        Set Memory Addressing Mode (0x20)
        Set Column Address (0x21)
        Set Page Address (0x22)
        Set Contrast Control for BANK0 (0x81)
        Set Segment Re-map (0xA0/0xA1) (Turn display upside down?)
        Entire Display ON (0xA4/0xA5) (0xA5 sets entire display to on/off and stops tracking of GDDRAM content. 0xA4 resumes tracking of GDDRAM content)
        Set Normal/INverse Display (0xA6/0xA7)
        Set Multiplex Ratio (0xA8) (Changes mapping from RAM to screen)
        Set Display ON/OFF (0xAF/0xAE)
        NOP (0xE3)

*/