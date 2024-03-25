//
// Created by liush on 2024/3/21.
//

#ifndef C8XX_MXCUBE_CLION_USBKEYBOARD_KEYBOARD_H
#define C8XX_MXCUBE_CLION_USBKEYBOARD_KEYBOARD_H

typedef enum {
    FALSE = 0,
    TRUE  = 1
} bool;

#define MX_ROW_COUNT 8
#define MX_COL_COUNT 18
#define ROW_OFFSET 2

void scan_keyboard(void);

#endif //C8XX_MXCUBE_CLION_USBKEYBOARD_KEYBOARD_H
