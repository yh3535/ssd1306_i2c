#ifndef _SSD1306_I2C_GT21L16S2Y_H_
#define _SSD1306_I2C_GT21L16S2Y_H_
/*
此库基于ssd1306_i2c，为ssd1306_i2c补充不同字体大小汉字显示功能。
请使用GB2312编码保存此文件。
基于GT21L16S2Y SPI接口芯片的汉字驱动程序。
*/
// sig_ 代表 ssd1306_i2c_GT21L16S2Y_
#include "ssd1306_i2c.h"
#include "hardware/spi.h"
//地址
// #define ASCII_5x7_Address           0x03BFC0    //5x7点ASCII字符
// #define ASCII_7x8_Address           0x0066C0    //7x8点ASCII字符
#define ASCII_1212_Address          0x066D40    //6x12点ASCII字符
#define ASCII_1616_Address          0x03B7C0    //8x16点ASCII字符
// #define Arial_ASCII_16_Address      0x03C2C0    //16点阵不等宽ASCII方头(Arial)字符
// #define Arial_ASCII_12_Address      0x067340    //12点阵不等宽ASCII方头(Arial)字符
#define GB2312_CN1212_Address      0x03CF80    //11x12点GB2312标准点阵字符
#define GB2312_CN1616_Address      0x000000    //15x16点GB2312标准点阵字库
// #define Gb2312ToUnicodeBuff               0x002F00    //GB2312到Unicode内码转换表
// #define UnicodeToGb2312Buff               0x067D70    //Unicode到GB2312内码转换表

// 中文字体12x12，对应英文字体6x12
// 中文字体16x16，对应英文字体8x16
#define GT21L16S2Y_FONT_1212 1
#define GT21L16S2Y_FONT_1616 2

// 指令
#define GT21L16S2Y_CMD_READ (0x3u)
typedef struct _sig_inst{
    //实例需要一个spi端口与芯片传输数据
    ssd1306_i2c_inst *ssd1306;
    spi_inst_t *spi_port;
    uint8_t port_index[4];// 0 is CS#, 1 is RX (MISO), 2 is TX (MOSI), 3 is SCLK
    uint32_t font;

}sig_inst;


void        sig_inst_init(sig_inst *inst, ssd1306_i2c_inst* ssd1306, spi_inst_t *spi_port, uint8_t port_index[], uint32_t font, uint32_t speed_khz);
void        sig_cs_select(sig_inst *inst);
void        sig_cs_deselect(sig_inst *inst);
uint32_t    sig_getHanziAddressWord(sig_inst *inst, const char *hanzi);
void        sig_getHanziData(sig_inst * inst, const char *hanzi, uint8_t * data);
void        sig_bufferWriteHanzi(sig_inst *inst, const char *hanzi, uint32_t x, uint32_t y);
void        sig_bufferWriteHString(sig_inst * inst, const char *str, size_t len, uint32_t x, uint32_t y);



#endif /* _SSD1306_I2C_GT21L16S2Y_H_ */