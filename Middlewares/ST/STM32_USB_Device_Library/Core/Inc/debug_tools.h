//
// Created by liush on 2024/3/22.
//

#ifndef C8XX_MXCUBE_CLION_USBKEYBOARD_DEBUG_TOOLS_H
#define C8XX_MXCUBE_CLION_USBKEYBOARD_DEBUG_TOOLS_H

#include <stdint.h>

void debug_port_init(uint32_t GPIOF_PIN);
void debug_port_num(uint8_t n);
void debug_port_code(uint8_t x, uint8_t y);
void debug_port_num_code_hw(uint16_t data);
void debug_port_num_code_by(uint8_t data);

#endif //C8XX_MXCUBE_CLION_USBKEYBOARD_DEBUG_TOOLS_H
