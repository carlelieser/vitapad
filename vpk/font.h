#ifndef __FONT_H__
#define __FONT_H__

#include <vita2d.h>

extern vita2d_pgf *FONT;

#define FONT_SIZE 1.0f
#define FONT_X_SPACE 15.0f
#define FONT_Y_SPACE 23.0f

#define pgf_draw_text(x, y, color, text) \
  vita2d_pgf_draw_text(FONT, x, (y)+20, color, FONT_SIZE, text)

#define pgf_draw_textf(x, y, color, ...) \
  vita2d_pgf_draw_textf(FONT, x, (y)+20, color, FONT_SIZE, __VA_ARGS__)

#define pgf_text_width(text) \
  vita2d_pgf_text_width(FONT, FONT_SIZE, text)

#define pgf_text_height(text) \
  vita2d_pgf_text_height(FONT, FONT_SIZE, text)

#endif