#ifndef __MAIN_H__
#define __MAIN_H__

#define INCLUDE_EXTERN_RESOURCE(name) extern unsigned char _binary_resources_##name##_start; extern unsigned char _binary_resources_##name##_size; \

#define REAR_TOUCHPAD_CONTROLS_WIDTH 600
#define REAR_TOUCHPAD_CONTROLS_HEIGHT 200
#define REAR_TOUCHPAD_LEFT_X1 250
#define REAR_TOUCHPAD_RIGHT_X1 1100
#define REAR_TOUCHPAD_TOP_Y1 100
#define REAR_TOUCHPAD_BOTTOM_Y1 700
#define REAR_TOUCHPAD_LEFT_X2 (REAR_TOUCHPAD_LEFT_X1 + REAR_TOUCHPAD_CONTROLS_WIDTH)
#define REAR_TOUCHPAD_RIGHT_X2 (REAR_TOUCHPAD_RIGHT_X1 + REAR_TOUCHPAD_CONTROLS_WIDTH)
#define REAR_TOUCHPAD_TOP_Y2 (REAR_TOUCHPAD_TOP_Y1 + REAR_TOUCHPAD_CONTROLS_HEIGHT)
#define REAR_TOUCHPAD_BOTTOM_Y2 (REAR_TOUCHPAD_BOTTOM_Y1 + REAR_TOUCHPAD_CONTROLS_HEIGHT)

#define AUTO_DETACH_THRESHOLD (60 * 2)
#define TIMEOUT_DISPLAY_THRESHOLD 30

#define BACKGROUND_COLOR_INACTIVE RGBA8(0xFF, 0xFF, 0xFF, 0xFF)
#define STATUS_INACTIVE_COLOR RGBA8(0x0, 0x0, 0x0, 0x50)
#define CURRENT_ACTION_INACTIVE_COLOR RGBA8(0x0, 0x0, 0x0, 0xFF)

#define BACKGROUND_COLOR_ACTIVE RGBA8(0x0, 0x0, 0x0, 0x0)
#define STATUS_ACTIVE_COLOR RGBA8(0xF9, 0xDB, 0x6D, 0xC8)
#define CURRENT_ACTION_ACTIVE_COLOR RGBA8(0xEE, 0xE5, 0xE5, 0xFF)
#define TIMEOUT_COLOR RGBA8(0xFF, 0xFF, 0xFF, 0xC8)

#define FONT_SIZE 1.0f
#define FONT_X_SPACE 15.0f
#define FONT_Y_SPACE 23.0f

#define LOGO_Y 74
#define LOGO_WIDTH 500
#define LOGO_HALF_WIDTH (LOGO_WIDTH / 2)
#define STATUS_Y 320
#define ACTION_Y (STATUS_Y + 40)

#define pgf_draw_text(x, y, color, text) \
  vita2d_pgf_draw_text(FONT, x, (y)+20, color, FONT_SIZE, text)

#define pgf_draw_textf(x, y, color, ...) \
  vita2d_pgf_draw_textf(FONT, x, (y)+20, color, FONT_SIZE, __VA_ARGS__)

#define pgf_text_width(text) \
  vita2d_pgf_text_width(FONT, FONT_SIZE, text)

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define SCREEN_HALF_WIDTH (SCREEN_WIDTH / 2)
#define SCREEN_HALF_HEIGHT (SCREEN_HEIGHT / 2)

#endif