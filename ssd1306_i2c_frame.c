#include "ssd1306_i2c_GT21L16S2Y.h"
#include "ssd1306_i2c_frame.h"
#include "stdlib.h"
#include "string.h"

void si_frame_init(si_frame *frame, uint16_t frame_width, uint16_t frame_height)
{
    frame->width = frame_width;
    frame->height = frame_height;
    frame->page_number = frame_height / SSD1306_PAGE_HEIGHT + (frame_height % SSD1306_PAGE_HEIGHT ? 1 : 0);
    frame->buffer_size = frame_width * frame->page_number + 1;
    frame->buffer = calloc(1, frame->buffer_size);
    frame->buffer[0] = 0x40;
}

void si_frame_free(si_frame *frame)
{
    free(frame->buffer);
}

void si_frame_setPixel(si_frame *frame, int x, int y, bool on)
{
    if (!(x >= 0 && x < frame->width && y >= 0 && y < frame->height))
    {
        return;
    }
    uint8_t *buf = frame->buffer + 1; // the first byte is 0x40
    uint8_t *pbyte = &(buf[y / SSD1306_PAGE_HEIGHT * frame->width + x]);

    if (on)
        *pbyte |= 1 << (y % SSD1306_PAGE_HEIGHT); // y%SSD1306_PAGE_HEIGHT is which bit of the byte.
    else
        *pbyte &= ~(1 << (y % SSD1306_PAGE_HEIGHT));
}

void si_frame_drawLine(si_frame *frame, int x0, int y0, int x1, int y1, bool on)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true)
    {

        si_frame_setPixel(frame, x0, y0, on);
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

void si_frame_writeChar(si_frame *frame, uint16_t x, uint16_t y, uint8_t ch)
{
    if (x > frame->width - SSD1306_FONTS_WIDTH || y > frame->height - SSD1306_FONTS_HEIGHT)
        return;
    uint8_t *buf = frame->buffer + 1;
    int16_t page_y = y / SSD1306_PAGE_HEIGHT;
    int16_t mod_y = y % SSD1306_PAGE_HEIGHT;
    const uint8_t *font_data = ssd1306_i2c_fonts[ssd1306_i2c_getFontIndex(ch)];
    if (mod_y == 0)
    {
        for (int i = 0; i < SSD1306_FONTS_WIDTH; i++)
        {
            buf[page_y * frame->width + x + i] = font_data[i];
            buf[(page_y + 1) * frame->width + x + i] = font_data[SSD1306_FONTS_WIDTH + i];
        }
    }
    else
    {
        for (int i = 0; i < SSD1306_FONTS_WIDTH; i++)
        {
            ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x + i], mod_y, &font_data[i], 0, SSD1306_PAGE_HEIGHT - mod_y);
            ssd1306_i2c_generalByteBitCopy(&buf[(page_y + 1) * frame->width + x + i], 0, &font_data[i], SSD1306_PAGE_HEIGHT - mod_y, mod_y);
            ssd1306_i2c_generalByteBitCopy(&buf[(page_y + 1) * frame->width + x + i], mod_y, &font_data[SSD1306_FONTS_WIDTH + i], 0, SSD1306_PAGE_HEIGHT - mod_y);
            ssd1306_i2c_generalByteBitCopy(&buf[(page_y + 2) * frame->width + x + i], 0, &font_data[SSD1306_FONTS_WIDTH + i], SSD1306_PAGE_HEIGHT - mod_y, mod_y);
        }
    }
}

void si_frame_writeCString(si_frame *frame, uint16_t x, uint16_t y, const uint8_t *str)
{
    for (int i = 0; str[i] > 0; i++)
    {
        if (x > frame->width - SSD1306_FONTS_WIDTH)
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
        if (y > frame->height - SSD1306_FONTS_HEIGHT)
            return;

        si_frame_writeChar(frame, x, y, str[i]);
        x += SSD1306_FONTS_WIDTH;
    }
}

void si_frame_invert(si_frame *frame)
{
    uint8_t *buf = frame->buffer + 1;
    for (int i = 0; i < frame->buffer_size - 1; i++)
    {
        buf[i] = ~buf[i];
    }
}

void si_frame_invertArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max)
{
    if (x_max >= frame->width || y_max >= frame->height || x_min > x_max || y_min > y_max)
        return;
    uint32_t x = x_min;
    uint32_t y = y_min;
    uint32_t page_height = SSD1306_PAGE_HEIGHT;
    uint8_t *buf = (frame->buffer + 1);

    if (y_min / page_height != y_max / page_height)
    {
        uint32_t page_y = y / page_height;
        uint32_t mod_y = y % page_height;
        while (page_y < y_max / page_height)
        {

            for (x = x_min; x <= x_max; x++)
                ssd1306_i2c_generalByteBitInvertCopy(&buf[page_y * frame->width + x], mod_y, &buf[page_y * frame->width + x], mod_y, page_height - mod_y);

            page_y++;
            mod_y = 0;
        }
        for (x = x_min; x <= x_max; x++)
            ssd1306_i2c_generalByteBitInvertCopy(&buf[page_y * frame->width + x], 0, &buf[page_y * frame->width + x], 0, y_max % page_height + 1);
    }
    else
    {
        uint32_t page_y = y / page_height;
        for (x = x_min; x <= x_max; x++)
            ssd1306_i2c_generalByteBitInvertCopy(&buf[page_y * frame->width + x], y_min % page_height, &buf[page_y * frame->width + x], y_min % page_height, y_max - y_min + 1);
    }
}

void si_frame_invertPageArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max)
{
    // NOTE: This function is a page operation.
    if (x_max >= frame->width || page_max >= frame->page_number || x_min > x_max || page_min > page_max)
        return;
    uint8_t *buf = (frame->buffer + 1);

    for (uint i = page_min; i <= page_max; i++)
    {
        for (uint j = x_min; j <= x_max; j++)
        {
            buf[i * frame->width + j] = ~buf[i * frame->width + j];
        }
    }
}

void si_frame_clear(si_frame *frame)
{
    memset(frame->buffer + 1, 0, frame->buffer_size - 1);
}

void si_frame_clearArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max)
{
    if (x_max >= frame->width || y_max >= frame->height || x_min > x_max || y_min > y_max)
        return;
    uint32_t x = x_min;
    uint32_t y = y_min;
    uint32_t page_height = SSD1306_PAGE_HEIGHT;
    uint8_t *buf = (frame->buffer + 1);
    uint8_t zero = 0;
    if (y_min / page_height != y_max / page_height)
    {
        uint32_t page_y = y / page_height;
        uint32_t mod_y = y % page_height;
        while (page_y < y_max / page_height)
        {

            for (x = x_min; x <= x_max; x++)
                ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x], mod_y, &zero, mod_y, page_height - mod_y);

            page_y++;
            mod_y = 0;
        }
        for (x = x_min; x <= x_max; x++)
            ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x], 0, &zero, 0, y_max % page_height + 1);
    }
    else
    {
        uint32_t page_y = y / page_height;
        for (x = x_min; x <= x_max; x++)
            ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x], y_min % page_height, &zero, y_min % page_height, y_max - y_min + 1);
    }
}

void si_frame_clearPageArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max)
{
    if (x_max >= frame->width || page_max >= frame->page_number || x_min > x_max || page_min > page_max)
        return;
    uint8_t *buf = (frame->buffer + 1);

    for (uint i = page_min; i <= page_max; i++)
    {
        for (uint j = x_min; j <= x_max; j++)
        {
            buf[i * frame->width + j] = 0;
        }
    }
}

void si_frame_set(si_frame *frame)
{
    memset(frame->buffer + 1, 0xff, frame->buffer_size - 1);
}

void si_frame_setArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max)
{
    if (x_max >= frame->width || y_max >= frame->height || x_min > x_max || y_min > y_max)
        return;
    uint32_t x = x_min;
    uint32_t y = y_min;
    uint32_t page_height = SSD1306_PAGE_HEIGHT;
    uint8_t *buf = (frame->buffer + 1);
    uint8_t one = 0xff;
    if (y_min / page_height != y_max / page_height)
    {
        uint32_t page_y = y / page_height;
        uint32_t mod_y = y % page_height;
        while (page_y < y_max / page_height)
        {

            for (x = x_min; x <= x_max; x++)
                ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x], mod_y, &one, mod_y, page_height - mod_y);

            page_y++;
            mod_y = 0;
        }
        for (x = x_min; x <= x_max; x++)
            ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x], 0, &one, 0, y_max % page_height + 1);
    }
    else
    {
        uint32_t page_y = y / page_height;
        for (x = x_min; x <= x_max; x++)
            ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x], y_min % page_height, &one, y_min % page_height, y_max - y_min + 1);
    }
}

void si_frame_setPageArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max)
{
    if (x_max >= frame->width || page_max >= frame->page_number || x_min > x_max || page_min > page_max)
        return;
    uint8_t *buf = (frame->buffer + 1);

    for (uint i = page_min; i <= page_max; i++)
    {
        for (uint j = x_min; j <= x_max; j++)
        {
            buf[i * frame->width + j] = 0xff;
        }
    }
}

void ssd1306_i2c_directPutFrame(ssd1306_i2c_inst *inst, uint16_t x, uint16_t page_y, si_frame *frame)
{

    // NOTE: Direct put frame has a lot of limitations, since it is a page operation and won't change inst->buffer.
    if (x + frame->width > SSD1306_WIDTH || page_y + frame->page_number > SSD1306_NUM_PAGES)
        return;
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        x,
        x + frame->width - 1,
        SSD1306_SET_PAGE_ADDR,
        page_y,
        page_y + frame->page_number - 1};

    ssd1306_i2c_send_cmd_list(inst, cmds, count_of(cmds));
    i2c_write_blocking(inst->i2c_port, SSD1306_I2C_ADDR, frame->buffer, frame->buffer_size, false);
}

void ssd1306_i2c_bufferPutFrame(ssd1306_i2c_inst *inst, int16_t x, int16_t y, si_frame *frame)
{
    int16_t x_min = x;
    int16_t y_min = y;
    int16_t x_max = x + frame->width - 1;
    int16_t y_max = y + frame->height - 1;
    if (x_max < 0 || y_max < 0 || x_min >= SSD1306_WIDTH || y_min >= SSD1306_HEIGHT)
        return;
    if (x_max >= SSD1306_WIDTH)
        x_max = SSD1306_WIDTH - 1;
    if (y_max >= SSD1306_HEIGHT)
        y_max = SSD1306_HEIGHT - 1;

    uint32_t page_height = SSD1306_PAGE_HEIGHT;
    uint8_t *inst_buf = (inst->buffer + 1);
    uint8_t *frame_buf = (frame->buffer + 1);
    uint16_t f_y = 0, f_x = 0;
    uint16_t f_y_start = 0, f_x_start = 0;
    if (x_min < 0)
    {
        f_x_start = -x_min;
        x_min = 0;
    }
    if (y_min < 0)
    {
        f_y_start = -y_min;
        y_min = 0;
    }
    if (y_min / page_height != y_max / page_height)
    {
        uint32_t page_y = y_min / page_height;
        uint32_t mod_y = y_min % page_height;
        uint32_t fpage_y = f_y_start / page_height;
        uint32_t fmod_y = f_y_start % page_height;
        if (page_height - mod_y <= page_height - fmod_y)
            for (x = x_min, f_x = f_x_start; x <= x_max; x++, f_x++)
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], mod_y,
                                               &frame_buf[fpage_y * frame->width + f_x], fmod_y,
                                               page_height - mod_y);
        else
        {
            uint32_t fpy_1 = fpage_y + 1;
            for (x = x_min, f_x = f_x_start; x <= x_max; x++, f_x++)
            {
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], mod_y,
                                               &frame_buf[fpage_y * frame->width + f_x], fmod_y,
                                               page_height - fmod_y);
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], mod_y + page_height - fmod_y,
                                               &frame_buf[fpy_1 * frame->width + f_x], 0,
                                               fmod_y - mod_y);
            }
            fpage_y = fpy_1;
        }
        page_y++;
        int32_t l = fmod_y - mod_y;
        int32_t ph_add_l = page_height + l;
        int32_t ph_sub_l = page_height - l;
        if (ph_add_l > page_height)
            ph_add_l %= page_height;
        if (ph_sub_l >= page_height)
            ph_sub_l %= page_height;
        while (page_y < y_max / page_height)
        {
            uint32_t fpy_1 = fpage_y + 1;
            for (x = x_min, f_x = f_x_start; x <= x_max; x++, f_x++)
            {
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], 0,
                                               &frame_buf[fpage_y * frame->width + f_x], ph_add_l,
                                               ph_sub_l);

                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], ph_sub_l,
                                               &frame_buf[fpy_1 * frame->width + f_x], 0,
                                               ph_add_l);
            }

            page_y++;
            fpage_y++;
        }
        if (y_max % page_height + 1 <= ph_sub_l)
            for (x = x_min, f_x = f_x_start; x <= x_max; x++, f_x++)
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], 0,
                                               &frame_buf[fpage_y * frame->width + f_x], ph_add_l,
                                               y_max % page_height + 1);
        else
        {
            uint32_t fpy_1 = fpage_y + 1;
            for (x = x_min, f_x = f_x_start; x <= x_max; x++, f_x++)
            {
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], 0,
                                               &frame_buf[fpage_y * frame->width + f_x], ph_add_l,
                                               ph_sub_l);
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], ph_sub_l,
                                               &frame_buf[fpy_1 * frame->width + f_x], 0,
                                               y_max % page_height + 1 - ph_sub_l);
            }
        }
    }
    else
    {
        uint32_t page_y = y_min / page_height;
        uint32_t fmod_y = f_y_start % page_height;
        uint32_t fpage_y = f_y_start / page_height;
        if (y_max - y_min + 1 < page_height - fmod_y)
            for (x = x_min, f_x = f_x_start; x <= x_max; x++, f_x++)
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], y_min % page_height,
                                               &frame_buf[fpage_y * frame->width + f_x], fmod_y,
                                               y_max - y_min + 1);
        else
        {
            uint32_t fpy_1 = fpage_y + 1;
            for (x = x_min, f_x = f_x_start; x <= x_max; x++, f_x++)
            {
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], y_min % page_height,
                                               &frame_buf[fpage_y * frame->width + f_x], fmod_y,
                                               page_height - fmod_y);
                ssd1306_i2c_generalByteBitCopy(&inst_buf[page_y * SSD1306_WIDTH + x], y_min % page_height + page_height - fmod_y,
                                               &frame_buf[fpy_1 * frame->width + f_x], 0,
                                               y_max - y_min + 1 - (page_height - fmod_y));
            }
        }
    }
}

#ifdef _SSD1306_I2C_GT21L16S2Y_H_

void sifg_writeHanzi(si_frame *frame, sig_inst *sig, const char *hanzi, uint16_t x, uint16_t y)
{
    uint8_t data[32];
    sig_getHanziData(sig, hanzi, data);
    uint8_t *buf = frame->buffer + 1;
    const size_t page_height = SSD1306_PAGE_HEIGHT;
    uint16_t page_y = y / page_height;
    uint16_t mod_y = y % page_height;
    if (sig->font == GT21L16S2Y_FONT_1616)
    {
        if (x + 16 > frame->width || y + 16 > frame->height)
            return;
        if (mod_y == 0)
        {
            uint32_t py_1 = page_y + 1;
            for (int i = 0; i < 16; i++)
            {
                buf[page_y * frame->width + x + i] = data[i];
                buf[py_1 * frame->width + x + i] = data[i + 16];
            }
        }
        else
        {
            uint16_t py_1 = page_y + 1;
            uint16_t py_2 = page_y + 2;
            for (int i = 0; i < 16; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], mod_y, &data[i + 16], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_2 * frame->width + x + i], 0, &data[i + 16], page_height - mod_y, mod_y);
            }
        }
    }
    else if (sig->font == GT21L16S2Y_FONT_1212)
    {
        if (x + 12 > frame->width || y + 12 > frame->height)
            return;

        if (mod_y == 0)
        {
            uint32_t py_1 = page_y + 1;
            for (int i = 0; i < 12; i++)
            {
                buf[page_y * frame->width + x + i] = data[i];
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], 0, &data[i + 12], 0, 4);
            }
        }
        else if (mod_y <= 4)
        {
            // 此时只占用两个page
            uint32_t py_1 = page_y + 1;
            for (int i = 0; i < 12; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], mod_y, &data[i + 12], 0, 4);
            }
        }
        else
        {
            // 此时需要三个page
            uint16_t py_1 = page_y + 1;
            uint16_t py_2 = page_y + 2;
            for (int i = 0; i < 12; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], mod_y, &data[i + 12], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_2 * frame->width + x + i], 0, &data[i + 12], page_height - mod_y, mod_y - 4);
            }
        }
    }
}

void sifg_writeHString(si_frame *frame, sig_inst *sig, const char *str, size_t len, uint16_t x, uint16_t y)
{
    uint32_t width = 0;
    if (sig->font == GT21L16S2Y_FONT_1616)
        width = 16;
    else if (sig->font == GT21L16S2Y_FONT_1212)
        width = 12;

    for (int i = 0; i < len; i++)
    {
        if (x + width > frame->width)
        {
            x = 0;
            y += width;
        }
        if (y + width > frame->height)
            return;
        sifg_writeHanzi(frame, sig, str, x, y);
        str += 2;
        x += width;
    }
}

void sifg_writeAscii(si_frame *frame, sig_inst *sig, const char *ascii, uint16_t x, uint16_t y)
{
    uint16_t width = 0, height = 0;
    if (sig->font == GT21L16S2Y_FONT_1616)
    {
        width = 8;
        height = 16;
    }
    else if (sig->font == GT21L16S2Y_FONT_1212)
    {
        width = 6;
        height = 12;
    }

    uint8_t data[16];
    sig_getAsciiData(sig, ascii, data);
    uint8_t *buf = frame->buffer + 1;
    const size_t page_height = SSD1306_PAGE_HEIGHT;
    uint16_t page_y = y / page_height;
    uint16_t mod_y = y % page_height;
    if (sig->font == GT21L16S2Y_FONT_1616)
    {
        if (x + width > frame->width || y + height > frame->height)
            return;
        if (mod_y == 0)
        {
            uint32_t py_1 = page_y + 1;
            for (int i = 0; i < width; i++)
            {
                buf[page_y * frame->width + x + i] = data[i];
                buf[py_1 * frame->width + x + i] = data[i + width];
            }
        }
        else
        {
            uint16_t py_1 = page_y + 1;
            uint16_t py_2 = page_y + 2;
            for (int i = 0; i < width; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], mod_y, &data[i + width], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_2 * frame->width + x + i], 0, &data[i + width], page_height - mod_y, mod_y);
            }
        }
    }
    else if (sig->font == GT21L16S2Y_FONT_1212)
    {
        if (x + width > frame->width || y + height > frame->height)
            return;

        if (mod_y == 0)
        {
            uint32_t py_1 = page_y + 1;
            for (int i = 0; i < width; i++)
            {
                buf[page_y * frame->width + x + i] = data[i];
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], 0, &data[i + width], 0, 4);
            }
        }
        else if (mod_y <= 4)
        {
            // 此时只占用两个page
            uint32_t py_1 = page_y + 1;
            for (int i = 0; i < width; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], mod_y, &data[i + width], 0, 4);
            }
        }
        else
        {
            // 此时需要三个page
            uint16_t py_1 = page_y + 1;
            uint16_t py_2 = page_y + 2;
            for (int i = 0; i < width; i++)
            {
                ssd1306_i2c_generalByteBitCopy(&buf[page_y * frame->width + x + i], mod_y, &data[i], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], 0, &data[i], page_height - mod_y, mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_1 * frame->width + x + i], mod_y, &data[i + width], 0, page_height - mod_y);
                ssd1306_i2c_generalByteBitCopy(&buf[py_2 * frame->width + x + i], 0, &data[i + width], page_height - mod_y, mod_y - 4);
            }
        }
    }
}

void sifg_writeString(si_frame *frame, sig_inst *sig, const uint8_t *str, uint16_t x, uint16_t y)
{
    // ASCII字符的字宽。汉字需要乘以2。
    uint32_t width = 0, height = 0;
    if (sig->font == GT21L16S2Y_FONT_1616)
    {
        width = 8;
        height = 16;
    }
    else if (sig->font == GT21L16S2Y_FONT_1212)
    {
        width = 6;
        height = 12;
    }

    while (*str)
    {

        if (*str > 127 && *(str + 1) > 127)
        {
            if (x + width * 2 > frame->width)
            {
                x = 0;
                y += height;
            }
            if (y + height > frame->height)
                return;
            sifg_writeHanzi(frame, sig, str, x, y);
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
            if (y + height > frame->height)
                return;
        }
        else if (*str > 0)
        {
            if (x + width > frame->width)
            {
                x = 0;
                y += height;
            }
            if (y + height > frame->height)
                return;
            sifg_writeAscii(frame, sig, str, x, y);
            str++;
            x += width;
        }
        else
            return;
    }
}

#endif /* _SSD1306_I2C_GT21L16S2Y_H_ */
