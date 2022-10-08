#ifndef __MAIN_H__
#define __MAIN_H__

#include <vita2d.h>

#define INCLUDE_EXTERN_RESOURCE(name) extern unsigned char _binary_resources_##name##_start; extern unsigned char _binary_resources_##name##_size; \

extern int GAMEPAD_ACTIVE;
extern int CURRENT_SCREEN;

extern void startPlugin();
extern void stopPlugin();
void activate();
void deactivate();
void initTextures();
void initVita2d();
void init();
void freeTextures();
void finishVita2d();
void handleMainUserInput();
void updateTouchPadButtonsPressed();
void updateGamepadWithTouchpadControls();
void setBackground();
extern void setCurrentScreen();

#define LOGO_Y 30
#define ACTIVE_LOGO_Y 80
#define LOGO_WIDTH 500
#define LOGO_HALF_WIDTH (LOGO_WIDTH / 2)
#define STATUS_Y (SCREEN_HEIGHT - (FONT_Y_SPACE * 4))
#define START_ACTION_TEXT_Y (350)

#endif