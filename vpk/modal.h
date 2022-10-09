#ifndef __MODAL_H__
#define __MODAL_H__

#include <vita2d.h>
#include "touchpad.h"

#define MODAL_BORDER_RADIUS 16

int handleModalDialogAction();
void displayModalDialogs();
void drawModalDialogActions();
void handleModalDialogs();
void hideModal();
void openModal();

extern int modalActive();
extern void openTouchPadControlExitConfirmationModal();
extern void openTouchPadControlSaveConfirmationModal();
extern void openTouchPadControlResetToCleanSettingsModal();
extern void openTouchPadControlResetToDefaultSettingsModal();

#endif