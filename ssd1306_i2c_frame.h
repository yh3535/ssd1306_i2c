#ifndef _SSD1306_I2C_FRAME_H_
#define _SSD1306_I2C_FRAME_H_
#include "ssd1306_i2c.h"
/*
This imports a new class si_frame that
 makes ssd1306_i2c displaying more flexible.
It needs at least ssd1306_i2c.h but
 ssd1306_i2c_GT21L16S2Y.h is not necessary.
The prefix si_ means ssd1306_i2c_ .

The frame height is n pixels but some page
 operations will regard it as m pages, in
 which m = frame_height / SSD1306_PAGE_HEIGHT
  + (frame_height % SSD1306_PAGE_HEIGHT ? 1 : 0)
*/
typedef struct _si_frame
{
  uint16_t width, height;
  uint16_t page_number;
  uint8_t *buffer; // The buffer size should be frame_width * page_number + 1
                   // in which the first byte needs to be 0x40.
  uint32_t buffer_size;

} si_frame;

void si_frame_init(si_frame *frame, uint16_t frame_width, uint16_t frame_height);
void si_frame_free(si_frame *frame);
void si_frame_setPixel(si_frame *frame, int x, int y, bool on);
void si_frame_drawLine(si_frame *frame, int x0, int y0, int x1, int y1, bool on);
void si_frame_writeChar(si_frame *frame, uint16_t x, uint16_t y, uint8_t ch);
void si_frame_writeCString(si_frame *frame, uint16_t x, uint16_t y, const uint8_t *str);
void si_frame_invert(si_frame *frame);
void si_frame_invertArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);
void si_frame_invertPageArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max);
void si_frame_clear(si_frame *frame);
void si_frame_clearArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);
void si_frame_clearPageArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max);
void si_frame_set(si_frame *frame);
void si_frame_setArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);
void si_frame_setPageArea(si_frame *frame, uint16_t x_min, uint16_t x_max, uint16_t page_min, uint16_t page_max);

void ssd1306_i2c_directPutFrame(ssd1306_i2c_inst *inst, uint16_t x, uint16_t page_y, si_frame *frame);
void ssd1306_i2c_bufferPutFrame(ssd1306_i2c_inst *inst, uint16_t x, uint16_t y, si_frame *frame);
void ssd1306_i2c_immediatePutFrame(ssd1306_i2c_inst *inst, uint16_t x, uint16_t y, si_frame *frame);

#ifdef _SSD1306_I2C_GT21L16S2Y_H_
// If GT21L16S2Y is used, then extern these functions.

void sifg_writeHanzi(si_frame *frame, sig_inst *sig, const char *hanzi, uint16_t x, uint16_t y);
void sifg_writeHString(si_frame *frame, sig_inst *sig, const char *str, size_t len, uint16_t x, uint16_t y);
void sifg_writeAscii(si_frame *frame, sig_inst *sig, const char *ascii, uint16_t x, uint16_t y);
void sifg_writeString(si_frame *frame, sig_inst *sig, const uint8_t *str, uint16_t x, uint16_t y);

#endif /* _SSD1306_I2C_GT21L16S2Y_H_ */

#endif // _SSD1306_I2C_FRAME_H_