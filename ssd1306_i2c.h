#ifndef _SSD1306_I2C_H_
#define _SSD1306_I2C_H_
/*
This driver is for ssd1306 OLED display. 
It may also be compatible with ssd1315 
which has the same functionality as ssd1306 
but different pin order.
It comes with a default serif 8x16 font
 which comes from SimSun by Microsoft.
The code is based on the official example
 "ssd1306_i2c" from Raspberry Pi Pico.
*/
#include "hardware/i2c.h"
#include "pico/stdlib.h"
/*
By default, the display height is set to 64
pixels and the display width is set to 128 pixels.
You can change the display height and width by 
defining them before including this file.
*/
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT 64
#endif

#define SSD1306_I2C_ADDR            _u(0x3C)

// #ifndef SSD1306_I2C_CLK
// #define SSD1306_I2C_CLK             400
// #endif
#define SSD1306_SET_MEM_MODE        _u(0x20)
#define SSD1306_SET_COL_ADDR        _u(0x21)
#define SSD1306_SET_PAGE_ADDR       _u(0x22)
#define SSD1306_SET_HORIZ_SCROLL    _u(0x26)
#define SSD1306_SET_SCROLL          _u(0x2E)

#define SSD1306_SET_DISP_START_LINE _u(0x40)

#define SSD1306_SET_CONTRAST        _u(0x81)
#define SSD1306_SET_CHARGE_PUMP     _u(0x8D)

#define SSD1306_SET_SEG_REMAP       _u(0xA0)
#define SSD1306_SET_ENTIRE_ON       _u(0xA4)
#define SSD1306_SET_ALL_ON          _u(0xA5)
#define SSD1306_SET_NORM_DISP       _u(0xA6)
#define SSD1306_SET_INV_DISP        _u(0xA7)
#define SSD1306_SET_MUX_RATIO       _u(0xA8)
#define SSD1306_SET_DISP            _u(0xAE)
#define SSD1306_SET_COM_OUT_DIR     _u(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)

#define SSD1306_SET_DISP_OFFSET     _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV    _u(0xD5)
#define SSD1306_SET_PRECHARGE       _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG     _u(0xDA)
#define SSD1306_SET_VCOM_DESEL      _u(0xDB)

#define SSD1306_PAGE_HEIGHT         _u(8)
#define SSD1306_NUM_PAGES           (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * SSD1306_WIDTH)

#define SSD1306_WRITE_MODE         _u(0xFE)
#define SSD1306_READ_MODE          _u(0xFF)

#define SSD1306_FONTS_WIDTH 8
#define SSD1306_FONTS_HEIGHT 16
#define SSD1306_FONTS_SIZE 16//how many bytes per character
typedef struct _ssd1306_i2c_instance{
    i2c_inst_t *i2c_port;
    uint16_t port_index[2];         //0 is scl, 1 is sda
    uint8_t buffer[SSD1306_HEIGHT*SSD1306_WIDTH/SSD1306_PAGE_HEIGHT+1]; //for 12864 screens, it is 8*128 bytes.
                                                    //Another byte is for 0x40 data flag.

}ssd1306_i2c_inst;

typedef struct _ssd1306_i2c_area {
    uint16_t x_interval[2];
    uint16_t page_interval[2];
    uint8_t *buffer;
    uint32_t buffer_size;
}ssd1306_i2c_area;

typedef uint8_t ssd1306_i2c_buf_t[SSD1306_WIDTH];

extern const uint8_t ssd1306_i2c_fonts[][16];

int  ssd1306_i2c_inst_init(ssd1306_i2c_inst *inst, i2c_inst_t * i2c_port,uint16_t port_scl,uint16_t port_sda, uint speed_khz);
void ssd1306_i2c_send_cmd(ssd1306_i2c_inst *inst, uint8_t cmd);
void ssd1306_i2c_send_cmd_list(ssd1306_i2c_inst *inst,uint8_t *buf, int num);
void ssd1306_i2c_send_buf(ssd1306_i2c_inst *inst,uint8_t buf[], int buflen);
void ssd1306_i2c_init(ssd1306_i2c_inst *inst);
void ssd1306_i2c_scroll(ssd1306_i2c_inst *inst, bool on);
void ssd1306_i2c_flush(ssd1306_i2c_inst *inst);
void ssd1306_i2c_bufferSetPixel(ssd1306_i2c_inst *inst, int x,int y, bool on);
void ssd1306_i2c_bufferDrawLine(ssd1306_i2c_inst *inst, int x0, int y0, int x1, int y1, bool on);
int  ssd1306_i2c_getFontIndex(uint8_t ch);
void ssd1306_i2c_bufferWriteChar(ssd1306_i2c_inst *inst, uint16_t x, uint16_t y, uint8_t ch);
void ssd1306_i2c_bufferWriteCString(ssd1306_i2c_inst *inst, uint16_t x, uint16_t y, const char *str);
void ssd1306_i2c_cmdInvert(ssd1306_i2c_inst *inst, bool inv);
void ssd1306_i2c_bufferInvert(ssd1306_i2c_inst *inst);
void ssd1306_i2c_bufferInvertArea(ssd1306_i2c_inst *inst, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);
void ssd1306_i2c_bufferInvertPageArea(ssd1306_i2c_inst *inst, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max);
void ssd1306_i2c_bufferClear(ssd1306_i2c_inst *inst);
void ssd1306_i2c_bufferClearArea(ssd1306_i2c_inst *inst, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);
void ssd1306_i2c_bufferClearPageArea(ssd1306_i2c_inst *inst, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max);
void ssd1306_i2c_bufferSet(ssd1306_i2c_inst *inst);
void ssd1306_i2c_bufferSetArea(ssd1306_i2c_inst *inst, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);
void ssd1306_i2c_bufferSetPageArea(ssd1306_i2c_inst *inst, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max);


int  ssd1306_i2c_area_init(ssd1306_i2c_area *area, int x_min, int x_max, int page_min, int page_max);
void ssd1306_i2c_area_write(ssd1306_i2c_inst *inst, ssd1306_i2c_area *area);
void ssd1306_i2c_area_copyData(ssd1306_i2c_area *area, const uint8_t *data, uint size);
void ssd1306_i2c_area_free(ssd1306_i2c_area *area);

void ssd1306_i2c_generalByteBitCopy(uint8_t *dest, uint d_start, const uint8_t *src, uint s_start, uint length);
void ssd1306_i2c_generalByteBitInvertCopy(uint8_t *dest, uint d_start, const uint8_t *src, uint s_start, uint length);
#endif