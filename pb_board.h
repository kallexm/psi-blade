#ifndef PB_BOARD_H__
#define PB_BOARD_H__

#include <samd21g18a.h>

/* OLED function port and pinout */
#define HARD_PIN_OLED_D_C_MODE           7
#define HARD_PIN_OLED_RESET              8
#define HARD_PIN_OLED_SPI_SDIN           9
#define HARD_PIN_OLED_SPI_SCLK          10
#define HARD_PIN_OLED_SPI_CS            11
#define HARD_PIN_OLED_SPI_UNUSED        12

#define PORTB_BIT_OLED_D_C_MODE          8
#define PORTB_BIT_OLED_RESET             9
#define PORTA_BIT_OLED_SPI_SDIN          4
#define PORTA_BIT_OLED_SPI_SCLK          5
#define PORTA_BIT_OLED_SPI_CS            6
#define PORTA_BIT_OLED_SPI_UNUSED        7

#define FUNCTION_OLED_BASE              SERCOM0

/* Unused port and pins */
#define HARD_PIN_UNUSED_0               13
#define HARD_PIN_UNUSED_1               16
#define HARD_PIN_UNUSED_2               19

#define PORTA_BIT_UNUSED_0               8
#define PORTA_BIT_UNUSED_1              11
#define PORTB_BIT_UNUSED_2              10

/* Motor+Encoder function port and pinout */
#define HARD_PIN_MOTOR_ENCODER_EVT_A    14
#define HARD_PIN_MOTOR_ENCODER_EVT_B    15
#define HARD_PIN_MOTOR_PWM              20
#define HARD_PIN_MOTOR_DIR              21
#define HARD_PIN_MOTOR_SLEEP            22

#define PORTA_BIT_MOTOR_ENCODER_EVT_A    9
#define PORTA_BIT_MOTOR_ENCODER_EVT_B   10
#define PORTB_BIT_MOTOR_PWM             11
#define PORTA_BIT_MOTOR_DIR             12
#define PORTA_BIT_MOTOR_SLEEP           13

/* MPU function port and pinout */
#define HARD_PIN_MPU_I2C_SDA            25
#define HARD_PIN_MPU_I2C_SCL            26
#define HARD_PIN_MPU_I2C_UNUSED_0       27
#define HARD_PIN_MPU_I2C_UNUSED_1       28
#define HARD_PIN_MPU_INTERRUPT          29

#define PORTA_BIT_MPU_I2C_SDA           16
#define PORTA_BIT_MPU_I2C_SCL           17
#define PORTA_BIT_MPU_I2C_UNUSED_0      18
#define PORTA_BIT_MPU_I2C_UNUSED_1      19
#define PORTA_BIT_MPU_INTERRUPT         20

#define FUNCTION_MPU_BASE               SERCOM1

/* USART function port and pinout */
#define HARD_PIN_USART_TXD              31
#define HARD_PIN_USART_RXD              32
#define HARD_PIN_USART_UNUSED_0         33
#define HARD_PIN_USART_UNUSED_1         34

#define PORTA_BIT_USART_TXD             22
#define PORTA_BIT_USART_RXD             23
#define PORTA_BIT_USART_UNUSED_0        24
#define PORTA_BIT_USART_UNUSED_1        25

#define FUNCTION_USART_BASE             SERCOM3

/* RGB LEDs function port and pinout */
#define HARD_PIN_LED_PEARLS             30
#define HARD_PIN_LED_DIAMOND            47
#define HARD_PIN_LED_BLADE              48

#define PORTA_BIT_LED_PEARLS            21
#define PORTB_BIT_LED_DIAMOND            2
#define PORTB_BIT_LED_BLADE              3

/* Button function port and pinout */
#define HARD_PIN_BUTTON_3               37
#define HARD_PIN_BUTTON_2               38
#define HARD_PIN_BUTTON_1               39
#define HARD_PIN_BUTTON_0               41

#define PORTB_BIT_BUTTON_3              22
#define PORTB_BIT_BUTTON_2              23
#define PORTA_BIT_BUTTON_1              27
#define PORTA_BIT_BUTTON_0              28

/* Serial Wire Debug port and pinout */
#define HARD_PIN_DEBUG_SWD_SWDCLK       45
#define HARD_PIN_DEBUG_SWD_SWDIO        46

#define PORTA_BIT_DEBUG_SWD_SWDCLK      30
#define PORTA_BIT_DEBUG_SWD_SWDIO       31

#endif /* PB_BOARD_H__ */
