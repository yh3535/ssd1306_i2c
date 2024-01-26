
#include "ssd1306_i2c.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"

int ssd1306_i2c_inst_init(ssd1306_i2c_inst *inst, i2c_inst_t *i2c_port, uint16_t port_scl, uint16_t port_sda, uint speed_khz)
{
    inst->i2c_port = i2c_port;
    inst->port_index[0] = port_scl;
    inst->port_index[1] = port_sda;
    memset(inst->buffer, 0, sizeof(inst->buffer));
    inst->buffer[0] = 0x40;
    i2c_init(i2c_port, speed_khz * 1000);
    gpio_set_function(port_sda, GPIO_FUNC_I2C);
    gpio_set_function(port_scl, GPIO_FUNC_I2C);
    gpio_pull_up(port_sda);
    gpio_pull_up(port_scl);
    return 0;
}

void ssd1306_i2c_send_cmd(ssd1306_i2c_inst *inst, uint8_t cmd)
{
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(inst->i2c_port, SSD1306_I2C_ADDR, buf, 2, false);
}

void ssd1306_i2c_send_cmd_list(ssd1306_i2c_inst *inst, uint8_t *buf, int num)
{
    for (int i = 0; i < num; i++)
        ssd1306_i2c_send_cmd(inst, buf[i]);
}

void ssd1306_i2c_send_buf(ssd1306_i2c_inst *inst, uint8_t buf[], int buflen)
{
    uint8_t *temp_buf = malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf + 1, buf, buflen);

    i2c_write_blocking(inst->i2c_port, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);

    free(temp_buf);
}

void ssd1306_i2c_init(ssd1306_i2c_inst *inst)
{
    // 这里部分命令不是必须的，因为默认设置就是这样。但为了说明初始化序列是什么样的，还是都写在这里。
    // 某些值是硬件制造商推荐的。

    uint8_t cmds[] = {
        SSD1306_SET_DISP, // 关闭显示
        /* 内存映射 */
        SSD1306_SET_MEM_MODE, // 下个字节设置寻址模式： 0 = 水平, 1 = 垂直, 2 = 页
        0x00,                 // 水平寻址模式
        /* resolution and layout */
        SSD1306_SET_DISP_START_LINE,    // 设置开始行号为 0
        SSD1306_SET_SEG_REMAP | 0x01,   // 设置段重映射，列地址127被映射到第0段
        SSD1306_SET_MUX_RATIO,          // 设置多路复用率
        SSD1306_HEIGHT - 1,             // 显示屏高度 - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // 设置常规输出扫描方向，从下到上，COM[N-1]到COM[0]
        SSD1306_SET_DISP_OFFSET,        // 下个字节设置显示偏移量
        0x00,                           // 0偏移
        SSD1306_SET_COM_PIN_CFG,        // 下个字节设置常规引脚硬件配置，板子指定的magic number。
                                        // 0x02 用于 128x32, 0x12 可能用于 128x64。 其他选项有 0x22, 0x32
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
        0x02,
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
        0x12,
#else
        0x02,
#endif
        /* 计时和驱动模式 */
        SSD1306_SET_DISP_CLK_DIV, // 下个字节设置显示时钟分频系数
        0x80,                     // 系数为1，标准频率
        SSD1306_SET_PRECHARGE,    // 设置预充周期
        0xF1,                     // Vcc 由我们的板子内部产生
        SSD1306_SET_VCOM_DESEL,   // 下个字节设置高电平最低值
        0x30,                     // 0.83倍Vcc
        /* display */
        SSD1306_SET_CONTRAST, // 设置对比度控制
        0xFF,
        SSD1306_SET_ENTIRE_ON,     // 开启整个显示屏，以显示RAM内容
        SSD1306_SET_NORM_DISP,     // 常规显示而非反转
        SSD1306_SET_CHARGE_PUMP,   // 设置电源
        0x14,                      // Vcc 由我们的板子内部产生
        SSD1306_SET_SCROLL | 0x00, // 取消水平滚动。如果允许滚动，内存写入就会出错。
        SSD1306_SET_DISP | 0x01,   // 开启显示
    };

    ssd1306_i2c_send_cmd_list(inst, cmds, count_of(cmds));
    memset(inst->buffer, 0, sizeof(inst->buffer));
    inst->buffer[0] = 0x40;
    ssd1306_i2c_flush(inst);
}

void ssd1306_i2c_scroll(ssd1306_i2c_inst *inst, bool on)
{
    // 配置水平滚动
    uint8_t cmds[] = {
        SSD1306_SET_HORIZ_SCROLL | 0x00,
        0x00,                                // 空字节
        0x00,                                // 从第0页开始
        0x00,                                // 时间间隔
        0x03,                                // 到第3页结束 SSD1306_NUM_PAGES ??
        0x00,                                // 空字节
        0xFF,                                // 空字节
        SSD1306_SET_SCROLL | (on ? 0x01 : 0) // 开始/关闭滚动
    };

    ssd1306_i2c_send_cmd_list(inst, cmds, count_of(cmds));
}

void ssd1306_i2c_flush(ssd1306_i2c_inst *inst)
{
    // write data in buffer to display
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        0,
        SSD1306_WIDTH - 1,
        SSD1306_SET_PAGE_ADDR,
        0,
        SSD1306_NUM_PAGES - 1};

    ssd1306_i2c_send_cmd_list(inst, cmds, count_of(cmds));
    i2c_write_blocking(inst->i2c_port, SSD1306_I2C_ADDR, inst->buffer, sizeof(inst->buffer), false);
}

void ssd1306_i2c_setPixel(ssd1306_i2c_inst *inst, int x, int y, bool on)
{
    assert(x >= 0 && x < SSD1306_WIDTH && y >= 0 && y < SSD1306_HEIGHT);
    if (!(x >= 0 && x < SSD1306_WIDTH && y >= 0 && y < SSD1306_HEIGHT))
    {
        return;
    }
    ssd1306_i2c_buf_t *buf = (ssd1306_i2c_buf_t *)(inst->buffer + 1); // the first byte is 0x40
    uint8_t *pbyte = &(buf[y / SSD1306_PAGE_HEIGHT][x]);

    if (on)
        *pbyte |= 1 << (y % SSD1306_PAGE_HEIGHT); // y%SSD1306_PAGE_HEIGHT是一列8个比特中具体哪个比特
    else
        *pbyte &= ~(1 << (y % SSD1306_PAGE_HEIGHT));
}

void ssd1306_i2c_drawLine(ssd1306_i2c_inst *inst, int x0, int y0, int x1, int y1, bool on)
{

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true)
    {

        if (y0 == 20)
        {
            y0 = y0;
        }
        ssd1306_i2c_setPixel(inst, x0, y0, on);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

int ssd1306_i2c_getFontIndex(uint8_t ch)
{
    if (ch >= 32 && ch <= 126)
        return ch - ' ';
    else
        return 0;
}

void ssd1306_i2c_bufferWriteChar(ssd1306_i2c_inst *inst, int16_t x, int16_t y, uint8_t ch)
{
    //TODO: Rewrite this function using the new general_byte_bit_copy.
    if (x > SSD1306_WIDTH - SSD1306_FONTS_WIDTH || y > SSD1306_HEIGHT - SSD1306_FONTS_HEIGHT)
        return;
    ssd1306_i2c_buf_t *buf = (ssd1306_i2c_buf_t *)(inst->buffer + 1);
    int16_t page_y = y / SSD1306_PAGE_HEIGHT;
    int16_t beyond_page_y = y % SSD1306_PAGE_HEIGHT;
    if (y % SSD1306_PAGE_HEIGHT == 0)
    {
        for (int i = 0; i < SSD1306_FONTS_SIZE; i++)
        {
            buf[page_y + i / SSD1306_FONTS_WIDTH][x + i % SSD1306_FONTS_WIDTH] = ssd1306_i2c_fonts[ssd1306_i2c_getFontIndex(ch)][i];
        }
    }
    else
    {
        const uint8_t *chrp = ssd1306_i2c_fonts[ssd1306_i2c_getFontIndex(ch)];
        for (int i = 0; i < SSD1306_FONTS_SIZE / 2; i++)
        {

            buf[page_y][x + i % SSD1306_FONTS_WIDTH] &= 0xff >> (SSD1306_PAGE_HEIGHT - beyond_page_y);
            buf[page_y][x + i % SSD1306_FONTS_WIDTH] |=
                (chrp[i] << beyond_page_y);

            buf[page_y + 1][x + i % SSD1306_FONTS_WIDTH] =
                (chrp[i] >> (SSD1306_PAGE_HEIGHT - beyond_page_y)) | (chrp[i + SSD1306_FONTS_WIDTH] << beyond_page_y);

            buf[page_y + 2][x + i % SSD1306_FONTS_WIDTH] &= 0xff << beyond_page_y;
            buf[page_y + 2][x + i % SSD1306_FONTS_WIDTH] |=
                (chrp[i + SSD1306_FONTS_WIDTH] >> (SSD1306_PAGE_HEIGHT - beyond_page_y));
        }
    }
}

void ssd1306_i2c_bufferWriteCString(ssd1306_i2c_inst *inst, int16_t x, int16_t y, const char *str)
{
    // To write ASCII string.

    for (int i = 0; str[i] > 0; i++)
    {
        if (x > SSD1306_WIDTH - SSD1306_FONTS_WIDTH)
        {
            x = 0;
            y += SSD1306_FONTS_HEIGHT;
        }
        if (str[i] == '\n')
        {
            x = 0;
            y += SSD1306_FONTS_HEIGHT;
            continue;
        }
        if (str[i] == '\r')
        {
            x = 0;
            continue;
        }
        if (y > SSD1306_HEIGHT - SSD1306_FONTS_HEIGHT)
        {
            return;
        }
        ssd1306_i2c_bufferWriteChar(inst, x, y, str[i]);
        x += SSD1306_FONTS_WIDTH;
    }
}

void ssd1306_i2c_invert(ssd1306_i2c_inst *inst, bool inv)
{
    if (inv)
        ssd1306_i2c_send_cmd(inst, SSD1306_SET_INV_DISP);
    else
        ssd1306_i2c_send_cmd(inst, SSD1306_SET_NORM_DISP);
}

void ssd1306_i2c_invertArea(ssd1306_i2c_inst *inst, uint x_min, uint x_max, uint y_min, uint y_max)
{
    //TODO: Rewrite this function using the new general_byte_bit_copy or general_byte_bit_invert_copy.
    ssd1306_i2c_buf_t *buf = (ssd1306_i2c_buf_t *)(inst->buffer + 1);
    uint y_ = y_min, x_ = x_min;
    if (y_min / 8 != y_max / 8)
    {
        while (y_ / 8 < y_max / 8)
        {
            uint height = y_ % SSD1306_PAGE_HEIGHT;
            uint shift = SSD1306_PAGE_HEIGHT - height;
            uint page_y = y_ / SSD1306_PAGE_HEIGHT;
            if (height == 0)
            {
                y_ += SSD1306_PAGE_HEIGHT;

            }
            else
            {
                y_ += shift;
            }
            for (uint x_ = x_min; x_ <= x_max; x_++)
            {
                uint8_t t = ~(buf[page_y][x_]);
                // buf[page_y][x_] = (buf[page_y][x_] & (0xff>>shift)) | ((t>>height)<<height);
                buf[page_y][x_] = (buf[page_y][x_] & (0xff >> shift)) | (t & (0xff << height));
            }
        }
        uint shift = y_max % SSD1306_PAGE_HEIGHT + 1;
        uint page_y = y_max / SSD1306_PAGE_HEIGHT;
        uint height = SSD1306_PAGE_HEIGHT - shift;
        for (uint x_ = x_min; x_ <= x_max; x_++)
        {

            uint8_t t = ~(buf[page_y][x_]);
            // buf[page_y][x_] = (buf[page_y][x_] & (0xff<<shift)) | ((t <<height) >> height);
            buf[page_y][x_] = (buf[page_y][x_] & (0xff << shift)) | (t & (0xff >> height));
        }
    }
    else
    {
        // here y_min and y_max are in the same page
        uint s1 = y_min % SSD1306_PAGE_HEIGHT;
        uint page_y = y_max / SSD1306_PAGE_HEIGHT;
        uint s2 = SSD1306_PAGE_HEIGHT - y_max % SSD1306_PAGE_HEIGHT - 1;
        for (uint x_ = x_min; x_ <= x_max; x_++)
        {
            uint8_t t = ~(buf[page_y][x_]);
            uint8_t mask = 0xff >> s1; //(((0xff >> s1) << (s1+s2)) >> s2);
            mask = mask << (s1 + s2);
            mask = mask >> s2;
            buf[page_y][x_] = (t & mask) | (buf[page_y][x_] & ~mask);
        }
    }
}

void ssd1306_i2c_invertPageArea(ssd1306_i2c_inst *inst, uint x_min, uint x_max, uint page_min, uint page_max)
{
    ssd1306_i2c_buf_t *buf = (ssd1306_i2c_buf_t *)(inst->buffer + 1);

    for (uint i = page_min; i <= page_max; i++)
    {
        for (uint j = x_min; j <= x_max; j++)
        {
            buf[i][j] = ~buf[i][j];
        }
    }
}



int ssd1306_i2c_area_init(ssd1306_i2c_area *area, int x_min, int x_max, int page_min, int page_max)
{
    area->x_interval[0] = x_min;
    area->page_interval[0] = page_min;
    area->x_interval[1] = x_max;
    area->page_interval[1] = page_max;
    area->buffer_size = (x_max - x_min + 1) * (page_max - page_min + 1) + 1;
    area->buffer = malloc(area->buffer_size);
    memset(area->buffer, 0, area->buffer_size);
    area->buffer[0] = 0x40;
    return 0;
}

void ssd1306_i2c_area_write(ssd1306_i2c_inst *inst, ssd1306_i2c_area *area)
{
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        area->x_interval[0],
        area->x_interval[1],
        SSD1306_SET_PAGE_ADDR,
        area->page_interval[0],
        area->page_interval[1]};

    ssd1306_i2c_send_cmd_list(inst, cmds, count_of(cmds));
    i2c_write_blocking(inst->i2c_port, SSD1306_I2C_ADDR, area->buffer, area->buffer_size, false);
}

void ssd1306_i2c_area_copyData(ssd1306_i2c_area *area, const uint8_t *data, uint size)
{
    uint len = (size < area->buffer_size - 1) ? size : area->buffer_size - 1;
    memcpy(area->buffer + 1, data, len);
}

void ssd1306_i2c_area_free(ssd1306_i2c_area *area)
{
    free(area->buffer);
}

void general_byte_bit_copy(uint8_t *dest, uint d_start, uint8_t *src, uint s_start, uint length)
{
    size_t size = sizeof(*dest) * 8;

    if (length + d_start > size || length + s_start > size || length < 1)
        return;

    uint8_t smask_lsb0 = 0xff << s_start;
    uint8_t smask_msb0 = 0xff >> (size - s_start - length);
    uint8_t smask = smask_lsb0 & smask_msb0;
    uint8_t tmp = *src & smask; // only keep destination bit field
    uint8_t dmask_lsb0 = 0xff << d_start;
    uint8_t dmask_msb0 = 0xff >> (size - d_start - length);
    uint8_t dmask = ~(dmask_lsb0 & dmask_msb0);//destination bit field set to 0
    *dest &= dmask;
    if(s_start >= d_start)
        *dest |= tmp >> (s_start - d_start);
    else
        *dest |= tmp << (d_start - s_start);
}
void general_byte_bit_invert_copy(uint8_t *dest, uint d_start, uint8_t *src, uint s_start, uint length)
{
    size_t size = sizeof(*dest) * 8;

    if (length + d_start > size || length + s_start > size || length < 1)
        return;

    uint8_t smask_lsb0 = 0xff << s_start;
    uint8_t smask_msb0 = 0xff >> (size - s_start - length);
    uint8_t smask = smask_lsb0 & smask_msb0;
    uint8_t tmp = (~ *src) & smask; // only keep destination bit field
    uint8_t dmask_lsb0 = 0xff << d_start;
    uint8_t dmask_msb0 = 0xff >> (size - d_start - length);
    uint8_t dmask = ~(dmask_lsb0 & dmask_msb0);//destination bit field set to 0
    *dest &= dmask;
    if(s_start >= d_start)
        *dest |= tmp >> (s_start - d_start);
    else
        *dest |= tmp << (d_start - s_start);
}

const uint8_t ssd1306_i2c_fonts[][16] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /*" ",0*/
    {0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00}, /*"!",1*/
    {0x00, 0x10, 0x0C, 0x02, 0x10, 0x0C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /*""",2*/
    {0x00, 0x40, 0xC0, 0x78, 0x40, 0xC0, 0x78, 0x00, 0x00, 0x04, 0x3F, 0x04, 0x04, 0x3F, 0x04, 0x00}, /*"#",3*/
    {0x00, 0x70, 0x88, 0x88, 0xFC, 0x08, 0x30, 0x00, 0x00, 0x18, 0x20, 0x20, 0xFF, 0x21, 0x1E, 0x00}, /*"$",4*/
    {0xF0, 0x08, 0xF0, 0x80, 0x60, 0x18, 0x00, 0x00, 0x00, 0x31, 0x0C, 0x03, 0x1E, 0x21, 0x1E, 0x00}, /*"%",5*/
    {0x00, 0xF0, 0x08, 0x88, 0x70, 0x00, 0x00, 0x00, 0x1E, 0x21, 0x23, 0x2C, 0x19, 0x27, 0x21, 0x10}, /*"&",6*/
    {0x00, 0x12, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /*"'",7*/
    {0x00, 0x00, 0x00, 0xE0, 0x18, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x07, 0x18, 0x20, 0x40, 0x00}, /*"(",8*/
    {0x00, 0x02, 0x04, 0x18, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x18, 0x07, 0x00, 0x00, 0x00}, /*")",9*/
    {0x40, 0x40, 0x80, 0xF0, 0x80, 0x40, 0x40, 0x00, 0x02, 0x02, 0x01, 0x0F, 0x01, 0x02, 0x02, 0x00}, /*"*",10*/
    {0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x01}, /*"+",11*/
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00}, /*",",12*/
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00}, /*"-",13*/
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00}, /*".",14*/
    {0x00, 0x00, 0x00, 0x00, 0xC0, 0x38, 0x04, 0x00, 0x00, 0x60, 0x18, 0x07, 0x00, 0x00, 0x00, 0x00}, /*"/",15*/
    {0x00, 0xE0, 0x10, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x00, 0x0F, 0x10, 0x20, 0x20, 0x10, 0x0F, 0x00}, /*"0",16*/
    {0x00, 0x00, 0x10, 0x10, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00}, /*"1",17*/
    {0x00, 0x70, 0x08, 0x08, 0x08, 0x08, 0xF0, 0x00, 0x00, 0x30, 0x28, 0x24, 0x22, 0x21, 0x30, 0x00}, /*"2",18*/
    {0x00, 0x30, 0x08, 0x08, 0x08, 0x88, 0x70, 0x00, 0x00, 0x18, 0x20, 0x21, 0x21, 0x22, 0x1C, 0x00}, /*"3",19*/
    {0x00, 0x00, 0x80, 0x40, 0x30, 0xF8, 0x00, 0x00, 0x00, 0x06, 0x05, 0x24, 0x24, 0x3F, 0x24, 0x24}, /*"4",20*/
    {0x00, 0xF8, 0x88, 0x88, 0x88, 0x08, 0x08, 0x00, 0x00, 0x19, 0x20, 0x20, 0x20, 0x11, 0x0E, 0x00}, /*"5",21*/
    {0x00, 0xE0, 0x10, 0x88, 0x88, 0x90, 0x00, 0x00, 0x00, 0x0F, 0x11, 0x20, 0x20, 0x20, 0x1F, 0x00}, /*"6",22*/
    {0x00, 0x18, 0x08, 0x08, 0x88, 0x68, 0x18, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x01, 0x00, 0x00, 0x00}, /*"7",23*/
    {0x00, 0x70, 0x88, 0x08, 0x08, 0x88, 0x70, 0x00, 0x00, 0x1C, 0x22, 0x21, 0x21, 0x22, 0x1C, 0x00}, /*"8",24*/
    {0x00, 0xF0, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x00, 0x01, 0x12, 0x22, 0x22, 0x11, 0x0F, 0x00}, /*"9",25*/
    {0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00}, /*":",26*/
    {0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00}, /*";",27*/
    {0x00, 0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00}, /*"<",28*/
    {0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00}, /*"=",29*/
    {0x00, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00}, /*">",30*/
    {0x00, 0x70, 0x48, 0x08, 0x08, 0x88, 0x70, 0x00, 0x00, 0x00, 0x00, 0x30, 0x37, 0x00, 0x00, 0x00}, /*"?",31*/
    {0xC0, 0x30, 0xC8, 0x28, 0xE8, 0x10, 0xE0, 0x00, 0x07, 0x18, 0x27, 0x28, 0x2F, 0x28, 0x17, 0x00}, /*"@",32*/
    {0x00, 0x00, 0xC0, 0x38, 0xE0, 0x00, 0x00, 0x00, 0x20, 0x3C, 0x23, 0x02, 0x02, 0x27, 0x38, 0x20}, /*"A",33*/
    {0x08, 0xF8, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x11, 0x0E, 0x00}, /*"B",34*/
    {0xC0, 0x30, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x07, 0x18, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00}, /*"C",35*/
    {0x08, 0xF8, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x10, 0x0F, 0x00}, /*"D",36*/
    {0x08, 0xF8, 0x88, 0x88, 0xE8, 0x08, 0x10, 0x00, 0x20, 0x3F, 0x20, 0x20, 0x23, 0x20, 0x18, 0x00}, /*"E",37*/
    {0x08, 0xF8, 0x88, 0x88, 0xE8, 0x08, 0x10, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00}, /*"F",38*/
    {0xC0, 0x30, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 0x07, 0x18, 0x20, 0x20, 0x22, 0x1E, 0x02, 0x00}, /*"G",39*/
    {0x08, 0xF8, 0x08, 0x00, 0x00, 0x08, 0xF8, 0x08, 0x20, 0x3F, 0x21, 0x01, 0x01, 0x21, 0x3F, 0x20}, /*"H",40*/
    {0x00, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00}, /*"I",41*/
    {0x00, 0x00, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x00, 0xC0, 0x80, 0x80, 0x80, 0x7F, 0x00, 0x00, 0x00}, /*"J",42*/
    {0x08, 0xF8, 0x88, 0xC0, 0x28, 0x18, 0x08, 0x00, 0x20, 0x3F, 0x20, 0x01, 0x26, 0x38, 0x20, 0x00}, /*"K",43*/
    {0x08, 0xF8, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x20, 0x30, 0x00}, /*"L",44*/
    {0x08, 0xF8, 0xF8, 0x00, 0xF8, 0xF8, 0x08, 0x00, 0x20, 0x3F, 0x01, 0x3E, 0x01, 0x3F, 0x20, 0x00}, /*"M",45*/
    {0x08, 0xF8, 0x30, 0xC0, 0x00, 0x08, 0xF8, 0x08, 0x20, 0x3F, 0x20, 0x00, 0x07, 0x18, 0x3F, 0x00}, /*"N",46*/
    {0xE0, 0x10, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x0F, 0x10, 0x20, 0x20, 0x20, 0x10, 0x0F, 0x00}, /*"O",47*/
    {0x08, 0xF8, 0x08, 0x08, 0x08, 0x08, 0xF0, 0x00, 0x20, 0x3F, 0x21, 0x01, 0x01, 0x01, 0x00, 0x00}, /*"P",48*/
    {0xE0, 0x10, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x0F, 0x10, 0x28, 0x28, 0x30, 0x50, 0x4F, 0x00}, /*"Q",49*/
    {0x08, 0xF8, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x03, 0x0C, 0x30, 0x20}, /*"R",50*/
    {0x00, 0x70, 0x88, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 0x38, 0x20, 0x21, 0x21, 0x22, 0x1C, 0x00}, /*"S",51*/
    {0x18, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x18, 0x00, 0x00, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x00, 0x00}, /*"T",52*/
    {0x08, 0xF8, 0x08, 0x00, 0x00, 0x08, 0xF8, 0x08, 0x00, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x1F, 0x00}, /*"U",53*/
    {0x08, 0x78, 0x88, 0x00, 0x00, 0xC8, 0x38, 0x08, 0x00, 0x00, 0x07, 0x38, 0x0E, 0x01, 0x00, 0x00}, /*"V",54*/
    {0x08, 0xF8, 0x00, 0xF8, 0x00, 0xF8, 0x08, 0x00, 0x00, 0x03, 0x3E, 0x01, 0x3E, 0x03, 0x00, 0x00}, /*"W",55*/
    {0x08, 0x18, 0x68, 0x80, 0x80, 0x68, 0x18, 0x08, 0x20, 0x30, 0x2C, 0x03, 0x03, 0x2C, 0x30, 0x20}, /*"X",56*/
    {0x08, 0x38, 0xC8, 0x00, 0xC8, 0x38, 0x08, 0x00, 0x00, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x00, 0x00}, /*"Y",57*/
    {0x10, 0x08, 0x08, 0x08, 0xC8, 0x38, 0x08, 0x00, 0x20, 0x38, 0x26, 0x21, 0x20, 0x20, 0x18, 0x00}, /*"Z",58*/
    {0x00, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x40, 0x40, 0x40, 0x00}, /*"[",59*/
    {0x00, 0x04, 0x38, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x38, 0xC0, 0x00}, /*"\",60*/
    {0x00, 0x02, 0x02, 0x02, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x7F, 0x00, 0x00, 0x00}, /*"]",61*/
    {0x00, 0x00, 0x04, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /*"^",62*/
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80}, /*"_",63*/
    {0x00, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /*"`",64*/
    {0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x19, 0x24, 0x24, 0x12, 0x3F, 0x20, 0x00}, /*"a",65*/
    {0x10, 0xF0, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x11, 0x20, 0x20, 0x11, 0x0E, 0x00}, /*"b",66*/
    {0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x0E, 0x11, 0x20, 0x20, 0x20, 0x11, 0x00}, /*"c",67*/
    {0x00, 0x00, 0x80, 0x80, 0x80, 0x90, 0xF0, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x20, 0x10, 0x3F, 0x20}, /*"d",68*/
    {0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x1F, 0x24, 0x24, 0x24, 0x24, 0x17, 0x00}, /*"e",69*/
    {0x00, 0x80, 0x80, 0xE0, 0x90, 0x90, 0x20, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00}, /*"f",70*/
    {0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x6B, 0x94, 0x94, 0x94, 0x93, 0x60, 0x00}, /*"g",71*/
    {0x10, 0xF0, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x20, 0x3F, 0x21, 0x00, 0x00, 0x20, 0x3F, 0x20}, /*"h",72*/
    {0x00, 0x80, 0x98, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00}, /*"i",73*/
    {0x00, 0x00, 0x00, 0x80, 0x98, 0x98, 0x00, 0x00, 0x00, 0xC0, 0x80, 0x80, 0x80, 0x7F, 0x00, 0x00}, /*"j",74*/
    {0x10, 0xF0, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x20, 0x3F, 0x24, 0x06, 0x29, 0x30, 0x20, 0x00}, /*"k",75*/
    {0x00, 0x10, 0x10, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00}, /*"l",76*/
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x3F, 0x20, 0x00, 0x3F}, /*"m",77*/
    {0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x20, 0x3F, 0x21, 0x00, 0x00, 0x20, 0x3F, 0x20}, /*"n",78*/
    {0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x1F, 0x00}, /*"o",79*/
    {0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x91, 0x20, 0x20, 0x11, 0x0E, 0x00}, /*"p",80*/
    {0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 0x00, 0x0E, 0x11, 0x20, 0x20, 0x91, 0xFF, 0x80}, /*"q",81*/
    {0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x20, 0x20, 0x3F, 0x21, 0x20, 0x00, 0x01, 0x00}, /*"r",82*/
    {0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x33, 0x24, 0x24, 0x24, 0x24, 0x19, 0x00}, /*"s",83*/
    {0x00, 0x80, 0x80, 0xE0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x10, 0x00}, /*"t",84*/
    {0x80, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x20, 0x10, 0x3F, 0x20}, /*"u",85*/
    {0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x03, 0x0C, 0x30, 0x0C, 0x03, 0x00, 0x00}, /*"v",86*/
    {0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x01, 0x0E, 0x30, 0x0C, 0x07, 0x38, 0x06, 0x01}, /*"w",87*/
    {0x00, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x20, 0x31, 0x0E, 0x2E, 0x31, 0x20, 0x00}, /*"x",88*/
    {0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x81, 0x86, 0x78, 0x18, 0x06, 0x01, 0x00}, /*"y",89*/
    {0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x21, 0x30, 0x2C, 0x22, 0x21, 0x30, 0x00}, /*"z",90*/
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x3E, 0x40, 0x40}, /*"{",91*/
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00}, /*"|",92*/
    {0x02, 0x02, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00}, /*"}",93*/
    {0x00, 0x02, 0x01, 0x02, 0x02, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /*"~",94*/
};
