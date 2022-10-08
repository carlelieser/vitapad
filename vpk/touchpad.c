#include <psp2/touch.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include "settings.h"
#include "color.h"
#include "font.h"
#include "touchpad.h"
#include "modal.h"

INCLUDE_EXTERN_RESOURCE(images_select_button_png);
INCLUDE_EXTERN_RESOURCE(images_start_button_png);
INCLUDE_EXTERN_RESOURCE(images_triangle_button_png);
INCLUDE_EXTERN_RESOURCE(images_square_button_png);
INCLUDE_EXTERN_RESOURCE(images_cross_button_png);
INCLUDE_EXTERN_RESOURCE(images_circle_button_png);

int L2_PRESSED = 0;
int L3_PRESSED = 0;
int R2_PRESSED = 0;
int R3_PRESSED = 0;

int REAR_TOUCHPAD_VISIBLE = 1;

char CURRENT_TOUCHPAD_BUTTON_HOVERING[3] = "L2";
char CURRENT_TOUCHPAD_BUTTON_SELECTED[3] = "";

int BUTTON_PLACEMENT_STEP = 40;

struct SceTouchPanelInfo FRONT_PANEL_INFO;
struct SceTouchPanelInfo REAR_PANEL_INFO;
struct SceTouchPanelInfo FULL_FRONT_PANEL_INFO;
struct SceTouchPanelInfo FULL_REAR_PANEL_INFO;

struct HelpBarItem HELP_BAR_ITEMS[5] = {};

char *TouchPadButtons[4] = {"L2", "L3", "R2", "R3"};

int STATUS_BAR_HEIGHT = FONT_Y_SPACE + 20;

vita2d_texture *SELECT_BUTTON_TEXTURE = NULL;
vita2d_texture *START_BUTTON_TEXTURE = NULL;
vita2d_texture *TRIANGLE_BUTTON_TEXTURE = NULL;
vita2d_texture *SQUARE_BUTTON_TEXTURE = NULL;
vita2d_texture *CROSS_BUTTON_TEXTURE = NULL;
vita2d_texture *CIRCLE_BUTTON_TEXTURE = NULL;

void initTouchPadTextures() {
    SELECT_BUTTON_TEXTURE = vita2d_load_PNG_buffer(&_binary_resources_images_select_button_png_start);
    START_BUTTON_TEXTURE = vita2d_load_PNG_buffer(&_binary_resources_images_start_button_png_start);
    TRIANGLE_BUTTON_TEXTURE = vita2d_load_PNG_buffer(&_binary_resources_images_triangle_button_png_start);
    SQUARE_BUTTON_TEXTURE = vita2d_load_PNG_buffer(&_binary_resources_images_square_button_png_start);
    CROSS_BUTTON_TEXTURE = vita2d_load_PNG_buffer(&_binary_resources_images_cross_button_png_start);
    CIRCLE_BUTTON_TEXTURE = vita2d_load_PNG_buffer(&_binary_resources_images_circle_button_png_start);
}

void freeTouchPadTextures() {
    vita2d_free_texture(SELECT_BUTTON_TEXTURE);
    vita2d_free_texture(START_BUTTON_TEXTURE);
    vita2d_free_texture(TRIANGLE_BUTTON_TEXTURE);
    vita2d_free_texture(SQUARE_BUTTON_TEXTURE);
    vita2d_free_texture(CROSS_BUTTON_TEXTURE);
    vita2d_free_texture(CIRCLE_BUTTON_TEXTURE);
    SELECT_BUTTON_TEXTURE = NULL;
    START_BUTTON_TEXTURE = NULL;
    TRIANGLE_BUTTON_TEXTURE = NULL;
    SQUARE_BUTTON_TEXTURE = NULL;
    CROSS_BUTTON_TEXTURE = NULL;
    CIRCLE_BUTTON_TEXTURE = NULL;
}

void updateHelpBarItems() {
    if (strcmp(CURRENT_TOUCHPAD_BUTTON_SELECTED, "") == 0) {
        HELP_BAR_ITEMS[2].title = "SELECT";
        HELP_BAR_ITEMS[3].title = "SWITCH TOUCHPAD";
    } else {
        HELP_BAR_ITEMS[2].title = "PLACE";
        if (REAR_TOUCHPAD_VISIBLE) {
            HELP_BAR_ITEMS[3].title = "MOVE TO FRONT";
        } else {
            HELP_BAR_ITEMS[3].title = "MOVE TO BACK";
        }
    }
}

void initHelpBarItems() {
    HELP_BAR_ITEMS[0].texture = SELECT_BUTTON_TEXTURE;
    HELP_BAR_ITEMS[0].textureWidth = 50;
    HELP_BAR_ITEMS[0].title = "RESET";

    HELP_BAR_ITEMS[1].texture = START_BUTTON_TEXTURE;
    HELP_BAR_ITEMS[1].textureWidth = 50;
    HELP_BAR_ITEMS[1].title = "SAVE";

    HELP_BAR_ITEMS[2].texture = CROSS_BUTTON_TEXTURE;
    HELP_BAR_ITEMS[2].textureWidth = 24;
    HELP_BAR_ITEMS[2].title = "SELECT";

    HELP_BAR_ITEMS[3].texture = SQUARE_BUTTON_TEXTURE;
    HELP_BAR_ITEMS[3].textureWidth = 24;
    HELP_BAR_ITEMS[3].title = "TOGGLE";

    HELP_BAR_ITEMS[4].texture = TRIANGLE_BUTTON_TEXTURE;
    HELP_BAR_ITEMS[4].textureWidth = 24;
    HELP_BAR_ITEMS[4].title = "EXIT";
}

void updateTouchPanelInfo() {
    FRONT_PANEL_INFO = getScaledTouchPanelInfo(SCE_TOUCH_PORT_FRONT, FRONT_PANEL_SCALE);
    REAR_PANEL_INFO = getScaledTouchPanelInfo(SCE_TOUCH_PORT_BACK, REAR_PANEL_SCALE);
    FULL_FRONT_PANEL_INFO = getScaledTouchPanelInfo(SCE_TOUCH_PORT_FRONT, 100);
    FULL_REAR_PANEL_INFO = getScaledTouchPanelInfo(SCE_TOUCH_PORT_BACK, 100);
}

void getButtonDistances(int *distances[], struct TouchPadButtonData buttons[]) {
    struct TouchPadButtonData currentButton = getTouchPadButtonConfig(getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING));

    for (int i = 0; i < getCharArraySize(buttons); ++i) {
        if (buttons[i].id) {
            int distance = 0;
            int xDistance = 0;
            int yDistance = 0;

            if (buttons[i].x1 <= currentButton.x2 || buttons[i].x2 <= currentButton.x2) {
                xDistance = currentButton.x1 - buttons[i].x2;
            }

            if (buttons[i].x1 >= currentButton.x1 || buttons[i].x2 >= currentButton.x1) {
                xDistance = buttons[i].x1 - currentButton.x2;
            }

            if (buttons[i].y1 <= currentButton.y2 || buttons[i].y2 <= currentButton.y2) {
                yDistance = currentButton.y2 - buttons[i].y1;
            }

            if (buttons[i].y1 >= currentButton.y1 || buttons[i].y2 >= currentButton.y1) {
                yDistance = buttons[i].y2 - currentButton.y1;
            }

            distance = xDistance + yDistance;
            distances[i] = distance;
        } else {
            distances[i] = 99999999;
        }
    }
}

void filterButtons(char *buttons[], struct TouchPadButtonData filtered[], int x1, int y1, int x2, int y2) {
    for (int i = 0; i < getCharArraySize(buttons); ++i) {
        if (buttons[i]) {
            if (strcmp(CURRENT_TOUCHPAD_BUTTON_HOVERING, buttons[i]) != 0) {
                struct TouchPadButtonData button = getTouchPadButtonConfig(getIndexOfTouchPadButton(buttons[i]));
                if (button.x1 >= x1 && button.x2 <= x2 && button.y1 >= y1 && button.y2 <= y2) {
                    filtered[i] = button;
                }
            }
        }
    }
}

struct TouchPadButtonData findButton(int front, int x1, int y1, int x2, int y2) {
    struct TouchPadButtonData button;
    char *buttonsOnTouchPad[4] = {};
    getButtonsForTouchPad(front, &buttonsOnTouchPad);

    struct TouchPadButtonData filtered[4] = {};
    filterButtons(buttonsOnTouchPad, &filtered, x1, y1, x2, y2);

    int distances[4] = {};
    int sortedDistances[4] = {};

    getButtonDistances(&distances, filtered);

    memcpy(sortedDistances, distances, sizeof(distances));
    selectionSort(sortedDistances, getCharArraySize(sortedDistances));

    int foundIndex = -1;

    for (int i = 0; i < getCharArraySize(distances); ++i) {
        if (distances[i] == sortedDistances[0]) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1 && foundIndex != getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING)) {
        if (filtered[foundIndex].id) {
            return filtered[foundIndex];
        }
    }

    button.id = "0";

    return button;
}

const int PADDING = 1;
const int NEGATIVE_PADDING = -1 * PADDING;

void selectButtonFromCoordinates(int x1, int y1, int x2, int y2) {
    struct TouchPadButtonData button = findButton(!REAR_TOUCHPAD_VISIBLE, x1, y1, x2, y2);
    if (strcmp(button.id, "0")) strcpy(CURRENT_TOUCHPAD_BUTTON_HOVERING, button.id);
}

void hoverRight(struct TouchPadButtonData currentButton, SceTouchPanelInfo currentPanel) {
    selectButtonFromCoordinates(currentButton.x1 + PADDING, NEGATIVE_PADDING, currentPanel.maxAaX + PADDING, currentPanel.maxAaY + PADDING);
}

void hoverLeft(struct TouchPadButtonData currentButton, SceTouchPanelInfo currentPanel) {
    selectButtonFromCoordinates(NEGATIVE_PADDING, NEGATIVE_PADDING, currentButton.x2 - PADDING, currentPanel.maxAaY + PADDING);
}

void hoverUp(struct TouchPadButtonData currentButton, SceTouchPanelInfo currentPanel) {
    selectButtonFromCoordinates(NEGATIVE_PADDING, NEGATIVE_PADDING, currentPanel.maxAaX + PADDING, currentButton.y2 - PADDING);
}

void hoverDown(struct TouchPadButtonData currentButton, SceTouchPanelInfo currentPanel) {
    selectButtonFromCoordinates(NEGATIVE_PADDING, currentButton.y1 + PADDING,currentPanel.maxAaX + PADDING, currentPanel.maxAaY + PADDING);
}

void updateButtonPlacement(int *start, int *end, int add, int condition, int minStart, int maxEnd) {
    *start = *start + (add ? BUTTON_PLACEMENT_STEP : -1 * BUTTON_PLACEMENT_STEP);
    *end = *end + (add ? BUTTON_PLACEMENT_STEP : -1 * BUTTON_PLACEMENT_STEP);
    if (condition) {
        *start = minStart;
        *end = maxEnd;
    }
}

void moveCurrentButtonDown() {
    int index = getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING);
    struct SceTouchPanelInfo panel = getScaledTouchPanelInfo(REAR_TOUCHPAD_VISIBLE ? SCE_TOUCH_PORT_BACK : SCE_TOUCH_PORT_FRONT, 100);
    switch(index) {
        case 0:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.L2_Y1, &TOUCHPAD_CONTROL_SETTINGS.L2_Y2, 1, TOUCHPAD_CONTROL_SETTINGS.L2_Y2 + BUTTON_PLACEMENT_STEP >= panel.maxAaY, panel.maxAaY - (TOUCHPAD_CONTROL_SETTINGS.L2_Y2 - TOUCHPAD_CONTROL_SETTINGS.L2_Y1), panel.maxAaY);
            break;
        case 1:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.L3_Y1, &TOUCHPAD_CONTROL_SETTINGS.L3_Y2, 1, TOUCHPAD_CONTROL_SETTINGS.L3_Y2 + BUTTON_PLACEMENT_STEP >= panel.maxAaY, panel.maxAaY - (TOUCHPAD_CONTROL_SETTINGS.L3_Y2 - TOUCHPAD_CONTROL_SETTINGS.L3_Y1), panel.maxAaY);
            break;
        case 2:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.R2_Y1, &TOUCHPAD_CONTROL_SETTINGS.R2_Y2, 1, TOUCHPAD_CONTROL_SETTINGS.R2_Y2 + BUTTON_PLACEMENT_STEP >= panel.maxAaY, panel.maxAaY - (TOUCHPAD_CONTROL_SETTINGS.R2_Y2 - TOUCHPAD_CONTROL_SETTINGS.R2_Y1), panel.maxAaY);
            break;
        case 3:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.R3_Y1, &TOUCHPAD_CONTROL_SETTINGS.R3_Y2, 1, TOUCHPAD_CONTROL_SETTINGS.R3_Y2 + BUTTON_PLACEMENT_STEP >= panel.maxAaY, panel.maxAaY - (TOUCHPAD_CONTROL_SETTINGS.R3_Y2 - TOUCHPAD_CONTROL_SETTINGS.R3_Y1), panel.maxAaY);
            break;
    }
}

void moveCurrentButtonRight() {
    int index = getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING);
    struct SceTouchPanelInfo panel = getScaledTouchPanelInfo(REAR_TOUCHPAD_VISIBLE ? SCE_TOUCH_PORT_BACK : SCE_TOUCH_PORT_FRONT, 100);
    switch(index) {
        case 0:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.L2_X1, &TOUCHPAD_CONTROL_SETTINGS.L2_X2, 1, TOUCHPAD_CONTROL_SETTINGS.L2_X2 + BUTTON_PLACEMENT_STEP >= panel.maxAaX, panel.maxAaX - (TOUCHPAD_CONTROL_SETTINGS.L2_X2 - TOUCHPAD_CONTROL_SETTINGS.L2_X1), panel.maxAaX);
            break;
        case 1:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.L3_X1, &TOUCHPAD_CONTROL_SETTINGS.L3_X2, 1, TOUCHPAD_CONTROL_SETTINGS.L3_X2 + BUTTON_PLACEMENT_STEP >= panel.maxAaX, panel.maxAaX - (TOUCHPAD_CONTROL_SETTINGS.L3_X2 - TOUCHPAD_CONTROL_SETTINGS.L3_X1), panel.maxAaX);
            break;
        case 2:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.R2_X1, &TOUCHPAD_CONTROL_SETTINGS.R2_X2, 1, TOUCHPAD_CONTROL_SETTINGS.R2_X2 + BUTTON_PLACEMENT_STEP >= panel.maxAaX, panel.maxAaX - (TOUCHPAD_CONTROL_SETTINGS.R2_X2 - TOUCHPAD_CONTROL_SETTINGS.R2_X1), panel.maxAaX);
            break;
        case 3:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.R3_X1, &TOUCHPAD_CONTROL_SETTINGS.R3_X2, 1, TOUCHPAD_CONTROL_SETTINGS.R3_X2 + BUTTON_PLACEMENT_STEP >= panel.maxAaX, panel.maxAaX - (TOUCHPAD_CONTROL_SETTINGS.R3_X2 - TOUCHPAD_CONTROL_SETTINGS.R3_X1), panel.maxAaX);
            break;
    }
}

void moveCurrentButtonLeft() {
    int index = getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING);
    switch(index) {
        case 0:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.L2_X1, &TOUCHPAD_CONTROL_SETTINGS.L2_X2, 0, TOUCHPAD_CONTROL_SETTINGS.L2_X1 - BUTTON_PLACEMENT_STEP <= 0, 0, TOUCHPAD_CONTROL_SETTINGS.L2_X2 - TOUCHPAD_CONTROL_SETTINGS.L2_X1);
            break;
        case 1:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.L3_X1, &TOUCHPAD_CONTROL_SETTINGS.L3_X2, 0, TOUCHPAD_CONTROL_SETTINGS.L3_X1 - BUTTON_PLACEMENT_STEP <= 0, 0, TOUCHPAD_CONTROL_SETTINGS.L3_X2 - TOUCHPAD_CONTROL_SETTINGS.L3_X1);
            break;
        case 2:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.R2_X1, &TOUCHPAD_CONTROL_SETTINGS.R2_X2, 0, TOUCHPAD_CONTROL_SETTINGS.R2_X1 - BUTTON_PLACEMENT_STEP <= 0, 0, TOUCHPAD_CONTROL_SETTINGS.R2_X2 - TOUCHPAD_CONTROL_SETTINGS.R2_X1);
            break;
        case 3:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.R3_X1, &TOUCHPAD_CONTROL_SETTINGS.R3_X2, 0, TOUCHPAD_CONTROL_SETTINGS.R3_X1 - BUTTON_PLACEMENT_STEP <= 0, 0, TOUCHPAD_CONTROL_SETTINGS.R3_X2 - TOUCHPAD_CONTROL_SETTINGS.R3_X1);
            break;
    }
}

void moveCurrentButtonUp() {
    int index = getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING);
    switch(index) {
        case 0:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.L2_Y1, &TOUCHPAD_CONTROL_SETTINGS.L2_Y2, 0, TOUCHPAD_CONTROL_SETTINGS.L2_Y1 - BUTTON_PLACEMENT_STEP <= 0, 0, TOUCHPAD_CONTROL_SETTINGS.L2_Y2 - TOUCHPAD_CONTROL_SETTINGS.L2_Y1);
            break;
        case 1:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.L3_Y1, &TOUCHPAD_CONTROL_SETTINGS.L3_Y2, 0, TOUCHPAD_CONTROL_SETTINGS.L3_Y1 - BUTTON_PLACEMENT_STEP <= 0, 0, TOUCHPAD_CONTROL_SETTINGS.L3_Y2 - TOUCHPAD_CONTROL_SETTINGS.L3_Y1);
            break;
        case 2:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.R2_Y1, &TOUCHPAD_CONTROL_SETTINGS.R2_Y2, 0, TOUCHPAD_CONTROL_SETTINGS.R2_Y1 - BUTTON_PLACEMENT_STEP <= 0, 0, TOUCHPAD_CONTROL_SETTINGS.R2_Y2 - TOUCHPAD_CONTROL_SETTINGS.R2_Y1);
            break;
        case 3:
            updateButtonPlacement(&TOUCHPAD_CONTROL_SETTINGS.R3_Y1, &TOUCHPAD_CONTROL_SETTINGS.R3_Y2, 0, TOUCHPAD_CONTROL_SETTINGS.R3_Y1 - BUTTON_PLACEMENT_STEP <= 0, 0, TOUCHPAD_CONTROL_SETTINGS.R3_Y2 - TOUCHPAD_CONTROL_SETTINGS.R3_Y1);
            break;
    }
}

void handleTouchPadControlSelectionAndPlacement() {
    struct TouchPadButtonData currentButton = getTouchPadButtonConfig(getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING));
    struct SceTouchPanelInfo currentPanel = REAR_TOUCHPAD_VISIBLE ? getCenteredRearTouchPadInfo() : getCenteredFrontTouchPadInfo();
    int touchPadButtonSelected = strcmp(CURRENT_TOUCHPAD_BUTTON_SELECTED, "") != 0;

    if (modalActive()) return;

    if (pressed_pad[PAD_RIGHT]) {
        if (touchPadButtonSelected) {
            moveCurrentButtonRight();
        } else {
            hoverRight(currentButton, currentPanel);
        }
    }

    if (pressed_pad[PAD_LEFT]) {
        if (touchPadButtonSelected) {
            moveCurrentButtonLeft();
        } else {
            hoverLeft(currentButton, currentPanel);
        }
    }

    if (pressed_pad[PAD_UP]) {
        if (touchPadButtonSelected) {
            moveCurrentButtonUp();
        } else {
            hoverUp(currentButton, currentPanel);
        }
    }

    if (pressed_pad[PAD_DOWN]) {
        if (touchPadButtonSelected) {
            moveCurrentButtonDown();
        } else {
            hoverDown(currentButton, currentPanel);
        }
    }

    if (pressed_pad[PAD_SQUARE]) {
        if (strcmp(CURRENT_TOUCHPAD_BUTTON_SELECTED, "") != 0) {
            toggleButtonPlacement();
            strcpy(CURRENT_TOUCHPAD_BUTTON_SELECTED, "");
        } else {
            setRearTouchPadVisibility(!REAR_TOUCHPAD_VISIBLE);
            setButtonHoverForActiveTouchPad();
        }
    }

    if (pressed_pad[PAD_CROSS]) {
        if (strcmp(CURRENT_TOUCHPAD_BUTTON_SELECTED, "") != 0) {
            strcpy(CURRENT_TOUCHPAD_BUTTON_SELECTED, "");
        } else {
            strcpy(CURRENT_TOUCHPAD_BUTTON_SELECTED, CURRENT_TOUCHPAD_BUTTON_HOVERING);
        }
    }

    if (pressed_pad[PAD_START]) {
        if (checkTouchPadControlsChanged(CLEAN_TOUCHPAD_CONTROL_SETTINGS, TOUCHPAD_CONTROL_SETTINGS)) {
            setCleanSettingsUpdated(0);
            saveTouchPadControlSettings();
            openTouchPadControlSaveConfirmationModal();
        }
    }

    if (pressed_pad[PAD_SELECT]) {
        if (checkTouchPadControlsChanged(CLEAN_TOUCHPAD_CONTROL_SETTINGS, TOUCHPAD_CONTROL_SETTINGS)) {
            openTouchPadControlResetToCleanSettingsModal();
        }
    }
}

void toggleRearTouchPadVisible() {
    REAR_TOUCHPAD_VISIBLE = REAR_TOUCHPAD_VISIBLE ? 0 : 1;
}

void setButtonHoverForActiveTouchPad() {
    char button[3] = {};
    if (!buttonIsInTouchPad(!REAR_TOUCHPAD_VISIBLE, CURRENT_TOUCHPAD_BUTTON_HOVERING)) {
        getFirstButtonForTouchPad(!REAR_TOUCHPAD_VISIBLE, &button);
        strcpy(CURRENT_TOUCHPAD_BUTTON_HOVERING, button);
    }
}

void enforceTouchPadBorders(int *x1, int *x2, int *y1, int *y2, int usingRearTouchPad) {
    struct SceTouchPanelInfo info = usingRearTouchPad ? FULL_REAR_PANEL_INFO : FULL_FRONT_PANEL_INFO;
    if (*x2 > info.maxAaX) {
        int diff = *x2 - info.maxAaX;
        *x1 = *x1 - diff;
        *x2 = *x2 - diff;
    }
    if (*y2 > info.maxAaY) {
        int diff = *y2 - info.maxAaY;
        *y1 = *y1 - diff;
        *y2 = *y2 - diff;
    }
}

void checkTouchPadBorders() {
    int index = getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING);
    switch(index) {
        case 0:
            enforceTouchPadBorders(&TOUCHPAD_CONTROL_SETTINGS.L2_X1, &TOUCHPAD_CONTROL_SETTINGS.L2_X2, &TOUCHPAD_CONTROL_SETTINGS.L2_Y1, &TOUCHPAD_CONTROL_SETTINGS.L2_Y2,TOUCHPAD_CONTROL_SETTINGS.L2_REAR_TOUCHPAD);
            break;
        case 1:
            enforceTouchPadBorders(&TOUCHPAD_CONTROL_SETTINGS.L3_X1, &TOUCHPAD_CONTROL_SETTINGS.L3_X2, &TOUCHPAD_CONTROL_SETTINGS.L3_Y1, &TOUCHPAD_CONTROL_SETTINGS.L3_Y2,TOUCHPAD_CONTROL_SETTINGS.L3_REAR_TOUCHPAD);
            break;
        case 2:
            enforceTouchPadBorders(&TOUCHPAD_CONTROL_SETTINGS.R2_X1, &TOUCHPAD_CONTROL_SETTINGS.R2_X2, &TOUCHPAD_CONTROL_SETTINGS.R2_Y1, &TOUCHPAD_CONTROL_SETTINGS.R2_Y2,TOUCHPAD_CONTROL_SETTINGS.R2_REAR_TOUCHPAD);
            break;
        case 3:
            enforceTouchPadBorders(&TOUCHPAD_CONTROL_SETTINGS.R3_X1, &TOUCHPAD_CONTROL_SETTINGS.R3_X2, &TOUCHPAD_CONTROL_SETTINGS.R3_Y1, &TOUCHPAD_CONTROL_SETTINGS.R3_Y2,TOUCHPAD_CONTROL_SETTINGS.R3_REAR_TOUCHPAD);
            break;
    }
}

void toggleButtonPlacement() {
    int index = getIndexOfTouchPadButton(CURRENT_TOUCHPAD_BUTTON_HOVERING);
    switch(index) {
        case 0:
           TOUCHPAD_CONTROL_SETTINGS.L2_REAR_TOUCHPAD = TOUCHPAD_CONTROL_SETTINGS.L2_REAR_TOUCHPAD ? 0 : 1;
            break;
        case 1:
           TOUCHPAD_CONTROL_SETTINGS.L3_REAR_TOUCHPAD = TOUCHPAD_CONTROL_SETTINGS.L3_REAR_TOUCHPAD ? 0 : 1;
            break;
        case 2:
           TOUCHPAD_CONTROL_SETTINGS.R2_REAR_TOUCHPAD = TOUCHPAD_CONTROL_SETTINGS.R2_REAR_TOUCHPAD ? 0 : 1;
            break;
        case 3:
           TOUCHPAD_CONTROL_SETTINGS.R3_REAR_TOUCHPAD = TOUCHPAD_CONTROL_SETTINGS.R3_REAR_TOUCHPAD ? 0 : 1;
            break;
    }

    checkTouchPadBorders();
    setButtonHoverForActiveTouchPad();
}

void setRearTouchPadVisibility(int visible) {
    REAR_TOUCHPAD_VISIBLE = visible;
}

void updateCleanSettings() {
    CLEAN_TOUCHPAD_CONTROL_SETTINGS = TOUCHPAD_CONTROL_SETTINGS;
    setCleanSettingsUpdated(1);
}

void loadCleanSettings() {
    TOUCHPAD_CONTROL_SETTINGS = CLEAN_TOUCHPAD_CONTROL_SETTINGS;
    setCleanSettingsUpdated(0);
}

void resetSelectedTouchPadButton() {
    strcpy(CURRENT_TOUCHPAD_BUTTON_SELECTED, "");
}

struct SceTouchPanelInfo getScaledTouchPanelInfo(SceTouchPortType port, int scale) {
    SceTouchPanelInfo info;
    sceTouchGetPanelInfo(port, &info);

    info.minAaX = scaleDownInt(info.minAaX, scale);
    info.minAaY = scaleDownInt(info.minAaY, scale);
    info.maxAaX = scaleDownInt(info.maxAaX, scale);
    info.maxAaY = scaleDownInt(info.maxAaY, scale);
    info.minDispX = scaleDownInt(info.minDispX, scale);
    info.minDispY = scaleDownInt(info.minDispY, scale);
    info.maxDispX = scaleDownInt(info.maxDispX, scale);
    info.maxDispY = scaleDownInt(info.maxDispY, scale);

    info.minAaY = info.minAaY - info.minAaY;
    info.maxAaY = info.maxAaY - info.minAaY;

    return info;
}

struct SceTouchPanelInfo getCenteredRearTouchPadInfo() {
    struct SceTouchPanelInfo scaledInfo = getScaledTouchPanelInfo(SCE_TOUCH_PORT_BACK, 40);
    struct SceTouchPanelInfo frontPanelInfo = getScaledTouchPanelInfo(SCE_TOUCH_PORT_FRONT, 50);

    int xOffset = (frontPanelInfo.maxDispX / 2) - (scaledInfo.maxAaX / 2) + 1;
    int yOffset = (frontPanelInfo.maxDispY / 2) - (scaledInfo.maxAaY / 2);

    scaledInfo.minAaX = scaledInfo.minAaX + xOffset;
    scaledInfo.maxAaX = scaledInfo.maxAaX + xOffset;
    scaledInfo.minAaY = scaledInfo.minAaY + yOffset;
    scaledInfo.maxAaY = scaledInfo.maxAaY + yOffset;

    return scaledInfo;
}

struct SceTouchPanelInfo getCenteredFrontTouchPadInfo() {
    struct SceTouchPanelInfo scaledInfo = getScaledTouchPanelInfo(SCE_TOUCH_PORT_FRONT, 35);
    struct SceTouchPanelInfo frontPanelInfo = getScaledTouchPanelInfo(SCE_TOUCH_PORT_FRONT, 50);

    int xOffset = (frontPanelInfo.maxDispX / 2) - (scaledInfo.maxAaX / 2) + 1;
    int yOffset = (frontPanelInfo.maxDispY / 2) - (scaledInfo.maxAaY / 2);

    scaledInfo.minAaX = scaledInfo.minAaX + xOffset;
    scaledInfo.maxAaX = scaledInfo.maxAaX + xOffset;
    scaledInfo.minAaY = scaledInfo.minAaY + yOffset;
    scaledInfo.maxAaY = scaledInfo.maxAaY + yOffset;

    return scaledInfo;
}

struct TouchPadButtonData getTouchPadButtonConfig(int index) {
    struct TouchPadButtonData data;
    switch(index) {
        case 0:
            data.id = "L2";
            data.rearTouchPad = TOUCHPAD_CONTROL_SETTINGS.L2_REAR_TOUCHPAD;
            data.x1 = TOUCHPAD_CONTROL_SETTINGS.L2_X1;
            data.x2 = TOUCHPAD_CONTROL_SETTINGS.L2_X2;
            data.y1 = TOUCHPAD_CONTROL_SETTINGS.L2_Y1;
            data.y2 = TOUCHPAD_CONTROL_SETTINGS.L2_Y2;
            break;
        case 1:
            data.id = "L3";
            data.rearTouchPad = TOUCHPAD_CONTROL_SETTINGS.L3_REAR_TOUCHPAD;
            data.x1 = TOUCHPAD_CONTROL_SETTINGS.L3_X1;
            data.x2 = TOUCHPAD_CONTROL_SETTINGS.L3_X2;
            data.y1 = TOUCHPAD_CONTROL_SETTINGS.L3_Y1;
            data.y2 = TOUCHPAD_CONTROL_SETTINGS.L3_Y2;
            break;
        case 2:
            data.id = "R2";
            data.rearTouchPad = TOUCHPAD_CONTROL_SETTINGS.R2_REAR_TOUCHPAD;
            data.x1 = TOUCHPAD_CONTROL_SETTINGS.R2_X1;
            data.x2 = TOUCHPAD_CONTROL_SETTINGS.R2_X2;
            data.y1 = TOUCHPAD_CONTROL_SETTINGS.R2_Y1;
            data.y2 = TOUCHPAD_CONTROL_SETTINGS.R2_Y2;
            break;
        case 3:
            data.id = "R3";
            data.rearTouchPad = TOUCHPAD_CONTROL_SETTINGS.R3_REAR_TOUCHPAD;
            data.x1 = TOUCHPAD_CONTROL_SETTINGS.R3_X1;
            data.x2 = TOUCHPAD_CONTROL_SETTINGS.R3_X2;
            data.y1 = TOUCHPAD_CONTROL_SETTINGS.R3_Y1;
            data.y2 = TOUCHPAD_CONTROL_SETTINGS.R3_Y2;
            break;
    }

    if (data.rearTouchPad) {
        struct SceTouchPanelInfo rearTouchPadInfo = getCenteredRearTouchPadInfo();
        data.x1 = scaleDownInt(data.x1, REAR_PANEL_SCALE) + rearTouchPadInfo.minAaX;
        data.x2 = scaleDownInt(data.x2, REAR_PANEL_SCALE) + rearTouchPadInfo.minAaX;
        data.y1 = scaleDownInt(data.y1, REAR_PANEL_SCALE) + rearTouchPadInfo.minAaY;
        data.y2 = scaleDownInt(data.y2, REAR_PANEL_SCALE) + rearTouchPadInfo.minAaY;
    } else {
        struct SceTouchPanelInfo frontTouchPadInfo = getCenteredFrontTouchPadInfo();
        data.x1 = scaleDownInt(data.x1, FRONT_PANEL_SCALE) + frontTouchPadInfo.minAaX;
        data.x2 = scaleDownInt(data.x2, FRONT_PANEL_SCALE) + frontTouchPadInfo.minAaX;
        data.y1 = scaleDownInt(data.y1, FRONT_PANEL_SCALE) + frontTouchPadInfo.minAaY;
        data.y2 = scaleDownInt(data.y2, FRONT_PANEL_SCALE) + frontTouchPadInfo.minAaY;
    }

    return data;
}

void getButtonsForTouchPad(int front, char *buttons[]) {
    for (int i = 0; i < getCharArraySize(TouchPadButtons); ++i) {
        struct TouchPadButtonData info = getTouchPadButtonConfig(i);
        if ((front && !info.rearTouchPad) || (!front && info.rearTouchPad)) {
            buttons[i] = TouchPadButtons[i];
        }
    }
}

char getFirstButtonForTouchPad(int front, char *button){
    char *buttons[4] = {};
    getButtonsForTouchPad(front, &buttons);
    for (int i = 0; i < getCharArraySize(buttons); ++i) {
        if (buttons[i]) {
            strcpy(button, buttons[i]);
            return;
        }
    }
}

int getIndexOfTouchPadButton(char *id) {
    for (int i = 0; i < getCharArraySize(TouchPadButtons); ++i) {
        if (strcmp(id, TouchPadButtons[i]) == 0) return i;
    }

    return 0;
}

int buttonIsInTouchPad(int front, char *id) {
    char *buttons[4] = {};
    getButtonsForTouchPad(front, &buttons);
    for (int i = 0; i < getCharArraySize(buttons); ++i) {
        if (buttons[i]) {
            if (strcmp(id, buttons[i]) == 0) return 1;
        }
    }
    return 0;
}

int checkTouchPadControlsChanged(struct VitaPadTouchPadControlSettings source, struct VitaPadTouchPadControlSettings target) {
    int L2_SAME = source.L2_X1 == target.L2_X1 && source.L2_X2 == target.L2_X2 && source.L2_Y1 == target.L2_Y1 && source.L2_Y2 == target.L2_Y2 && source.L2_REAR_TOUCHPAD == target.L2_REAR_TOUCHPAD;
    int L3_SAME = source.L3_X1 == target.L3_X1 && source.L3_X2 == target.L3_X2 && source.L3_Y1 == target.L3_Y1 && source.L3_Y2 == target.L3_Y2 && source.L3_REAR_TOUCHPAD == target.L3_REAR_TOUCHPAD;
    int R2_SAME = source.R2_X1 == target.R2_X1 && source.R2_X2 == target.R2_X2 && source.R2_Y1 == target.R2_Y1 && source.R2_Y2 == target.R2_Y2 && source.R2_REAR_TOUCHPAD == target.R2_REAR_TOUCHPAD;
    int R3_SAME = source.R3_X1 == target.R3_X1 && source.R3_X2 == target.R3_X2 && source.R3_Y1 == target.R3_Y1 && source.R3_Y2 == target.R3_Y2 && source.R3_REAR_TOUCHPAD == target.R3_REAR_TOUCHPAD;

    if (L2_SAME && L3_SAME && R2_SAME && R3_SAME) {
        return 0;
    }

    return 1;
}

void updateTouchPadButtonsPressed() {
    SceTouchData frontTouchPadData;
    SceTouchData rearTouchPadData;

    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &frontTouchPadData, 1);
    sceTouchPeek(SCE_TOUCH_PORT_BACK, &rearTouchPadData, 1);

    L2_PRESSED = findTouchReport(TOUCHPAD_CONTROL_SETTINGS.L2_REAR_TOUCHPAD ? rearTouchPadData : frontTouchPadData, TOUCHPAD_CONTROL_SETTINGS.L2_X1, TOUCHPAD_CONTROL_SETTINGS.L2_X2, TOUCHPAD_CONTROL_SETTINGS.L2_Y1, TOUCHPAD_CONTROL_SETTINGS.L2_Y2);
    L3_PRESSED = findTouchReport(TOUCHPAD_CONTROL_SETTINGS.L3_REAR_TOUCHPAD ? rearTouchPadData : frontTouchPadData, TOUCHPAD_CONTROL_SETTINGS.L3_X1, TOUCHPAD_CONTROL_SETTINGS.L3_X2, TOUCHPAD_CONTROL_SETTINGS.L3_Y1, TOUCHPAD_CONTROL_SETTINGS.L3_Y2);
    R2_PRESSED = findTouchReport(TOUCHPAD_CONTROL_SETTINGS.R2_REAR_TOUCHPAD ? rearTouchPadData : frontTouchPadData, TOUCHPAD_CONTROL_SETTINGS.R2_X1, TOUCHPAD_CONTROL_SETTINGS.R2_X2, TOUCHPAD_CONTROL_SETTINGS.R2_Y1, TOUCHPAD_CONTROL_SETTINGS.R2_Y2);
    R3_PRESSED = findTouchReport(TOUCHPAD_CONTROL_SETTINGS.R3_REAR_TOUCHPAD ? rearTouchPadData : frontTouchPadData, TOUCHPAD_CONTROL_SETTINGS.R3_X1, TOUCHPAD_CONTROL_SETTINGS.R3_X2, TOUCHPAD_CONTROL_SETTINGS.R3_Y1, TOUCHPAD_CONTROL_SETTINGS.R3_Y2);
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

void drawRearTouchPanelBorder() {
    struct SceTouchPanelInfo info = getCenteredRearTouchPadInfo();

    vita2d_draw_line(info.minAaX, info.minAaY, info.maxAaX + 1, info.minAaY, TOUCHPAD_CONTROLS_PANEL_BORDER_BG);
    vita2d_draw_line(info.minAaX, info.minAaY, info.minAaX, info.maxAaY + 1, TOUCHPAD_CONTROLS_PANEL_BORDER_BG);
    vita2d_draw_line(info.minAaX, info.maxAaY + 1, info.maxAaX + 1, info.maxAaY + 1, TOUCHPAD_CONTROLS_PANEL_BORDER_BG);
    vita2d_draw_line(info.maxAaX + 1, info.minAaY, info.maxAaX + 1, info.maxAaY + 1, TOUCHPAD_CONTROLS_PANEL_BORDER_BG);
}

void drawFrontTouchPanelBorder() {
    struct SceTouchPanelInfo info = getCenteredFrontTouchPadInfo();

    vita2d_draw_line(info.minAaX, info.minAaY, info.maxAaX + 1, info.minAaY, TOUCHPAD_CONTROLS_PANEL_BORDER_BG);
    vita2d_draw_line(info.minAaX, info.minAaY, info.minAaX, info.maxAaY + 1, TOUCHPAD_CONTROLS_PANEL_BORDER_BG);
    vita2d_draw_line(info.minAaX, info.maxAaY + 1, info.maxAaX + 1, info.maxAaY + 1, TOUCHPAD_CONTROLS_PANEL_BORDER_BG);
    vita2d_draw_line(info.maxAaX + 1, info.minAaY, info.maxAaX + 1, info.maxAaY + 1, TOUCHPAD_CONTROLS_PANEL_BORDER_BG);
}


void displayStatusBar() {
    int statusBarTextY = SCREEN_HEIGHT - (STATUS_BAR_HEIGHT / 2) - (FONT_Y_SPACE / 2);
    vita2d_draw_rectangle(0, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, SCREEN_WIDTH, STATUS_BAR_HEIGHT, TOUCHPAD_CONTROLS_STATUS_BAR_BG);
    pgf_draw_text(SCREEN_MARGIN, statusBarTextY, WHITE, REAR_TOUCHPAD_VISIBLE ? "REAR TOUCHPAD" : "FRONT TOUCHPAD");

    if (strcmp(CURRENT_TOUCHPAD_BUTTON_SELECTED, "") != 0) {
        char selectedText[12];
        sprintf(selectedText, "%s SELECTED", CURRENT_TOUCHPAD_BUTTON_SELECTED);
        pgf_draw_text((SCREEN_WIDTH / 2) - (pgf_text_width(selectedText) / 2), statusBarTextY, WHITE, selectedText);
    }

    int length = getCharArraySize(TouchPadButtons);
    for (int i = 0; i < length; ++i) {
        int margin = FONT_X_SPACE * 2;
        int x = SCREEN_WIDTH - ((length - i) * pgf_text_width(TouchPadButtons[i])) - ((length - i) * margin);
        int active = buttonIsInTouchPad(!REAR_TOUCHPAD_VISIBLE, TouchPadButtons[i]);
        pgf_draw_text(x, statusBarTextY, active ? WHITE : WHITE_OPACITY_30, TouchPadButtons[i]);
    }
}

int calculateHelpBarWidth(int itemPadding, int textPadding) {
    int width = 0;
    int startingAtZero = 1;

    for (int i = 0; i < (sizeof HELP_BAR_ITEMS / sizeof HELP_BAR_ITEMS[0]); ++i) {
        struct HelpBarItem item = HELP_BAR_ITEMS[i];

        if (strcmp(item.title, "RESET") == 0 || strcmp(item.title, "SAVE") == 0) {
            if (!checkTouchPadControlsChanged(CLEAN_TOUCHPAD_CONTROL_SETTINGS, TOUCHPAD_CONTROL_SETTINGS)) {
                continue;
            }
        }

        int imageX = startingAtZero ? 0 : itemPadding + width;
        int titleX = imageX + item.textureWidth + textPadding;
        startingAtZero = 0;
        width = titleX + pgf_text_width(item.title);
    }
    return width;
}

void handleTouchPadControlHelpBarActions() {
    if (!CLEAN_TOUCHPAD_CONTROL_SETTINGS_UPDATED) {
        updateCleanSettings();
    }

    if (pressed_pad[PAD_TRIANGLE]) {
        if (modalActive()) return;
        if (checkTouchPadControlsChanged(CLEAN_TOUCHPAD_CONTROL_SETTINGS, TOUCHPAD_CONTROL_SETTINGS)) {
            openTouchPadControlExitConfirmationModal();
        } else {
            exitTouchPadControlSettings();
        }
    }
}

void displayHelpBar() {
    int textPadding = 10;
    int itemPadding = 30;
    int previousX = 0;
    int useStartPosition = 1;
    int startPosition = SCREEN_HALF_WIDTH - (calculateHelpBarWidth(itemPadding, textPadding) / 2);
    int top = SCREEN_MARGIN + 10;

    for (int i = 0; i < (sizeof HELP_BAR_ITEMS / sizeof HELP_BAR_ITEMS[0]); ++i) {
        struct HelpBarItem item = HELP_BAR_ITEMS[i];

        if (strcmp(item.title, "RESET") == 0 || strcmp(item.title, "SAVE") == 0) {
            if (!checkTouchPadControlsChanged(CLEAN_TOUCHPAD_CONTROL_SETTINGS, TOUCHPAD_CONTROL_SETTINGS)) {
                continue;
            }
        }

        int imageX = useStartPosition ? startPosition : itemPadding + previousX;
        int titleX = imageX + item.textureWidth + textPadding;
        useStartPosition = 0;
        vita2d_draw_texture(item.texture, imageX, top + 1);
        pgf_draw_text(titleX, top, WHITE_OPACITY_80, item.title);
        previousX = titleX + pgf_text_width(item.title);
    }
}

void displayTouchPadControls() {
    char *buttons[4] = {};
    getButtonsForTouchPad(!REAR_TOUCHPAD_VISIBLE, &buttons);
    int length = getCharArraySize(buttons);
    for (int i = 0; i < length; i++) {
        if (buttons[i]) {
            int index = getIndexOfTouchPadButton(buttons[i]);
            struct TouchPadButtonData data = getTouchPadButtonConfig(index);
            int width = data.x2 - data.x1;
            int height = data.y2 - data.y1;
            int pressed = checkButtonPressed(index);
            int selected = strcmp(CURRENT_TOUCHPAD_BUTTON_SELECTED, buttons[i]) == 0;
            int hovering = strcmp(CURRENT_TOUCHPAD_BUTTON_HOVERING, buttons[i]) == 0;
            int textY = (data.y1 + height / 2) - (FONT_Y_SPACE / 2);
            vita2d_draw_rectangle(data.x1, data.y1, width, height, selected ? TOUCHPAD_CONTROLS_BUTTON_SELECTED_BG : hovering ? TOUCHPAD_CONTROLS_BUTTON_HOVER_BG : TOUCHPAD_CONTROLS_BUTTON_BG);
            pgf_draw_text((data.x1 + width / 2) - (pgf_text_width(data.id) / 2),  pressed ? textY - (FONT_Y_SPACE / 2) : textY, BLACK, data.id);
            if (pressed) pgf_draw_text((data.x1 + width / 2) - (pgf_text_width("ACTIVE") / 2), (data.y1 + height / 2) - (FONT_Y_SPACE / 2) - (FONT_Y_SPACE / 2) + FONT_Y_SPACE, BLACK_OPACITY_50, "ACTIVE");
        }
    }
}

void displayTouchPadBorders() {
    if (REAR_TOUCHPAD_VISIBLE) {
        drawRearTouchPanelBorder();
    } else {
        drawFrontTouchPanelBorder();
    }
}

void displayTouchPadControlSettings() {
    displayStatusBar();
    displayHelpBar();
    displayTouchPadBorders();
    displayTouchPadControls();
}

void exitTouchPadControlSettings() {
    setCurrentScreen(0);
    setCleanSettingsUpdated(0);
    resetSelectedTouchPadButton();
}