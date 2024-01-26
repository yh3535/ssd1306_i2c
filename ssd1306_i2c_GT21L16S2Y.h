#ifndef _SSD1306_I2C_GT21L16S2Y_H_
#define _SSD1306_I2C_GT21L16S2Y_H_
/*
�˿����ssd1306_i2c��Ϊssd1306_i2c���䲻ͬ�����С������ʾ���ܡ�
��ʹ��GB2312���뱣����ļ���
����GT21L16S2Y SPI�ӿ�оƬ�ĺ�����������
*/
// sig_ ���� ssd1306_i2c_GT21L16S2Y_
#include "ssd1306_i2c.h"
#include "hardware/spi.h"
//��ַ
// #define ASCII_5x7_Address           0x03BFC0    //5x7��ASCII�ַ�
// #define ASCII_7x8_Address           0x0066C0    //7x8��ASCII�ַ�
#define ASCII_1212_Address          0x066D40    //6x12��ASCII�ַ�
#define ASCII_1616_Address          0x03B7C0    //8x16��ASCII�ַ�
// #define Arial_ASCII_16_Address      0x03C2C0    //16���󲻵ȿ�ASCII��ͷ(Arial)�ַ�
// #define Arial_ASCII_12_Address      0x067340    //12���󲻵ȿ�ASCII��ͷ(Arial)�ַ�
#define GB2312_CN1212_Address      0x03CF80    //11x12��GB2312��׼�����ַ�
#define GB2312_CN1616_Address      0x000000    //15x16��GB2312��׼�����ֿ�
// #define Gb2312ToUnicodeBuff               0x002F00    //GB2312��Unicode����ת����
// #define UnicodeToGb2312Buff               0x067D70    //Unicode��GB2312����ת����

// ��������12x12����ӦӢ������6x12
// ��������16x16����ӦӢ������8x16
#define GT21L16S2Y_FONT_1212 1
#define GT21L16S2Y_FONT_1616 2

// ָ��
#define GT21L16S2Y_CMD_READ (0x3u)
typedef struct _sig_inst{
    //ʵ����Ҫһ��spi�˿���оƬ��������
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