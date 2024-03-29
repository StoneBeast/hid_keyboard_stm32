//
// Created by liush on 2024/3/21.
//

#ifndef C8XX_MXCUBE_CLION_USBKEYBOARD_MIDDLE_INTERFAC_H
#define C8XX_MXCUBE_CLION_USBKEYBOARD_MIDDLE_INTERFAC_H

#include "stm32f1xx.h"
#include "stm32f1xx_hal_gpio.h"
#include "keyboard.h"

#define GPIO_PIN(x) ((uint32_t)((uint32_t)0x01U<<(x)))

void gpio_port_write(GPIO_TypeDef *GPIOx, uint16_t data);
uint16_t gpio_output_port_get(GPIO_TypeDef *GPIOx);
void gpio_bit_reset(GPIO_TypeDef *GPIOx, uint32_t gpio_pin);
void gpio_bit_set(GPIO_TypeDef *GPIOx, uint32_t gpio_pin);
uint16_t gpio_input_port_get(GPIO_TypeDef *GPIOx);
void delay_ms(uint16_t ms);
uint8_t buffer_cmp(const uint8_t* buffer);
uint8_t* get_key_buffer();

void delay_us(int us);

void gpio_bit_write(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
uint8_t gpio_input_bit_get(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

uint8_t find_buffer(const uint8_t *buffer, uint8_t key_code);
void handle_led_gpio(uint8_t status_data);

#endif //C8XX_MXCUBE_CLION_USBKEYBOARD_MIDDLE_INTERFAC_H
