#include <stdio.h>

#include "ssd1306_i2c.h"
#include "ssd1306_i2c_GT21L16S2Y.h"
#include "ssd1306_i2c_frame.h"
#include "stdlib.h"
#include "hardware/spi.h"
// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

const uint8_t data[][16] = {
    {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF0, 0xF8, 0xF8, 0xFC, 0xFC, 0xFE, 0xFE, 0xFE, 0xFC, 0xF8, 0xF8},
    {0xF0, 0xF0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xE0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFE, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07, 0x07},
    {0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0F, 0x0F, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x3F, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00},
};

int main()
{
    stdio_init_all();

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 1);
    puts("Hello, world!");

    ssd1306_i2c_inst ssd1306;
    ssd1306_i2c_inst_init(&ssd1306, i2c0, I2C_SCL, I2C_SDA, 400);
    ssd1306_i2c_init(&ssd1306);
    // for (int i = 0; i < 3; i++) {
    //     ssd1306_i2c_send_cmd(&ssd1306,SSD1306_SET_ALL_ON);    // 所有像素点亮
    //     sleep_ms(500);
    //     ssd1306_i2c_send_cmd(&ssd1306,SSD1306_SET_ENTIRE_ON); // 回到RAM状态
    //     sleep_ms(500);
    // }
    ssd1306_i2c_drawLine(&ssd1306, 10, 10, 127, 63, 1);
    ssd1306_i2c_bufferWriteChar(&ssd1306, 8, 3, 'z');
    ssd1306_i2c_bufferWriteCString(&ssd1306, 0, 0, "hello world\nthis is a test character output.");
    ssd1306_i2c_flush(&ssd1306);
    int i = 0;
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
    ssd1306_i2c_area_init(&area, 0, 39, 6, 7);
    ssd1306_i2c_area_copyData(&area, (const uint8_t *)data, sizeof(data));
    ssd1306_i2c_area_write(&ssd1306, &area);
    ssd1306_i2c_area_free(&area);
    ssd1306_i2c_bufferInvertPageArea(&ssd1306, 0, 127, 0, 1);
    ssd1306_i2c_flush(&ssd1306);
    sleep_ms(1000);
    uint y = 0;
    while (y < 48)
    {
        ssd1306_i2c_bufferInvertArea(&ssd1306, 0, 127, y, y + 15);
        y++;
        ssd1306_i2c_bufferInvertArea(&ssd1306, 0, 127, y, y + 15);
        ssd1306_i2c_flush(&ssd1306);

        sleep_ms(abs(24 - y) / 3.0);
    }

    sig_inst sig;
    uint8_t p[] = {17, 16, 19, 18};
    sig_inst_init(&sig, &ssd1306, spi0, p, GT21L16S2Y_FONT_1616, 5000);
    // sig_bufferWriteHanzi(&sig, "不", 0, 52);
    // sig_bufferWriteHString(&sig, "不是他干的是另一个人干的哈哈哈",15, 4, 40);
    // sig_bufferWriteAscii(&sig, "b", 64, 48);
    // sig_bufferWriteAscii(&sig, "B", 76, 48);
    // sig_bufferWriteString(&sig, "这不是他的\n这是woden的，你不该在这里", 0, 0);
    // ssd1306_i2c_flush(&ssd1306);
    // sleep_ms(1000);
    // ssd1306_i2c_bufferClearArea(&ssd1306, 0, 127, 48, 63);
    // ssd1306_i2c_flush(&ssd1306);
    // sleep_ms(1000);

    // ssd1306_i2c_bufferClear(&ssd1306);
    // sig_bufferWriteString(&sig, "DSP时间测试\n长度：4096\n时间：256 ms", 0, 0);
    // ssd1306_i2c_flush(&ssd1306);
    // sleep_ms(2000);
    sig.font = GT21L16S2Y_FONT_1212;
    ssd1306_i2c_bufferClear(&ssd1306);
    sig_bufferWriteString(&sig, "12像素长度字体测试\nThis is the 12x12 font. Is it OK?\n谷歌病毒桶是真tmd离谱啊 coloros不自带play商店。我安装了一个，重启后play商店变系统应用无法卸载了", 0, 0);
    ssd1306_i2c_flush(&ssd1306);

    sleep_ms(1000);
    ssd1306_i2c_bufferClear(&ssd1306);
    ssd1306_i2c_flush(&ssd1306);

    si_frame f;
    si_frame_init(&f, 32, 32);
    si_frame_writeCString(&f, 0, 0, "hello");
    si_frame_invert(&f);
    // si_frame_invertPageArea(&f,0,15,0,3);
    // si_frame_invertArea(&f,0,7,4,25);
    // for (int x = 0; x < SSD1306_WIDTH - 32; x+=4)
    // {
    //     for (int y = 0; y < SSD1306_HEIGHT - 32; y++)
    //     {
    //         ssd1306_i2c_bufferClear(&ssd1306);
    //         ssd1306_i2c_bufferPutFrame(&ssd1306, x, y, &f);
    //         ssd1306_i2c_flush(&ssd1306);
    //         sleep_ms(50);
    //     }
    // }

    ssd1306_i2c_bufferClear(&ssd1306);
    ssd1306_i2c_flush(&ssd1306);
    si_frame f1;
    si_frame_init(&f1, 64,36);

    //sig.font = GT21L16S2Y_FONT_1616;
    sifg_writeString(&f1,&sig,"测试test字符串\nhello world",0,0);
    si_frame_drawLine(&f1,0,0,0,f1.height-1,1);
    si_frame_drawLine(&f1,0,0,f1.width-1,0,1);
    si_frame_drawLine(&f1,0,f1.height-1,f1.width-1,f1.height-1,1);
    si_frame_drawLine(&f1,f1.width-1,0,f1.width-1,f1.height-1,1);
    ssd1306_i2c_bufferPutFrame(&ssd1306,0,0,&f1);
    ssd1306_i2c_flush(&ssd1306);
    for (int x = 0; x < SSD1306_WIDTH - f1.width; x+=4)
    {
        for (int y = 0; y < SSD1306_HEIGHT - f1.height; y++)
        {
            ssd1306_i2c_bufferClear(&ssd1306);
            ssd1306_i2c_bufferPutFrame(&ssd1306, x, y, &f1);
            ssd1306_i2c_flush(&ssd1306);
            sleep_ms(50);
        }
    }
    return 0;
}
