//
// Created by liush on 2024/3/21.
//

#include "middle_interfac.h"

uint8_t key_buffer[8] = {0};

void gpio_port_write(GPIO_TypeDef *GPIOx, uint16_t data)
{
  GPIOx->ODR = data;
}
uint16_t gpio_output_port_get(GPIO_TypeDef *GPIOx)
{
  return (uint16_t)GPIOx->ODR;
}
void gpio_bit_reset(GPIO_TypeDef *GPIOx, uint32_t gpio_pin)
{
  HAL_GPIO_WritePin(GPIOx, gpio_pin, GPIO_PIN_RESET);
}
void gpio_bit_set(GPIO_TypeDef *GPIOx, uint32_t gpio_pin)
{
  HAL_GPIO_WritePin(GPIOx, gpio_pin, GPIO_PIN_SET);
}
uint16_t gpio_input_port_get(GPIO_TypeDef *GPIOx)
{
  return (uint16_t)GPIOx->IDR;
}
void delay_ms(uint16_t ms)
{
  if (ms > 1)
  {
    ms -= 1;
  }
  HAL_Delay(ms);
}

void delay_us(int us)
{
  uint32_t now_count = SysTick->VAL;
  uint32_t delay = us * 72;
  uint32_t target_count;

  if (now_count >= delay)
  {
    target_count = now_count - delay;
  }
  else
  {
    while (!((SysTick->CTRL>>16) & 1));
    target_count = 72000-1+now_count-delay;
  }

  while (SysTick->VAL > target_count);
}

//  0/1: different/same
uint8_t buffer_cmp(const uint8_t* buffer)
{
  uint8_t eq_flag = 1;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (key_buffer[i] != buffer[i])
    {
      eq_flag = 0;
      break;
    }
  }

  return eq_flag;
}
uint8_t* get_key_buffer()
{
  return key_buffer;
}

void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
  if (uwTick % 100 == 0)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
}

void gpio_bit_write(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
}

uint8_t gpio_input_bit_get(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  GPIO_PinState pin_state =  HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);

  return pin_state==GPIO_PIN_SET? 0x01:0x00;
}
