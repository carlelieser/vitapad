#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/power.h>
#include <psp2/usbserial.h>
#include <psp2/types.h>
#include <psp2/mtpif.h>
#include <psp2/usbstorvstor.h>
#include <psp2/appmgr.h>
#include <psp2/power.h>
#include <psp2/shellutil.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/gxm.h>
#include <vita2d.h>
#include <psp2/pgf.h>
#include <psp2/common_dialog.h>
#include <psp2/sysmodule.h>
#include <psp2/avconfig.h>
#include <taihen.h>
#include <time.h>
#include "vitaPadUapi.h"
#include "main.h"

INCLUDE_EXTERN_RESOURCE(logo_active_png);
INCLUDE_EXTERN_RESOURCE(logo_inactive_png);

int DEFAULT_ARM_CLOCK;
int DEFAULT_BUS_CLOCK;
int DEFAULT_GPU_CLOCK;
int DEFAULT_GPU_XBAR_CLOCK;

time_t CURRENT_TIME;
time_t GAMEPAD_LAST_USED;

int GAMEPAD_ACTIVE;
int USB_AND_CONTROLS_LOCKED;

int SECONDS_SINCE_GAMEPAD_LAST_USED;

void startPlugin();
void stopPlugin();
void lockUsbAndControls();
void unlockUsbAndControls();
void activate();
void deactivate();
void init();
void handleControlInput();
void updateCurrentTime();
void checkGamepadLastUsed();
void resetGamepadLastUsed();
void updateGamepadWithTouchpadControls();
void saveSystemClocks();
void applySystemClocks();
void resetSystemClocks();
void applyUnderClock();

int findTouchReport();

vita2d_pgf *FONT = NULL;
vita2d_texture *LOGO_TEXTURE_INACTIVE;
vita2d_texture *LOGO_TEXTURE_ACTIVE;

vita2d_pgf *loadSystemFonts() {
    vita2d_system_pgf_config configs[] = {
            { SCE_FONT_LANGUAGE_DEFAULT, NULL },
    };
    return vita2d_load_system_pgf(1, configs);
}

int main(int argc, char *argv[])
{
    init();
    return 0;
}

void initVita2d() {
	sceSysmoduleLoadModule(SCE_SYSMODULE_PGF);
	vita2d_init_advanced(8 * 1024 * 1024);
	FONT = loadSystemFonts();
	LOGO_TEXTURE_INACTIVE = vita2d_load_PNG_buffer(&_binary_resources_logo_inactive_png_start);
	LOGO_TEXTURE_ACTIVE = vita2d_load_PNG_buffer(&_binary_resources_logo_active_png_start);
}

void finalizeVita2d() {
	vita2d_end_drawing();
	vita2d_swap_buffers();
	vita2d_fini();
	vita2d_free_texture(LOGO_TEXTURE_ACTIVE);
	vita2d_free_texture(LOGO_TEXTURE_INACTIVE);
	vita2d_free_pgf(FONT);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF);
	FONT = NULL;
	LOGO_TEXTURE_ACTIVE = NULL;
	LOGO_TEXTURE_INACTIVE = NULL;
}

void init() {
	initVita2d();
    handleControlInput();
}

void activate() {
    GAMEPAD_ACTIVE = 1;
}

void deactivate() {
    GAMEPAD_ACTIVE = 0;
}

void startPlugin() {
    sceAppMgrDestroyOtherApp();
    sceMtpIfStopDriver(1);
    sceUsbstorVStorStop();
    saveSystemClocks();
    applyUnderClock();
	vitaPadStart();
	activate();
}

void stopPlugin() {
	vitaPadStop();
	finalizeVita2d();
    resetSystemClocks();
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_STOP);
    deactivate();
    unlockUsbAndControls();
    sceAppMgrLoadExec("app0:eboot.bin", NULL, NULL);
}

void lockUsbAndControls() {
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU);
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION);
    USB_AND_CONTROLS_LOCKED = 1;
}

void unlockUsbAndControls() {
    sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
	sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);
	sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU);
	sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION);
	USB_AND_CONTROLS_LOCKED = 0;
}

int findTouchReport(SceTouchData data, int x1, int x2, int y1, int y2) {
	int i;

	for (i = 0; i < data.reportNum; i++) {
		if (data.report[i].x > x1 && data.report[i].x <= x2 && data.report[i].y > y1 && data.report[i].y <= y2) {
			return 1;
		}
	}

	return 0;
}

void displayStatus() {
	char *status;
	int color;

	if (GAMEPAD_ACTIVE) {
		status = "CONNECTED";
		color = STATUS_ACTIVE_COLOR;
	} else {
		status = "NOT CONNECTED";
		color = STATUS_INACTIVE_COLOR;
	}

	pgf_draw_text(SCREEN_HALF_WIDTH - (pgf_text_width(status) / 2), STATUS_Y, color, status);
}

void displayCurrentAction() {
	char *action;
	int color;

	if (GAMEPAD_ACTIVE) {
		action = "PRESS SELECT + START TO DISCONNECT";
		color = CURRENT_ACTION_ACTIVE_COLOR;
	} else {
		action = "PRESS X TO START";
		color = CURRENT_ACTION_INACTIVE_COLOR;
	}

	pgf_draw_text(SCREEN_HALF_WIDTH - (pgf_text_width(action) / 2), ACTION_Y, color, action);
}

void displayLogo() {
	if (GAMEPAD_ACTIVE) {
		vita2d_draw_texture(LOGO_TEXTURE_ACTIVE, SCREEN_HALF_WIDTH - LOGO_HALF_WIDTH, LOGO_Y);
	} else {
		vita2d_draw_texture(LOGO_TEXTURE_INACTIVE, SCREEN_HALF_WIDTH - LOGO_HALF_WIDTH, LOGO_Y);
	}
}

void displayTimeout() {
	SECONDS_SINCE_GAMEPAD_LAST_USED = CURRENT_TIME - GAMEPAD_LAST_USED;
	int timeout = AUTO_DETACH_THRESHOLD - SECONDS_SINCE_GAMEPAD_LAST_USED;
	char status[200];

	if (SECONDS_SINCE_GAMEPAD_LAST_USED >= TIMEOUT_DISPLAY_THRESHOLD) {
		sprintf(status, "AUTO DETACH IN %d SECONDS", timeout);
		pgf_draw_text(SCREEN_HALF_WIDTH - (pgf_text_width(status) / 2), ACTION_Y + 40, TIMEOUT_COLOR, status);
	}
}

void setBackground() {
	if (GAMEPAD_ACTIVE) {
		vita2d_set_clear_color(BACKGROUND_COLOR_ACTIVE);
	} else {
		vita2d_set_clear_color(BACKGROUND_COLOR_INACTIVE);
	}
}

void handleControlInput() {
	SceCtrlData pad;
    SceTouchData touch;

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

    sceShellUtilInitEvents(0);

	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);

		vita2d_start_drawing();
		setBackground();
		vita2d_clear_screen();

		displayLogo();
		displayStatus();
		displayCurrentAction();

		if (GAMEPAD_ACTIVE) {
			updateCurrentTime();
            updateGamepadWithTouchpadControls(touch);
			checkGamepadLastUsed(pad);
			displayTimeout();
			scePowerRequestDisplayOn();

            if (vitaPadUsbAttached() && !USB_AND_CONTROLS_LOCKED) {
                lockUsbAndControls();
            }

            if (((pad.buttons & (SCE_CTRL_START | SCE_CTRL_SELECT)) == (SCE_CTRL_SELECT | SCE_CTRL_START)) || vitaPadUsbAttached() == 0 || (SECONDS_SINCE_GAMEPAD_LAST_USED >= AUTO_DETACH_THRESHOLD)) {
                stopPlugin();
            }
        } else {
            if ((pad.buttons & SCE_CTRL_CROSS) == SCE_CTRL_CROSS) {
                startPlugin();
            }
        }

        vita2d_end_drawing();
		vita2d_swap_buffers();
	}
}

void checkGamepadLastUsed(SceCtrlData pad) {
	int LX = (int) pad.lx - 128;
	int LY = (int) pad.ly - 128;
	int RX = (int) pad.rx - 128;
	int RY = (int) pad.ry - 128;
	int LOW_END_THRESHOLD = -10;
	int HIGH_END_THRESHOLD = 10;

	if ((pad.buttons & SCE_CTRL_UP) || (pad.buttons & SCE_CTRL_DOWN) || (pad.buttons & SCE_CTRL_LEFT) || (pad.buttons & SCE_CTRL_RIGHT) || (pad.buttons & SCE_CTRL_LTRIGGER) || (pad.buttons & SCE_CTRL_RTRIGGER) || (pad.buttons & SCE_CTRL_TRIANGLE) || (pad.buttons & SCE_CTRL_CIRCLE) || (pad.buttons & SCE_CTRL_CROSS) || (pad.buttons & SCE_CTRL_SQUARE) || (pad.buttons & SCE_CTRL_START) || (pad.buttons & SCE_CTRL_SELECT) || (LX < LOW_END_THRESHOLD || LX > HIGH_END_THRESHOLD ) || (LY < LOW_END_THRESHOLD || LY > HIGH_END_THRESHOLD) || (RX < LOW_END_THRESHOLD || RX > HIGH_END_THRESHOLD ) || (RY < LOW_END_THRESHOLD || RY > HIGH_END_THRESHOLD)) {
		resetGamepadLastUsed();
	}
}

void updateGamepadWithTouchpadControls(SceTouchData touch) {
	sceTouchPeek(SCE_TOUCH_PORT_BACK, &touch, 1);
	int L2_PRESSED = findTouchReport(touch, REAR_TOUCHPAD_LEFT_X1, REAR_TOUCHPAD_LEFT_X2, REAR_TOUCHPAD_TOP_Y1, REAR_TOUCHPAD_TOP_Y2);
	int L3_PRESSED = findTouchReport(touch, REAR_TOUCHPAD_LEFT_X1, REAR_TOUCHPAD_LEFT_X2, REAR_TOUCHPAD_BOTTOM_Y1, REAR_TOUCHPAD_BOTTOM_Y2);
	int R2_PRESSED = findTouchReport(touch, REAR_TOUCHPAD_RIGHT_X1, REAR_TOUCHPAD_RIGHT_X2, REAR_TOUCHPAD_TOP_Y1, REAR_TOUCHPAD_TOP_Y2);
	int R3_PRESSED = findTouchReport(touch, REAR_TOUCHPAD_RIGHT_X1, REAR_TOUCHPAD_RIGHT_X2, REAR_TOUCHPAD_BOTTOM_Y1,  REAR_TOUCHPAD_BOTTOM_Y2);

	if (L2_PRESSED || L3_PRESSED || R2_PRESSED || R3_PRESSED) {
		resetGamepadLastUsed();
	}

	vitaPadUpdateL2Pressed(L2_PRESSED);
    vitaPadUpdateR2Pressed(R2_PRESSED);
    vitaPadUpdateL3Pressed(L3_PRESSED);
    vitaPadUpdateR3Pressed(R3_PRESSED);
}

void resetGamepadLastUsed() {
	GAMEPAD_LAST_USED = time(NULL);
}

void updateCurrentTime() {
	CURRENT_TIME = time(NULL);
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