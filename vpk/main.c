#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/usbserial.h>
#include <psp2/types.h>
#include <psp2/mtpif.h>
#include <psp2/usbstorvstor.h>
#include <psp2/appmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/gxm.h>
#include <psp2/pgf.h>
#include <psp2/common_dialog.h>
#include <psp2/sysmodule.h>
#include <psp2/avconfig.h>
#include <taihen.h>
#include <time.h>
#include "vitaPadUapi.h"
#include "main.h"
#include "settings.h"
#include "color.h"
#include "utils.h"
#include "touchpad.h"
#include "modal.h"
#include "menu.h"
#include "font.h"

INCLUDE_EXTERN_RESOURCE(images_logo_active_png);
INCLUDE_EXTERN_RESOURCE(images_logo_inactive_png);
INCLUDE_EXTERN_RESOURCE(images_bg_inactive_png);

int GAMEPAD_ACTIVE = 0;
int CURRENT_SCREEN = 0;

vita2d_texture *LOGO_TEXTURE_INACTIVE;
vita2d_texture *LOGO_TEXTURE_ACTIVE;
vita2d_texture *BG_TEXTURE_INACTIVE;

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

void initTextures() {
    LOGO_TEXTURE_INACTIVE = vita2d_load_PNG_buffer(&_binary_resources_images_logo_inactive_png_start);
    LOGO_TEXTURE_ACTIVE = vita2d_load_PNG_buffer(&_binary_resources_images_logo_active_png_start);
    BG_TEXTURE_INACTIVE = vita2d_load_PNG_buffer(&_binary_resources_images_bg_inactive_png_start);
}

void initVita2d() {
	sceSysmoduleLoadModule(SCE_SYSMODULE_PGF);
	vita2d_init_advanced(8 * 1024 * 1024);
	FONT = loadSystemFonts();
    initTextures();
    initTouchPadTextures();
}

void freeTextures() {
    vita2d_free_texture(LOGO_TEXTURE_ACTIVE);
    vita2d_free_texture(LOGO_TEXTURE_INACTIVE);
    vita2d_free_texture(BG_TEXTURE_INACTIVE);
    freeTouchPadTextures();
    LOGO_TEXTURE_ACTIVE = NULL;
    LOGO_TEXTURE_INACTIVE = NULL;
    BG_TEXTURE_INACTIVE = NULL;
}

void finishVita2d() {
	vita2d_end_drawing();
	vita2d_swap_buffers();
	vita2d_fini();
    freeTextures();
	vita2d_free_pgf(FONT);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF);
	FONT = NULL;
}

void init() {
    initSettings();
    loadSettings();
    loadTouchPadControlSettings();
	initVita2d();
    updateTouchPanelInfo();
    initHelpBarItems();
    handleMainUserInput();
}

void activate() {
    GAMEPAD_ACTIVE = 1;
}

void deactivate() {
    GAMEPAD_ACTIVE = 0;
}

void startPlugin() {
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceAppMgrDestroyOtherApp();
    sceMtpIfStopDriver(1);
    sceUsbstorVStorStop();
    saveSystemClocks();
    applyUnderClock();
	vitaPadStart(SETTINGS.XINPUT_MAPPING);
	activate();
    setCurrentScreen(1);
}

void stopPlugin() {
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_STOP);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_STOP);
	vitaPadStop();
	finishVita2d();
    resetSystemClocks();
    deactivate();
    unlockUsbAndControls();
    sceAppMgrLoadExec("app0:eboot.bin", NULL, NULL);
}

void setCurrentScreen(int screen) {
    CURRENT_SCREEN = screen;
}

void displayStatus() {
	char *status = GAMEPAD_ACTIVE ? "CONNECTED" : "DISCONNECTED";
    int x = 20;
    int y = SCREEN_HEIGHT - (FONT_Y_SPACE + SCREEN_MARGIN);
    
	if (GAMEPAD_ACTIVE) {
        if (SETTINGS.ACTIVE_DISPLAY) {
            pgf_draw_text(x, y , NAPLES_YELLOW, status);
        }
	} else {
        pgf_draw_text(x, y, BLACK_OPACITY_30, status);
	}
}

void displayVersion() {
    char *version = "v.2.0.0";
    int x = SCREEN_WIDTH - (pgf_text_width(version) + 20);
    int y = SCREEN_HEIGHT - (FONT_Y_SPACE + SCREEN_MARGIN);

    if (GAMEPAD_ACTIVE) {
        if (SETTINGS.ACTIVE_DISPLAY) {
            pgf_draw_text(x, y, WHITE_OPACITY_30, version);
        }
    } else {
        pgf_draw_text(x, y, BLACK_OPACITY_30, version);
    }
}

void displayCurrentAction() {
	if (GAMEPAD_ACTIVE) {
        if (SETTINGS.ACTIVE_DISPLAY) {
            char text[] = "PRESS PS BUTTON TO DISCONNECT";
            pgf_draw_text(SCREEN_HALF_WIDTH - (pgf_text_width(text) / 2), START_ACTION_TEXT_Y, WHITE, text);
        }
	}
}

void displayLogo() {
	if (GAMEPAD_ACTIVE) {
		if (SETTINGS.ACTIVE_DISPLAY) {
            vita2d_draw_texture(LOGO_TEXTURE_ACTIVE, SCREEN_HALF_WIDTH - LOGO_HALF_WIDTH, ACTIVE_LOGO_Y);
        }
	} else {
		vita2d_draw_texture(LOGO_TEXTURE_INACTIVE, SCREEN_HALF_WIDTH - LOGO_HALF_WIDTH, LOGO_Y);
	}
}

int checkButtonPressed(int index) {
    if (index == 0) return L2_PRESSED;
    if (index == 1) return L3_PRESSED;
    if (index == 2) return R2_PRESSED;
    if (index == 3) return R3_PRESSED;

    return 0;
}

void setBackgroundColor(int color) {
    vita2d_set_clear_color(color);
}

void handleMainUserInput() {
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

	while (1) {
        vita2d_start_drawing();
        vita2d_clear_screen();

        if (CURRENT_SCREEN == 0) {
            vita2d_draw_texture(BG_TEXTURE_INACTIVE, 0, 0);
            displayLogo();
            displayStatus();
            displayVersion();
            displayMenu(280);
            readPad();

            handleMenuActions();
        }

        if (CURRENT_SCREEN == 1) {
            setBackgroundColor(BLACK);
            displayLogo();
            displayStatus();
            displayVersion();
            displayCurrentAction();
            readPad();
            updateTouchPadButtonsPressed();
            updateGamepadWithTouchpadControls();
            vitaPadPreventSleep();
            requestDisplayOn();

            if (vitaPadUsbAttached() && !USB_AND_CONTROLS_LOCKED) {
                lockUsbAndControls();
            }

            if (pressed_pad[PAD_PSBUTTON] || vitaPadUsbAttached() == 0) {
                stopPlugin();
            }
        }

        if (CURRENT_SCREEN == 2) {
            setBackgroundColor(TOUCHPAD_CONTROLS_BG);
            readPad();
            updateTouchPadButtonsPressed();
            updateHelpBarItems();
            displayTouchPadControlSettings();
            handleTouchPadControlSelectionAndPlacement();
            handleTouchPadControlHelpBarActions();
        }

        handleModalDialogs();
        displayModalDialogs();

        vita2d_end_drawing();
        vita2d_swap_buffers();
	}
}

void updateGamepadWithTouchpadControls() {
	vitaPadUpdateL2Pressed(L2_PRESSED);
    vitaPadUpdateR2Pressed(R2_PRESSED);
    vitaPadUpdateL3Pressed(L3_PRESSED);
    vitaPadUpdateR3Pressed(R3_PRESSED);
}