//
// Created by liush on 2024/3/22.
//

#include "debug_tools.h"
#include "middle_interfac.h"


#define DEBUG_PORT GPIOC

uint32_t debug_pin;

void debug_port_init(uint32_t GPIOF_PIN)
{
  debug_pin = GPIOF_PIN;


  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DEBUG_PORT, debug_pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = debug_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DEBUG_PORT, &GPIO_InitStruct);

}

void debug_port_num(uint8_t n)
{
  uint8_t temp = n+1;

  // while (time++ < 200000);
  delay_ms(50);
  while (temp>0)
  {
    gpio_bit_set(DEBUG_PORT, debug_pin);
    gpio_bit_reset(DEBUG_PORT, debug_pin);
    temp--;
  }
}

void debug_port_num_code_by(uint8_t data)
{
  delay_us(150);
  for (uint8_t i = 0; i < 8; i++)
  {
    gpio_bit_set(DEBUG_PORT, debug_pin);
    delay_us(50);
    gpio_bit_write(DEBUG_PORT, debug_pin,
                   ((data >> i) & 0x01 == 1)? SET:RESET);
    gpio_bit_set(DEBUG_PORT, debug_pin);
    delay_us(50);
    gpio_bit_reset(DEBUG_PORT, debug_pin);
    delay_us(50);
  }
}

void debug_port_num_code_hw(uint16_t data)
{
  delay_us(150);
  for (uint8_t i = 0; i < 16; i++)
  {
    gpio_bit_set(DEBUG_PORT, debug_pin);
    delay_us(50);
    gpio_bit_write(DEBUG_PORT, debug_pin,
                   (((data >> i) & 0x01) == 1)? GPIO_PIN_SET:GPIO_PIN_RESET);
    gpio_bit_set(DEBUG_PORT, debug_pin);
    delay_us(50);
    gpio_bit_reset(DEBUG_PORT, debug_pin);
    delay_us(50);
  }
}

void debug_port_code(uint8_t x, uint8_t y)
{
  // uint32_t time = 0;
  uint8_t temp_x = x+1;
  uint8_t temp_y = y+1;

  delay_ms(100);
  while (temp_x>0)
  {
    gpio_bit_set(DEBUG_PORT, debug_pin);
    gpio_bit_reset(DEBUG_PORT, debug_pin);
    temp_x--;
  }

  // while (time-- > 185000);
  delay_ms(10);

  while (temp_y > 0)
  {
    gpio_bit_set(DEBUG_PORT, debug_pin);
    gpio_bit_reset(DEBUG_PORT, debug_pin);
    temp_y--;
  }
}
