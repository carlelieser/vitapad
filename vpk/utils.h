#ifndef __UTILS_H__
#define __UTILS_H__

#include <vitasdk.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int USB_AND_CONTROLS_LOCKED;

#define FRONT_PANEL_SCALE 35
#define REAR_PANEL_SCALE 40

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define SCREEN_HALF_WIDTH (SCREEN_WIDTH / 2)
#define SCREEN_HALF_HEIGHT (SCREEN_HEIGHT / 2)
#define SCREEN_MARGIN 20

#define ALIGN_CENTER(a, b) (((a) - (b)) / 2)
#define ALIGN_RIGHT(x, w) ((x) - (w))

#define ANALOG_CENTER 128
#define ANALOG_THRESHOLD 64
#define ANALOG_SENSITIVITY 16

enum PadButtons {
    PAD_UP,
    PAD_DOWN,
    PAD_LEFT,
    PAD_RIGHT,
    PAD_LTRIGGER,
    PAD_RTRIGGER,
    PAD_TRIANGLE,
    PAD_CIRCLE,
    PAD_CROSS,
    PAD_SQUARE,
    PAD_START,
    PAD_SELECT,
    PAD_PSBUTTON,
    PAD_ENTER,
    PAD_CANCEL,
    PAD_LEFT_ANALOG_UP,
    PAD_LEFT_ANALOG_DOWN,
    PAD_LEFT_ANALOG_LEFT,
    PAD_LEFT_ANALOG_RIGHT,
    PAD_RIGHT_ANALOG_UP,
    PAD_RIGHT_ANALOG_DOWN,
    PAD_RIGHT_ANALOG_LEFT,
    PAD_RIGHT_ANALOG_RIGHT,
    PAD_N_BUTTONS
};

typedef uint8_t Pad[PAD_N_BUTTONS];

extern SceCtrlData pad;
extern Pad old_pad, current_pad, pressed_pad, released_pad, hold_pad, hold2_pad;
extern Pad hold_count, hold2_count;

void readPad();

void requestDisplayOn();
void saveSystemClocks();
void applySystemClocks();
void applyUnderClock();
void resetSystemClocks();

void lockUsbAndControls();
void unlockUsbAndControls();

int getCharArraySize();
int getIntArraySize();
int scaleDownInt();
int findIndexSmallest();
void swap();
void selectionSort();
void drawRectangle();

#endif