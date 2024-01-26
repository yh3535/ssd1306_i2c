#include "ssd1306_i2c_GT21L16S2Y.h"
#include <pico/stdlib.h>
#include <stdlib.h>
#include <string.h>
void sig_inst_init(sig_inst *inst, ssd1306_i2c_inst *ssd1306, spi_inst_t *spi_port, uint8_t port_index[], uint32_t font, uint32_t speed_khz)
{
    inst->ssd1306 = ssd1306;
    inst->spi_port = spi_port;
    inst->font = font;
    memcpy(inst->port_index, port_index, sizeof(inst->port_index));

    spi_init(inst->spi_port, speed_khz * 1000);
    gpio_set_function(inst->port_index[1], GPIO_FUNC_SPI);
    gpio_set_function(inst->port_index[2], GPIO_FUNC_SPI);
    gpio_set_function(inst->port_index[3], GPIO_FUNC_SPI);
    gpio_init(inst->port_index[0]);
    gpio_set_dir(inst->port_index[0], GPIO_OUT);
    gpio_put(inst->port_index[0], 1);
}

void sig_cs_select(sig_inst *inst)
{
    asm volatile("nop \n nop \n nop");
    gpio_put(inst->port_index[0], 0); // Active low
    asm volatile("nop \n nop \n nop");
}

void sig_cs_deselect(sig_inst *inst)
{
    asm volatile("nop \n nop \n nop");
    gpio_put(inst->port_index[0], 1);
    asm volatile("nop \n nop \n nop");
}

uint32_t sig_getHanziAddressWord(sig_inst *inst, const char *hanzi)
{
    uint32_t Address = 0;
    uint8_t MSB = hanzi[0], LSB = hanzi[1];

    if (inst->font == GT21L16S2Y_FONT_1616)
    {
        uint32_t BaseAdd = GB2312_CN1616_Address;
        if (MSB == 0xA9 && LSB >= 0xA1)
            Address = (282 + (LSB - 0xA1)) * 32;
        else if (MSB >= 0xA1 && MSB <= 0xA3 && LSB >= 0xA1)
            Address = ((MSB - 0xA1) * 94 + (LSB - 0xA1)) * 32;
        else if (MSB >= 0xB0 && MSB <= 0xF7 && LSB >= 0xA1)
            Address = (846 + (MSB - 0xB0) * 94 + (LSB - 0xA1)) * 32;
    }
    else if (inst->font == GT21L16S2Y_FONT_1212)
    {
        uint32_t BaseAdd = GB2312_CN1212_Address;
        if (MSB >= 0xA1 && MSB <= 0xA3 && LSB >= 0xA1)
            Address = ((MSB - 0xA1) * 94 + (LSB - 0xA1)) * 24 + BaseAdd;
        else if (MSB == 0xA9 && LSB >= 0xA1)
            Address = (282 + (LSB - 0xA1)) * 24 + BaseAdd;
        else if (MSB >= 0xB0 && MSB <= 0xF7 && LSB >= 0xA1)
            Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1) + 376) * 24 + BaseAdd;
    }
    // addr[2]=Address;
    // addr[1]=Address >> 8;
    // addr[0]=Address >> 16;
    uint8_t addr_word[4] = {GT21L16S2Y_CMD_READ, Address >> 16, Address >> 8, Address};
    return *(uint32_t *)addr_word;
}

size_t sig_getHanziDataLength(sig_inst *inst)
{
    size_t length = 0;
    if (inst->font == GT21L16S2Y_FONT_1616)
        length = 32;
    else if (inst->font == GT21L16S2Y_FONT_1212)
        length = 24;
    return length;
}

void sig_getHanziData(sig_inst *inst, const char *hanzi, uint8_t *data)
{
    if (data == NULL || hanzi == NULL)
        return;
    size_t length = sig_getHanziDataLength(inst);
    uint32_t addr_word = sig_getHanziAddressWord(inst, hanzi);
    sig_cs_select(inst);
    spi_write_blocking(inst->spi_port, (const uint8_t *)&addr_word, sizeof(addr_word));
    spi_read_blocking(inst->spi_port, 0, data, length);
    sig_cs_deselect(inst);
}

void sig_bufferWriteHanzi(sig_inst *inst, const char *hanzi, uint32_t x, uint32_t y)
{
    uint8_t data[32];
    sig_getHanziData(inst, hanzi, data);
    ssd1306_i2c_buf_t *buf = (ssd1306_i2c_buf_t *)(inst->ssd1306->buffer + 1);
    const size_t page_height = SSD1306_PAGE_HEIGHT;
    int page_y = y / page_height;
    int mod_y = y % page_height;
    if (inst->font == GT21L16S2Y_FONT_1616)
    {
        if (x + 16 > SSD1306_WIDTH || y + 16 > SSD1306_HEIGHT)
            return;
        if (mod_y == 0)
        {
            for (int i = 0; i < 16; i++)
            {
                buf[page_y][x + i] = data[i];
                buf[page_y + 1][x + i] = data[i + 16];
            }
        }
        else
        {
            for (int i = 0; i < 16; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y][x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], mod_y, &data[i + 16], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 2][x + i], 0, &data[i + 16], page_height - mod_y, mod_y);
            }
        }
    }
    else if (inst->font == GT21L16S2Y_FONT_1212)
    {
        if (x + 12 > SSD1306_WIDTH || y + 12 > SSD1306_HEIGHT)
            return;

        if (mod_y == 0)
        {
            for (int i = 0; i < 12; i++)
            {
                buf[page_y][x + i] = data[i];
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], 0, &data[i + 12], 0, 4);
            }
        }
        else if (mod_y <= 4)
        {
            // 此时只占用两个page
            for (int i = 0; i < 12; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y][x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], mod_y, &data[i + 12], 0, 4);
            }
        }
        else
        {
            // 此时需要三个page
            for (int i = 0; i < 12; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y][x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], mod_y, &data[i + 12], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 2][x + i], 0, &data[i + 12], page_height - mod_y, mod_y - 4);
            }
        }
    }
}

void sig_bufferWriteHString(sig_inst *inst, const char *str, size_t len, uint32_t x, uint32_t y)
{
    uint32_t width = 0;
    if (inst->font == GT21L16S2Y_FONT_1616)
        width = 16;
    else if (inst->font == GT21L16S2Y_FONT_1212)
        width = 12;

    for (int i = 0; i < len; i++)
    {
        if (x + width > SSD1306_WIDTH)
        {
            x = 0;
            y += width;
        }
        if (y + width > SSD1306_HEIGHT)
            return;
        sig_bufferWriteHanzi(inst, str, x, y);
        str += 2;
        x += width;
    }
}

uint32_t sig_getAsciiAddressWord(sig_inst *inst, const char *ascii)
{
    uint32_t Address = 0;
    uint8_t ASCIICode = *ascii;

    if (inst->font == GT21L16S2Y_FONT_1616)
    {
        uint32_t BaseAdd = ASCII_1616_Address;
        if ((ASCIICode >= 0x20) && (ASCIICode <= 0x7E))
            Address = (ASCIICode - 0x20) * 16 + BaseAdd;
    }
    else if (inst->font == GT21L16S2Y_FONT_1212)
    {
        uint32_t BaseAdd = ASCII_1212_Address;
        if ((ASCIICode >= 0x20) && (ASCIICode <= 0x7E))
            Address = (ASCIICode - 0x20) * 12 + BaseAdd;
    }

    uint8_t addr_word[4] = {GT21L16S2Y_CMD_READ, Address >> 16, Address >> 8, Address};
    return *(uint32_t *)addr_word;
}

size_t sig_getAsciiDataLength(sig_inst *inst)
{
    size_t length = 0;
    if (inst->font == GT21L16S2Y_FONT_1616)
        length = 16;
    else if (inst->font == GT21L16S2Y_FONT_1212)
        length = 12;
    return length;
}

void sig_getAsciiData(sig_inst *inst, const char *ascii, uint8_t *data)
{
    if (data == NULL || ascii == NULL)
        return;
    size_t length = sig_getAsciiDataLength(inst);
    uint32_t addr_word = sig_getAsciiAddressWord(inst, ascii);
    sig_cs_select(inst);
    spi_write_blocking(inst->spi_port, (const uint8_t *)&addr_word, sizeof(addr_word));
    spi_read_blocking(inst->spi_port, 0, data, length);
    sig_cs_deselect(inst);
}

void sig_bufferWriteAscii(sig_inst *inst, const char *ascii, uint32_t x, uint32_t y)
{
    uint32_t width = 0, height = 0;
    if (inst->font == GT21L16S2Y_FONT_1616)
    {
        width = 8;
        height = 16;
    }
    else if (inst->font == GT21L16S2Y_FONT_1212)
    {
        width = 6;
        height = 12;
    }

    uint8_t data[16];
    sig_getAsciiData(inst, ascii, data);
    ssd1306_i2c_buf_t *buf = (ssd1306_i2c_buf_t *)(inst->ssd1306->buffer + 1);
    const size_t page_height = SSD1306_PAGE_HEIGHT;
    int page_y = y / page_height;
    int mod_y = y % page_height;
    if (inst->font == GT21L16S2Y_FONT_1616)
    {
        if (x + width > SSD1306_WIDTH || y + height > SSD1306_HEIGHT)
            return;
        if (mod_y == 0)
        {
            for (int i = 0; i < width; i++)
            {
                buf[page_y][x + i] = data[i];
                buf[page_y + 1][x + i] = data[i + width];
            }
        }
        else
        {
            for (int i = 0; i < width; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y][x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], mod_y, &data[i + width], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 2][x + i], 0, &data[i + width], page_height - mod_y, mod_y);
            }
        }
    }
    else if (inst->font == GT21L16S2Y_FONT_1212)
    {
        if (x + width > SSD1306_WIDTH || y + height > SSD1306_HEIGHT)
            return;

        if (mod_y == 0)
        {
            for (int i = 0; i < width; i++)
            {
                buf[page_y][x + i] = data[i];
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], 0, &data[i + width], 0, 4);
            }
        }
        else if (mod_y <= 4)
        {
            // 此时只占用两个page
            for (int i = 0; i < width; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y][x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], mod_y, &data[i + width], 0, 4);
            }
        }
        else
        {
            // 此时需要三个page
            for (int i = 0; i < width; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y][x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 1][x + i], mod_y, &data[i + width], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[page_y + 2][x + i], 0, &data[i + width], page_height - mod_y, mod_y - 4);
            }
        }
    }
}

void sig_bufferWriteString(sig_inst *inst, const uint8_t *str, uint32_t x, uint32_t y)
{
    // ASCII字符的字宽。汉字需要乘以2。
    uint32_t width = 0, height = 0;
    if (inst->font == GT21L16S2Y_FONT_1616)
    {
        width = 8;
        height = 16;
    }
    else if (inst->font == GT21L16S2Y_FONT_1212)
    {
        width = 6;
        height = 12;
    }

    while (*str)
    {

        if (*str > 127 && *(str + 1) > 127)
        {
            if (x + width * 2 > SSD1306_WIDTH)
            {
                x = 0;
                y += height;
            }
            if (y + height > SSD1306_HEIGHT)
                return;
            sig_bufferWriteHanzi(inst, str, x, y);
            str += 2;
            x += width * 2;
        }
        else if (*str == '\r')
        {
            x = 0;
            str++;
        }
        else if (*str == '\n')
        {
            x = 0;
            y += height;
            str++;
            if (y + height > SSD1306_HEIGHT)
                return;
        }
        else if (*str > 0)
        {
            if (x + width > SSD1306_WIDTH)
            {
                x = 0;
                y += height;
            }
            if (y + height > SSD1306_HEIGHT)
                return;
            sig_bufferWriteAscii(inst, str, x, y);
            str++;
            x += width;
        }
        else
            return;
    }
}