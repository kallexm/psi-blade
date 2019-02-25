#ifndef PB_SPI_H__
#define PB_SPI_H__

#define PB_SPI_MAX_CHANNELS_PER_SPI_INSTANCE 2

/*
    1. SPI instance define and memory allocation macro
        a. Parameters: name, output_queue_size, input_queue_size, num_observers
        b. Setup memory for output buffer (pb_circular_buffer)
        c. Setup memory for input buffer (pb_circular_buffer)
        d. Define input observer

    2. Init function
        a. Input parameters: SPI instance name, SERCOM[n] to use and configure,
        b. Sett pin function
        c. Enabling clock
        d. Configure module
        e. Enable interrupt
        f. If output_queue_size or input_queue_size is 0 (zero), then disable the
           functionality associated with it.

    3. Send function
        a. Can queue data to a queue.
           Data is sent when SPI hardware is ready (generates interrupt).
        b. Waiting loop, pulling a ready flag
    
    4. Receive function
        a. Interrupt is generated on receive.
           Interrupt handler takes data and queues it to a receive queue.
        b. A process function that should be run from main loop, dequeues data
           and notifies all subscribed parties by calling their callback function.
           Use observer module to call callback functions of subscribed modules.

    5. Subscribe to notification
    6. Unsubscribe to notification
    7. Process incomming data function

 */

#include "samd21.h"
#include "pb_observer.h"
#include "pb_circular_buffer.h"


typedef enum
{
    PB_SPI_SUCCESS,
    PB_SPI_FAILURE,
} pb_spi_retval_t;

typedef enum
{
    PB_SPI_MODE_SLAVE  = 0x2,
    PB_SPI_MODE_MASTER = 0x3,
} pb_spi_mode_t;

typedef enum
{
    PB_SPI_DATA_ORDER_MSB = 0,
    PB_SPI_DATA_ORDER_LSB = 1,
} pb_spi_data_order_t;

typedef enum
{
    PB_SPI_CLOCK_POLARITY_LOW_WHEN_IDLE  = 0,
    PB_SPI_CLOCK_POLARITY_HIGH_WHEN_IDLE = 1,
} pb_spi_clock_polarity_t;

typedef enum
{
    PB_SPI_CLOCK_PHASE_LEADING_SAMPLING  = 0,
    PB_SPI_CLOCK_PHASE_TRAILING_SAMPLING = 1,
} pb_spi_clock_phase_t;

typedef enum
{
    PB_SPI_DATA_IN_PIN_0 = 0x0,
    PB_SPI_DATA_IN_PIN_1 = 0x1,
    PB_SPI_DATA_IN_PIN_2 = 0x2,
    PB_SPI_DATA_IN_PIN_3 = 0x3,
} pb_spi_data_in_pin_t;

typedef enum
{
    PB_SPI_DATA_OUT_PIN_DO0_SCK1_SS2 = 0x0,
    PB_SPI_DATA_OUT_PIN_DO2_SCK3_SS1 = 0x1,
    PB_SPI_DATA_OUT_PIN_DO3_SCK1_SS2 = 0x2,
    PB_SPI_DATA_OUT_PIN_DO0_SCK3_SS1 = 0x3,
} pb_spi_data_out_pin_t;

typedef enum
{
    PB_SPI_PINS_MULTIPLEX_MAIN        = 0x2,
    PB_SPI_PINS_MULTIPLEX_ALTERNATIVE = 0x3,
} pb_spi_pins_multiplex_t;

typedef enum
{
    PB_SPI_RUN_IN_STANDBY_MODE_1 = 0x0,
    PB_SPI_RUN_IN_STANDBY_MODE_2 = 0x1,
} pb_spi_run_in_standby_t;

typedef enum
{
    PB_SPI_MASTER_SLAVE_SELECT_DISABLED = 0,
    PB_SPI_MASTER_SLAVE_SELECT_ENABLED  = 1,
} pb_spi_master_slave_select_enable_t;

typedef enum
{
    PB_SPI_SLAVE_SELECT_LOW_DETECT_DISABLED = 0,
    PB_SPI_SLAVE_SELECT_LOW_DETECT_ENABLED  = 1,
} pb_spi_slave_select_low_detect_t;

/*typedef enum
{
    PB_SPI_CHARACTER_SIZE_8BITS = 0x0,
    PB_SPI_CHARACTER_SIZE_9BITS = 0x1,
} pb_spi_character_size_t;*/

typedef struct
{
    pb_spi_mode_t                        mode;
    pb_spi_data_order_t                  data_order;
    pb_spi_clock_polarity_t              clock_polarity;
    pb_spi_clock_phase_t                 clock_phase;
    pb_spi_data_in_pin_t                 data_in_pin;
    pb_spi_data_out_pin_t                data_out_pin;
    pb_spi_pins_multiplex_t              pins_multiplex;
    pb_spi_run_in_standby_t              run_in_standby;
    pb_spi_master_slave_select_t         master_slave_select;
    pb_spi_slave_select_low_detect_t     slave_select_low_detect;
  /*pb_spi_character_size_t              character_size;*/
    uint32_t                             bit_rate_in_Hz;
} pb_spi_config_t;


typedef struct
{
    SercomSpi*                      spi_instance;
    pb_spi_config_t                 config;
    bool                            enabled;
    bool                            is_transmitting;
    pb_spi_channel_t*               transmitting_channel;
    pb_spi_transmit_complete_cb_t   on_transmit_complete;
    pb_spi_channel_t*               connected_channels[PB_SPI_MAX_CHANNELS_PER_SPI_INSTANCE];
    bool                            slave_read_request_pending[PB_SPI_MAX_CHANNELS_PER_SPI_INSTANCE];
} pb_spi_t;


typedef struct
{
    uint16_t slave_select_pin;
    uint16_t slave_read_request_interrupt_pin;
} pb_spi_channel_config_t;


typedef struct
{
    pb_observer_t*  p_observer;
    pb_crclrbuf_t*  p_out_queue;
    pb_crclrbuf_t*  p_in_queue;
    uint16_t        size_out_queue;
    uint16_t        size_in_queue;

    uint32_t        bytes_transmitted;
    uint32_t        bytes_received;
    uint32_t        bytes_overflowed;

    uint16_t        ss_pin;
    uint16_t        slave_request_pin;

    bool            is_transmitting;
    pb_spi_t*       spi_transport;
} pb_spi_channel_t;


typedef void (*pb_spi_on_evt_cb_t)(void* context);

typedef void (*pb_spi_transmit_complete_cb_t)(pb_spi_channel_t* channel);

#define PB_SPI_DEF(name_)  \
    pb_spi_t (name_);      \


#define PB_SPI_CHANNEL_DEF(name_, out_queue_size_, in_queue_size_, num_observers_) \
    PB_CRCLRBUF_DEF(name_ ## _out_queue, out_queue_size_, sizeof(uint8_t));        \
    PB_CRCLRBUF_DEF(name_ ## _in_queue, in_queue_size_, sizeof(uint8_t));          \
    PB_OBSERVER_DEF(name_ ## _observer, num_observers_);                           \
    pb_spi_channel_t (name_) = {(name_ ## _observer), (name_ ## _out_queue),       \
        (name_ ## _in_queue), out_queue_size_, in_queue_size, 0, 0, 0}             \

pb_spi_retval_t pb_spi_transport_init(pb_spi_t* spi, Sercom* SERCOMn, pb_spi_config_t* config);
pb_spi_retval_t pb_spi_channel_init(pb_spi_channel_t* channel, pb_spi_t* connect_to, pb_spi_channel_config_t* config);

pb_spi_retval_t pb_spi_queue_data(pb_spi_channel_t* channel, char char_to_send);
pb_spi_retval_t pb_spi_send_data(pb_spi_channel_t* channel, pb_spi_transmit_complete_cb_t cb);

pb_spi_retval_t pb_spi_subscribe(pb_spi_channel_t* channel, pb_spi_on_evt_cb_t cb, void* context);
pb_spi_retval_t pb_spi_unsubscribe(pb_spi_channel_t* channel, pb_spi_on_evt_cb_t cb);

uint32_t        pb_spi_process(pb_spi_channel_t* channel);



#endif /* PB_SPI_H__ */