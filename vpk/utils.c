#include <string.h>
#include <psp2/power.h>
#include <psp2/shellutil.h>
#include "utils.h"

int DEFAULT_ARM_CLOCK;
int DEFAULT_BUS_CLOCK;
int DEFAULT_GPU_CLOCK;
int DEFAULT_GPU_XBAR_CLOCK;

int USB_AND_CONTROLS_LOCKED = 0;

SceCtrlData pad;
Pad old_pad, current_pad, pressed_pad, released_pad, hold_pad, hold2_pad;
Pad hold_count, hold2_count;

void readPad() {
    memset(&pad, 0, sizeof(SceCtrlData));

    kuCtrlReadBufferPositive(&pad, 1);

    memcpy(&old_pad, current_pad, sizeof(Pad));
    memset(&current_pad, 0, sizeof(Pad));

    if (pad.buttons & SCE_CTRL_UP)
        current_pad[PAD_UP] = 1;
    if (pad.buttons & SCE_CTRL_DOWN)
        current_pad[PAD_DOWN] = 1;
    if (pad.buttons & SCE_CTRL_LEFT)
        current_pad[PAD_LEFT] = 1;
    if (pad.buttons & SCE_CTRL_RIGHT)
        current_pad[PAD_RIGHT] = 1;
    if (pad.buttons & SCE_CTRL_LTRIGGER)
        current_pad[PAD_LTRIGGER] = 1;
    if (pad.buttons & SCE_CTRL_RTRIGGER)
        current_pad[PAD_RTRIGGER] = 1;
    if (pad.buttons & SCE_CTRL_TRIANGLE)
        current_pad[PAD_TRIANGLE] = 1;
    if (pad.buttons & SCE_CTRL_CIRCLE)
        current_pad[PAD_CIRCLE] = 1;
    if (pad.buttons & SCE_CTRL_CROSS)
        current_pad[PAD_CROSS] = 1;
    if (pad.buttons & SCE_CTRL_SQUARE)
        current_pad[PAD_SQUARE] = 1;
    if (pad.buttons & SCE_CTRL_START)
        current_pad[PAD_START] = 1;
    if (pad.buttons & SCE_CTRL_SELECT)
        current_pad[PAD_SELECT] = 1;
    if (pad.buttons & SCE_CTRL_PSBUTTON)
        current_pad[PAD_PSBUTTON] = 1;

    if (pad.ly < ANALOG_CENTER - ANALOG_THRESHOLD) {
        current_pad[PAD_LEFT_ANALOG_UP] = 1;
    } else if (pad.ly > ANALOG_CENTER + ANALOG_THRESHOLD) {
        current_pad[PAD_LEFT_ANALOG_DOWN] = 1;
    }

    if (pad.lx < ANALOG_CENTER - ANALOG_THRESHOLD) {
        current_pad[PAD_LEFT_ANALOG_LEFT] = 1;
    } else if (pad.lx > ANALOG_CENTER + ANALOG_THRESHOLD) {
        current_pad[PAD_LEFT_ANALOG_RIGHT] = 1;
    }

    if (pad.ry < ANALOG_CENTER - ANALOG_THRESHOLD) {
        current_pad[PAD_RIGHT_ANALOG_UP] = 1;
    } else if (pad.ry > ANALOG_CENTER + ANALOG_THRESHOLD) {
        current_pad[PAD_RIGHT_ANALOG_DOWN] = 1;
    }

    if (pad.rx < ANALOG_CENTER - ANALOG_THRESHOLD) {
        current_pad[PAD_RIGHT_ANALOG_LEFT] = 1;
    } else if (pad.rx > ANALOG_CENTER + ANALOG_THRESHOLD) {
        current_pad[PAD_RIGHT_ANALOG_RIGHT] = 1;
    }

    int i;
    for (i = 0; i < PAD_N_BUTTONS; i++) {
        pressed_pad[i] = current_pad[i] & ~old_pad[i];
        released_pad[i] = ~current_pad[i] & old_pad[i];

        hold_pad[i] = pressed_pad[i];
        hold2_pad[i] = pressed_pad[i];

        if (current_pad[i]) {
            if (hold_count[i] >= 10) {
                hold_pad[i] = 1;
                hold_count[i] = 6;
            }

            if (hold2_count[i] >= 10) {
                hold2_pad[i] = 1;
                hold2_count[i] = 10;
            }

            hold_count[i]++;
            hold2_count[i]++;
        } else {
            hold_count[i] = 0;
            hold2_count[i] = 0;
        }
    }

    old_pad[PAD_ENTER] = old_pad[PAD_CROSS];
    current_pad[PAD_ENTER] = current_pad[PAD_CROSS];
    pressed_pad[PAD_ENTER] = pressed_pad[PAD_CROSS];
    released_pad[PAD_ENTER] = released_pad[PAD_CROSS];
    hold_pad[PAD_ENTER] = hold_pad[PAD_CROSS];
    hold2_pad[PAD_ENTER] = hold2_pad[PAD_CROSS];

    old_pad[PAD_CANCEL] = old_pad[PAD_CIRCLE];
    current_pad[PAD_CANCEL] = current_pad[PAD_CIRCLE];
    pressed_pad[PAD_CANCEL] = pressed_pad[PAD_CIRCLE];
    released_pad[PAD_CANCEL] = released_pad[PAD_CIRCLE];
    hold_pad[PAD_CANCEL] = hold_pad[PAD_CIRCLE];
    hold2_pad[PAD_CANCEL] = hold2_pad[PAD_CIRCLE];
}

void requestDisplayOn() {
    scePowerRequestDisplayOn();
}

void saveSystemClocks() {
    DEFAULT_ARM_CLOCK = scePowerGetArmClockFrequency();
    DEFAULT_BUS_CLOCK = scePowerGetBusClockFrequency();
    DEFAULT_GPU_CLOCK = scePowerGetGpuClockFrequency();
    DEFAULT_GPU_XBAR_CLOCK = scePowerGetGpuClockFrequency();
}

void applySystemClocks(int arm, int bus, int gpu, int gpuXbar) {
    scePowerSetArmClockFrequency(arm);
    scePowerSetBusClockFrequency(bus);
    scePowerSetGpuClockFrequency(gpu);
    scePowerSetGpuXbarClockFrequency(gpuXbar);
}

void applyUnderClock() {
    applySystemClocks(111, 111, 111, 111);
}

void resetSystemClocks() {
    applySystemClocks(DEFAULT_ARM_CLOCK, DEFAULT_BUS_CLOCK, DEFAULT_GPU_CLOCK, DEFAULT_GPU_XBAR_CLOCK);
}

void lockUsbAndControls() {
    sceShellUtilInitEvents(0);
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU);
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION);
    USB_AND_CONTROLS_LOCKED = 1;
}

void unlockUsbAndControls() {
    sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);
    sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU);
    sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION);
    USB_AND_CONTROLS_LOCKED = 0;
}

int getCharArraySize(char arr[]) {
    return sizeof arr / sizeof arr[0];
}

int getIntArraySize(int arr[]) {
    return sizeof arr / sizeof arr[0];
}

int scaleDownInt (int target, int scale) {
    return (target * scale) / 100;
}

int findIndexSmallest(int *arr[]) {
    int smallest = arr[0];

    for (int i = 0; i < getIntArraySize(arr); i++) {
        if (arr[i]) {
            if (arr[i] < smallest) {
                smallest = (int) arr[i];
            }
        }
    }

    for (int j = 0; j < getIntArraySize(arr); j++) {
        if (arr[j] == smallest) {
            return j;
        }
    }

    return 0;
}

void swap(int* xp, int* yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void selectionSort(int arr[], int n)
{
    int i, j, min_idx;
    for (i = 0; i < n - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;
        swap(&arr[min_idx], &arr[i]);
    }
}