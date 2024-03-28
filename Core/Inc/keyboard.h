//
// Created by liush on 2024/3/21.
//

#ifndef C8XX_MXCUBE_CLION_USBKEYBOARD_KEYBOARD_H
#define C8XX_MXCUBE_CLION_USBKEYBOARD_KEYBOARD_H

#define MX_ROW_COUNT    8
#define MX_COL_COUNT    18
#define ROW_OFFSET      2
#define FN_KEY_COUNT    10

#define KEY_F10         0x43
#define KEY_F11         0x44
#define KEY_F12         0x45
#define KEY_PRT_SC      0x46
#define KEY_INS         0x49
#define KEY_DEL         0x4c
#define KEY_UA          0x52
#define KEY_DA          0x51
#define KEY_LA          0x50
#define KEY_RA          0x4f

typedef enum {
    FALSE = 0,
    TRUE  = 1
} bool;

void scan_keyboard(void);

#endif //C8XX_MXCUBE_CLION_USBKEYBOARD_KEYBOARD_H
