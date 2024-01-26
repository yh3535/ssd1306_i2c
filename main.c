#include <stdio.h>

#include "ssd1306_i2c.h"
#include "ssd1306_i2c_GT21L16S2Y.h"
#include "stdlib.h"
#include "hardware/spi.h"
// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

const uint8_t data[][16]={
    {0x00,0x00,0xC0,0xE0,0xF0,0xF0,0xF8,0xF8,0xFC,0xFC,0xFE,0xFE,0xFE,0xFC,0xF8,0xF8},
    {0xF0,0xF0,0xE0,0xC0,0x00,0x00,0x00,0xC0,0xE0,0xE0,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF},
    {0xFF,0xFF,0xFF,0xFE,0xE0,0xE0,0xC0,0x00,0x00,0x00,0x00,0x01,0x03,0x03,0x07,0x07},
    {0x0F,0x1F,0x1F,0x1F,0x1F,0x0F,0x0F,0x07,0x03,0x03,0x01,0x00,0x00,0x00,0x00,0x01},
    {0x03,0x07,0x0F,0x1F,0x3F,0x3F,0x7F,0x7F,0x3F,0x3F,0x1F,0x0F,0x07,0x03,0x01,0x00},
};



int main()
{
    stdio_init_all();

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 1);
    puts("Hello, world!");

    ssd1306_i2c_inst ssd1306;
    ssd1306_i2c_inst_init(&ssd1306,i2c0,I2C_SCL,I2C_SDA,400);
    ssd1306_i2c_init(&ssd1306);
    // for (int i = 0; i < 3; i++) {
    //     ssd1306_i2c_send_cmd(&ssd1306,SSD1306_SET_ALL_ON);    // 所有像素点亮
    //     sleep_ms(500);
    //     ssd1306_i2c_send_cmd(&ssd1306,SSD1306_SET_ENTIRE_ON); // 回到RAM状态
    //     sleep_ms(500);
    // }
    ssd1306_i2c_drawLine(&ssd1306,10,10,127,63,1);
    ssd1306_i2c_bufferWriteChar(&ssd1306,8,3,'z');
    ssd1306_i2c_bufferWriteCString(&ssd1306,0,0,"hello world\nthis is a test character output.");
    ssd1306_i2c_flush(&ssd1306);
    int i=0;
    // while (1)
    // {
    //     /* code */
    //     char str[100];
    //     sprintf(str,"%d",i);
    //     ssd1306_i2c_bufferWriteCString(&ssd1306,0,47,str);
    //     ssd1306_i2c_flush(&ssd1306);
    //     i++;
    //     sleep_ms(1000);
        
    // }
    ssd1306_i2c_area area;
    ssd1306_i2c_area_init(&area,0,39,6,7);
    ssd1306_i2c_area_copyData(&area,(const uint8_t*)data,sizeof(data));
    ssd1306_i2c_area_write(&ssd1306,&area);
    ssd1306_i2c_area_free(&area);
    ssd1306_i2c_invertPageArea(&ssd1306,0,127,0,1);
    ssd1306_i2c_flush(&ssd1306);
    sleep_ms(1000);
    uint y=0;
    while(y<48)
    {
        ssd1306_i2c_invertArea(&ssd1306,0,127,y,y+15);
        y++;
        ssd1306_i2c_invertArea(&ssd1306,0,127,y,y+15);
        ssd1306_i2c_flush(&ssd1306);

        sleep_ms(abs(24-y)/3.0);
    }


    sig_inst sig;
    uint8_t p[]={17,16,19,18};
    sig_inst_init(&sig, &ssd1306, spi0, p,  GT21L16S2Y_FONT_1212, 5000);
    sig_bufferWriteHanzi(&sig, "不", 0, 52);
    sig_bufferWriteHString(&sig, "不是他干的是另一个人干的哈哈哈",15, 4, 40);


    ssd1306_i2c_flush(&ssd1306);
    return 0;
}
