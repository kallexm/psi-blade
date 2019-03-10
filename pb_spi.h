#ifndef PB_SPI_H__
#define PB_SPI_H__

#include "samd21.h"
#include "pb_observer.h"
#include "pb_circular_buffer.h"

typedef enum
{
    PB_SPI_SUCCESS,
    PB_SPI_FAILURE,
    PB_SPI_BUSY,
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
} pb_spi_master_slave_select_t;

typedef enum
{
    PB_SPI_SLAVE_SELECT_LOW_DETECT_DISABLED = 0,
    PB_SPI_SLAVE_SELECT_LOW_DETECT_ENABLED  = 1,
} pb_spi_slave_select_low_detect_t;


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
    uint32_t                             bit_rate_in_Hz;
} pb_spi_config_t;

typedef enum
{
    PB_SPI_STATE_IDLE,
    PB_SPI_STATE_SENDING,
    PB_SPI_STATE_RECEIVING,
    PB_SPI_STATE_SEND_AND_RECEIVE,
} pb_spi_channel_state_t;


struct pb_spi_channel_s; // Needed because struct pb_spi_t and pb_spi_channel_t has pointers to each other

typedef void (*pb_spi_send_complete_cb_t)(struct pb_spi_channel_s* channel);
typedef void (*pb_spi_receive_complete_cb_t)(struct pb_spi_channel_s* channel, uint8_t* p_receive_buffer, uint32_t rec_buf_length);
typedef void (*pb_spi_send_and_receive_complete_cb_t)(struct pb_spi_channel_s* channel, uint8_t* p_receive_buffer, uint32_t rec_buf_length);

typedef struct
{
    SercomSpi*                                  spi_instance;
    pb_spi_config_t                             config;
    bool                                        enabled;

    struct pb_spi_channel_s*                    transmitting_channel;
    pb_spi_channel_state_t                      state;
    union
    {
        pb_spi_send_complete_cb_t               send_callback;
        pb_spi_receive_complete_cb_t            receive_callback;
        pb_spi_send_and_receive_complete_cb_t   send_and_receive_callback;
    };

    uint8_t* const                              send_data_buffer;
    uint32_t                                    send_data_length;
    uint32_t                                    send_index;
    uint32_t const                              send_buffer_size;
    
    uint8_t* const                              receive_data_buffer;
    uint32_t                                    receive_data_length;
    uint32_t                                    receive_index;
    uint32_t const                              receive_buffer_size;
} pb_spi_t;


typedef enum
{
    PB_SPI_CHANNEL_SS_TYPE_ENABLE_LOW,
    PB_SPI_CHANNEL_SS_TYPE_ENABLE_HIGH,
} pb_spi_channel_ss_type_t;


typedef struct pb_spi_channel_s
{
    pb_spi_t*                 spi_transport;
    uint16_t                  ss_pin;
    pb_spi_channel_ss_type_t  ss_type;

    uint32_t                  bytes_transmitted;
    uint32_t                  bytes_received;
} pb_spi_channel_t;


#define PB_SPI_TRANSPORT_DEF(name_, send_buffer_size_, receive_buffer_size_)      \
    uint8_t (name_ ## _send_buffer)[send_buffer_size_];                           \
    uint8_t (name_ ## _receive_buffer)[receive_buffer_size_];                     \
    pb_spi_t (name_) = {NULL, {0}, 0, {NULL}, NULL, PB_SPI_STATE_IDLE, NULL,      \
                        (name_ ## _send_buffer), 0, 0, (send_buffer_size_),       \
                        (name_ ## _receive_buffer), 0, 0, (receive_buffer_size_)} \

pb_spi_retval_t pb_spi_transport_init(pb_spi_t* spi, Sercom* SERCOMn, pb_spi_config_t* config);
pb_spi_retval_t pb_spi_channel_init(pb_spi_channel_t* channel, pb_spi_t* connect_to, uint16_t slave_select_pin, pb_spi_channel_ss_type_t slave_select_enable_value);

pb_spi_retval_t pb_spi_send(pb_spi_channel_t* channel, uint8_t* data, uint32_t data_length, pb_spi_send_complete_cb_t cb);
pb_spi_retval_t pb_spi_receive(pb_spi_channel_t* channel, uint32_t bytes_to_receive, pb_spi_receive_complete_cb_t cb);
pb_spi_retval_t pb_spi_send_and_receive(pb_spi_channel_t* channel, uint8_t* send_data, uint32_t send_data_length, pb_spi_send_and_receive_complete_cb_t cb);

#endif /* PB_SPI_H__ */
