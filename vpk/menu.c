#include "string.h"
#include "stdio.h"
#include "settings.h"
#include "font.h"
#include "color.h"
#include "touchpad.h"
#include "modal.h"
#include "menu.h"
#include "utils.h"
#include "main.h"

char *MENU_ITEMS[4] = {"START", "ACTIVE DISPLAY", "TOUCHPAD CONTROLS", "RESET TOUCHPAD CONTROLS"};
int CURRENT_MENU_ITEM = 0;

int menuItemSelected(char *id) {
    return strcmp(MENU_ITEMS[CURRENT_MENU_ITEM], id) == 0;
}

int getMenuItemIndex(char *label) {
    for (int i = 0; i < (sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0])); ++i) {
        if (strcmp(MENU_ITEMS[i], label) == 0) return i;
    }
    return 0;
}

void setCurrentMenuItem(int index) {
    CURRENT_MENU_ITEM = index;
}

void handleMenuActions() {
    if (modalActive()) return;
    if (pressed_pad[PAD_CROSS]) {
        if (menuItemSelected("START")) {
            startPlugin();
        }
        if (menuItemSelected("ACTIVE DISPLAY")) {
            toggleActiveDisplay();
            saveSettings();
        }
        if (menuItemSelected("TOUCHPAD CONTROLS")) {
            setCurrentScreen(2);
        }
        if (menuItemSelected("RESET TOUCHPAD CONTROLS")) {
            openTouchPadControlResetToDefaultSettingsModal();
        }
    } else if (pressed_pad[PAD_UP]) {
        if (CURRENT_MENU_ITEM != 0) {
            CURRENT_MENU_ITEM = CURRENT_MENU_ITEM - 1;
        }
    } else if (pressed_pad[PAD_DOWN]) {
        if (CURRENT_MENU_ITEM < ((sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]) - 1))) {
            if (!checkTouchPadControlsChanged(DEFAULT_TOUCHPAD_CONTROL_SETTINGS, TOUCHPAD_CONTROL_SETTINGS) && CURRENT_MENU_ITEM == 2) {
                return;
            }
            CURRENT_MENU_ITEM += 1;
        }
    }
}

void displayMenu(int top) {
    for (int i = 0; i < (sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0])); ++i) {
        char text[MAX_MENU_ITEM_TITLE_LENGTH];

        if (strcmp(MENU_ITEMS[i], "ACTIVE DISPLAY") == 0) {
            sprintf(text, "%s %s", MENU_ITEMS[i], SETTINGS.ACTIVE_DISPLAY ? "ON" : "OFF");
        } else {
            strcpy(text, MENU_ITEMS[i]);
        }

        int active = i == CURRENT_MENU_ITEM;
        int titleX = SCREEN_HALF_WIDTH - (pgf_text_width(text) / 2);
        int titleY = top + (i * (FONT_Y_SPACE + 20));

        if (strcmp(MENU_ITEMS[i], "RESET TOUCHPAD CONTROLS") == 0) {
            if (checkTouchPadControlsChanged(DEFAULT_TOUCHPAD_CONTROL_SETTINGS, TOUCHPAD_CONTROL_SETTINGS)) {
                pgf_draw_text(titleX, titleY, active ? BLACK : BLACK_OPACITY_30, text);
            }
        } else {
            pgf_draw_text(titleX, titleY, active ? BLACK : BLACK_OPACITY_30, text);
        }
    }
}
