#include "keyboard.h"
#include "usbd_hid.h"
#include "middle_interfac.h"

#define BUFFER_SIZE 8
#define EP_ADDR_NOM 0x81
#define EP_ADDR_FN 0x82

#define DEV 0

/*      PB0  PB1  PB2  PB3  PB4  PB5  PB6  PB7  PB8  PB9  PB10  PB11  PB12  PB13  PB14  PB15  PA0  PA1
 * PA2
 * PA3
 * PA4
 * PA5
 * PA6
 * PA7
 * PA8
 * PA9
 */

/*
 * 需求: 音量-(f10 EA)
 *      音量+(f11 E9)
 *      静音(f12 E2)
 *      SysReq(PrtSc 9A)
 *      Scroll Lock(Ins 发送键码)
 *      Pause(Del B1)
 *      Page Up(uA 发送键码)
 *      Page Down(dA 发送键码)
 *      Home(lA 发送键码)
 *      End(rA 发送键码)
 */

static uint8_t gs_phy_mx[MX_ROW_COUNT][MX_COL_COUNT] = {
        {2,   3,   4,   5,  8,  9,   10,  81,  86,  0,  0,  0,  121, 11, 124, 116, 0,   0},
        {1,   112, 113, 6,  7,  13,  119, 80,  85,  75, 76, 0,  120, 12, 0,   58,  0,   0},
        {131, 132, 133, 50, 51, 56,  129, 79,  105, 89, 84, 0,  123, 55, 62,  0,   0,   0},
        {46,  47,  48,  49, 52, 53,  54,  0,   100, 95, 90, 0,  43,  42, 0,   64,  0,   0},
        {110, 45,  115, 35, 36, 117, 0,   83,  104, 99, 61, 0,  122, 41, 60,  0,   0,   0},
        {31,  32,  33,  34, 37, 38,  39,  108, 103, 98, 93, 57, 29,  40, 59,  0,   0,   128},
        {16,  30,  114, 21, 22, 28,  118, 107, 102, 97, 92, 44, 15,  27, 0,   0,   127, 0},
        {17,  18,  19,  20, 23, 24,  25,  106, 101, 96, 91, 0,  14,  26, 125, 126, 0,   0}
};

static uint8_t gs_phy_to_keycode[144] = {
        0, 0x35, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
        0x26, 0x27, 0x2d, 0x2e, 0, 0x2a, 0x2b, 0x14, 0x1a,
        0x08, 0x15, 0x17, 0x1c, 0x18, 0x0c, 0x12, 0x13, 0x2f,
        0x30, 0x31, 0x39, 0x04, 0x16, 0x07, 0x09, 0x0a, 0x0b,
        0x0d, 0x0e, 0x0f, 0x33, 0x34, 0, 0x28, 0xe1, 0, 0x1d,
        0x1b, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0,
        0xe5, 0xe0, 0xff, 0xe2, 0x2c, 0xe6, 0, 0xe4, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0x49, 0x4c, 0, 0, 0x50, 0, 0, 0,
        0x52, 0x51, 0, 0, 0, 0, 0x4f, 0x53, 0x5f, 0x5c, 0x59, 0,
        0x54, 0x60, 0x5d, 0x5a, 0x62, 0x55, 0x61, 0x5e, 0x5b,
        0x63, 0x56, 0x57, 0, 0x2a, 0, 0x29, 0, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
        0x46, 0, 0, 0xe3, 0, 0x65, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0
};

static uint8_t fn_key[FN_KEY_COUNT] = {
KEY_F10   ,
KEY_F11   ,
KEY_F12   ,
KEY_PRT_SC,
KEY_DEL   ,
KEY_INS   ,
KEY_UA    ,
KEY_DA    ,
KEY_LA    ,
KEY_RA    ,
};

static volatile bool gs_ghosting_flag = FALSE;
static volatile bool gs_fn_key_flag = FALSE;

typedef struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint8_t key_count;
    uint8_t normal_key_count;
} buffer_t;

extern USBD_HandleTypeDef hUsbDeviceFS;

//  接收输入按键，用于验证合法性
static uint8_t gs_mx_input_key_buffer[MX_ROW_COUNT][MX_COL_COUNT] = {0};
static uint8_t gs_mx_input_key_buffer_count = 0;

static uint32_t gs_input_key_buffer[MX_ROW_COUNT] = {0};

//  作为实际发送的key buffer的缓冲
static buffer_t gs_temp_key_buffer = {.buffer = {0}, .key_count = 0, .normal_key_count = 0};

static void handle_input_data(uint8_t row_inx, uint32_t gpio_input_data);

static void handle_original_code(uint8_t row_code, uint8_t col_code);

static bool is_ghosting(uint8_t row_code, uint8_t col_code);

static uint32_t get_col_data(void);

static void handle_fn_key(void);

/*!
    \brief      scan the keyboard matrix
    \param[in]  none
    \param[out] none
    \retval     none
*/
void scan_keyboard(void)
{
    gpio_port_write(GPIOA, (gpio_output_port_get(GPIOA) | 0x03fc));

    while (1)
    {
        uint32_t col_data = 0x00000000;
        gs_ghosting_flag = FALSE;
        gs_fn_key_flag = FALSE;

        /*
            这里增加判断，当本轮扫描出现冲突时，停止扫描以提高效率，但是每次循环
            开始前，会重置标志位，所以不影响下一次扫描
        */
        for (uint8_t row_inx = ROW_OFFSET; row_inx < (ROW_OFFSET + MX_ROW_COUNT); row_inx++)
        {
            //  逐行扫描
            gpio_bit_reset(GPIOA, GPIO_PIN(row_inx));

            //  获取当前的col输入
            col_data = get_col_data();
            //  处理col data
            handle_input_data(row_inx, col_data);

            gpio_bit_set(GPIOA, GPIO_PIN(row_inx));
        }

        //  处理fn按键
        handle_fn_key();

        if ((gs_ghosting_flag == FALSE) && (buffer_cmp(gs_temp_key_buffer.buffer) == 0))
        {
            memcpy(get_key_buffer(), gs_temp_key_buffer.buffer, 8);
            if ((gs_fn_key_flag && (gs_temp_key_buffer.key_count == 2)))
            {
                USBD_HID_SendReport(&hUsbDeviceFS, get_key_buffer(), 4U, EP_ADDR_FN);
            }
            else
            {
                USBD_HID_SendReport(&hUsbDeviceFS, get_key_buffer(), 8U, EP_ADDR_NOM);
            }
        }


        memset(gs_mx_input_key_buffer, 0, sizeof(gs_mx_input_key_buffer));
        gs_mx_input_key_buffer_count = 0;


        memset(gs_temp_key_buffer.buffer, 0, BUFFER_SIZE);
        gs_temp_key_buffer.key_count = 0;
        gs_temp_key_buffer.normal_key_count = 0;

    }
}

/********************************************** 处理扫描的行数据 *************************************************/

void handle_input_data(uint8_t row_inx, uint32_t gpio_input_data)
{
    gpio_input_data = ~gpio_input_data;

    if (gpio_input_data != gs_input_key_buffer[row_inx - ROW_OFFSET])
    {
        //  消抖
        delay_ms(50);
        if ((gpio_input_data ^ get_col_data()) == 0xffffffff)
        {
            gs_input_key_buffer[row_inx - ROW_OFFSET] = gpio_input_data;
        } else
        {
            gpio_input_data = gs_input_key_buffer[row_inx - ROW_OFFSET];
        }
    }

    if (gpio_input_data == 0xfffc0000)
    {
        return;
    }

    gpio_input_data &= 0x0003ffff;
    for (uint8_t col_inx = 0; col_inx < MX_COL_COUNT; col_inx++)
    {
        if ((gpio_input_data & 0x0001) == 0x0001)
        {
            handle_original_code(row_inx, col_inx);
        }

        gpio_input_data >>= 1;
        if (gpio_input_data == 0)
        {
            break;
        }
    }
}

/******************************************** 处理得出的物理键码数据 ***********************************************/

static void handle_original_code(uint8_t row_code, uint8_t col_code)
{
    if (is_ghosting(row_code - ROW_OFFSET, col_code) == FALSE)
    {
        gs_mx_input_key_buffer[row_code - ROW_OFFSET][col_code] = 1;
        gs_mx_input_key_buffer_count++;

        /* 得出HID键码 */
        uint8_t key_code = gs_phy_to_keycode[gs_phy_mx[row_code - ROW_OFFSET][col_code]];
        if (key_code == 0xff)
        {
            gs_fn_key_flag = TRUE;
        }
        uint8_t key_code_row = (key_code >> 4);
        uint8_t key_code_col = (key_code & 0x0f);
        if (key_code_row == 0x0e)
        {
            gs_temp_key_buffer.buffer[0] |= (0x01 << key_code_col);
        } else
        {
            gs_temp_key_buffer.buffer[gs_temp_key_buffer.normal_key_count + 2] = key_code;
            gs_temp_key_buffer.normal_key_count += 1;
        }
        gs_temp_key_buffer.key_count++;

    } else
    {
        gs_ghosting_flag = TRUE;
    }
}

static bool is_ghosting(uint8_t row_code, uint8_t col_code)
{
    if (gs_mx_input_key_buffer_count < 2)
    {
        return FALSE;
    }

    if (gs_temp_key_buffer.key_count > 14 || gs_temp_key_buffer.normal_key_count > 6)
    {
        return TRUE;
    }

    //  判断是否有同列
    for (uint8_t i = 0; i < row_code; i++)
    {
        if (gs_mx_input_key_buffer[i][col_code] == 1)
        {
            for (uint8_t j = 0; j < MX_COL_COUNT; j++)
            {
                if (j == col_code)
                {
                    continue;
                }
                if ((gs_mx_input_key_buffer[i][j] == 1) || (gs_mx_input_key_buffer[row_code][j] == 1))
                {
                    return TRUE;
                }
            }
        }
    }

    //  判断是否有同行
    for (uint8_t i = 0; i < col_code; i++)
    {
        if (gs_mx_input_key_buffer[row_code][i] == 1)
        {
            for (uint8_t j = 0; j < row_code; j++)
            {
                if ((gs_mx_input_key_buffer[j][i] == 1) || (gs_mx_input_key_buffer[j][col_code] == 1))
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

/********************************************** 工具函数 **************************************************/

static uint32_t get_col_data(void)
{
    uint32_t col_data = gpio_input_port_get(GPIOB);
    col_data |= (((uint32_t) gpio_input_bit_get(GPIOA, GPIO_PIN(0))) << 16);
    col_data |= (((uint32_t) gpio_input_bit_get(GPIOA, GPIO_PIN(1))) << 17);

    return col_data;
}

static void handle_fn_key(void)
{
    if (gs_fn_key_flag)
    {
        uint8_t tp = 0;
        uint8_t mp = 8;
        uint8_t temp_key = 0xfe;
        uint8_t temp_key_p = 11;

        for (uint8_t i = 0; i < FN_KEY_COUNT; ++i)
        {
            tp = find_buffer(gs_temp_key_buffer.buffer, fn_key[i]);
            if ((tp < mp) && (tp > 1))
            {
                mp = tp;
                temp_key = fn_key[i];
                temp_key_p = i;
            }
        }

        if ((temp_key != 0xfe) && (temp_key_p != 11))
        {
            memset(gs_temp_key_buffer.buffer, 0, BUFFER_SIZE);

            //  发送特殊报文
            if (temp_key_p < 5)
            {
                gs_temp_key_buffer.key_count = 2;
                gs_temp_key_buffer.normal_key_count = 2;
                gs_temp_key_buffer.buffer[0] = 0x02;
                gs_temp_key_buffer.buffer[1] = (1 << temp_key_p);
            }
            //  发送普通键码
            else
            {
                gs_temp_key_buffer.key_count = 1;
                gs_temp_key_buffer.normal_key_count = 1;
                gs_temp_key_buffer.buffer[0] = 0x00;
                gs_temp_key_buffer.buffer[1] = 0x00;
                switch (fn_key[temp_key_p])
                {
                    case KEY_INS:
                        gs_temp_key_buffer.buffer[2] = 0x47;
                        break;
                    case KEY_UA:
                        gs_temp_key_buffer.buffer[2] = 0x4b;
                        break;
                    case KEY_DA:
                        gs_temp_key_buffer.buffer[2] = 0x4e;
                        break;
                    case KEY_LA:
                        gs_temp_key_buffer.buffer[2] = 0x4a;
                        break;
                    case KEY_RA:
                        gs_temp_key_buffer.buffer[2] = 0x4d;
                        break;
                    default:
                        break;
                }
            }
        }
        else
        {
            gs_temp_key_buffer.buffer[find_buffer(gs_temp_key_buffer.buffer, 0xff)] = 0x00;
            gs_temp_key_buffer.key_count -= 1;
            gs_temp_key_buffer.normal_key_count -= 1;
            gs_fn_key_flag = FALSE;
        }
    }
}

void led_handler(uint8_t data_fragment)
{
    /*
     * PA10 scrlk
     * PF6  numlk
     * PF7  caplk
     */
    handle_led_gpio(data_fragment);
}