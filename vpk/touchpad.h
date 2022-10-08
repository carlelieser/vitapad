#ifndef __TOUCHPAD_H__
#define __TOUCHPAD_H__

#include <vita2d.h>
#include <psp2/touch.h>
#include "settings.h"

#define INCLUDE_EXTERN_RESOURCE(name) extern unsigned char _binary_resources_##name##_start; extern unsigned char _binary_resources_##name##_size; \

extern void handleTouchPadControlHelpBarActions();
extern void openTouchPadControlSaveConfirmationModal();
extern void openTouchPadControlResetToCleanSettingsModal();

extern vita2d_texture *SELECT_BUTTON_TEXTURE;
extern vita2d_texture *START_BUTTON_TEXTURE;
extern vita2d_texture *TRIANGLE_BUTTON_TEXTURE;
extern vita2d_texture *SQUARE_BUTTON_TEXTURE;
extern vita2d_texture *CROSS_BUTTON_TEXTURE;
extern vita2d_texture *CIRCLE_BUTTON_TEXTURE;

extern int L2_PRESSED;
extern int L3_PRESSED;
extern int R2_PRESSED;
extern int R3_PRESSED;
extern int REAR_TOUCHPAD_VISIBLE;

extern char CURRENT_TOUCHPAD_BUTTON_HOVERING[3];
extern char CURRENT_TOUCHPAD_BUTTON_SELECTED[3];

struct TouchPadButtonData {
    char *id;
    int rearTouchPad;
    int x1;
    int x2;
    int y1;
    int y2;
};

struct HelpBarItem {
    struct vita2d_texture *texture;
    int textureWidth;
    char *title;
    int *titleWidth;
};

struct TouchPadButtonData getTouchPadButtonConfig();
struct TouchPadButtonData findButton();

struct SceTouchPanelInfo getScaledTouchPanelInfo(SceTouchPortType port, int scale);
struct SceTouchPanelInfo getCenteredRearTouchPadInfo();
struct SceTouchPanelInfo getCenteredFrontTouchPadInfo();

char getFirstButtonForTouchPad();

void enforceTouchPadBorders();
void setButtonHoverForActiveTouchPad();
void toggleRearTouchPadVisible();
void updateCleanSettings();
void loadCleanSettings();

void getButtonDistances();
void getButtonsForTouchPad();
void filterButtons();
void selectButtonFromCoordinates();

void hoverLeft();
void hoverRight();
void hoverUp();
void hoverDown();

void updateButtonPlacement();
void toggleButtonPlacement();
void setRearTouchPadVisibility();

void moveCurrentButtonLeft();
void moveCurrentButtonRight();
void moveCurrentButtonUp();
void moveCurrentButtonDown();

int getIndexOfTouchPadButton();
int buttonIsInTouchPad();
int checkTouchPadControlsChanged();

int findTouchReport();

void handleTouchPadControlSelectionAndPlacement();
void resetSelectedTouchPadButton();

void updateHelpBarItems();
void initHelpBarItems();

void initTouchPadTextures();
void freeTouchPadTextures();

void updateTouchPanelInfo();
void displayTouchPadControls();
void displayTouchPadBorders();
void displayHelpBar();
void displayStatusBar();
void displayTouchPadControlSettings();
void exitTouchPadControlSettings();

#endif