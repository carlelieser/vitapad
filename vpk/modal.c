#include <string.h>
#include "font.h"
#include "settings.h"
#include "color.h"
#include "utils.h"
#include "menu.h"
#include "modal.h"

struct DialogAction {
    char *button;
    char *title;
};

struct DialogData {
    char *id;
    char *message;
    char *confirmLabel;
    char *cancelLabel;
};

struct DialogData CURRENT_MODAL = {
        .id = "",
        .message = "",
        .confirmLabel = "",
        .cancelLabel = ""
};

char TOUCHPAD_CONTROL_SAVE_MODAL_ID[] = "touch-pad-control-save-confirmation";
char TOUCHPAD_CONTROL_RESET_MODAL_ID[] = "touch-pad-control-clean-settings-reset";
char TOUCHPAD_CONTROL_RESET_DEFAULTS_MODAL_ID[] = "touch-pad-control-default-settings-reset";
char TOUCHPAD_CONTROL_EXIT_MODAL_ID[] = "touch-pad-control-exit";

int modalActive() {
    return strcmp(CURRENT_MODAL.id, "") != 0;
}

void openModal(char *id, char *message, char *confirmLabel, char *cancelLabel) {
    readPad();
    strcpy(CURRENT_TOUCHPAD_BUTTON_SELECTED, "");
    CURRENT_MODAL.id = id;
    CURRENT_MODAL.message = message;
    CURRENT_MODAL.confirmLabel = confirmLabel;
    CURRENT_MODAL.cancelLabel = cancelLabel;
}

void openTouchPadControlExitConfirmationModal() {
    openModal(TOUCHPAD_CONTROL_EXIT_MODAL_ID, "Are you sure you want to exit?\n\nYou have unsaved changes.", "EXIT", "CANCEL");
}

void openTouchPadControlSaveConfirmationModal() {
    openModal(TOUCHPAD_CONTROL_SAVE_MODAL_ID, "Touchpad control settings saved.", "SOUNDS GOOD", "");
}

void openTouchPadControlResetToCleanSettingsModal() {
    openModal(TOUCHPAD_CONTROL_RESET_MODAL_ID, "Are you sure you want to reset\n\ntouchpad controls?", "RESET", "CANCEL");
}

void openTouchPadControlResetToDefaultSettingsModal() {
    openModal(TOUCHPAD_CONTROL_RESET_DEFAULTS_MODAL_ID, "Are you sure you want to set touchpad\n\ncontrols to default settings?", "RESET", "CANCEL");
}

void hideModal() {
    CURRENT_MODAL.id = "";
    CURRENT_MODAL.message = "";
    CURRENT_MODAL.confirmLabel = "";
    CURRENT_MODAL.cancelLabel = "";
}

int handleModalDialogAction() {
    if (strcmp(CURRENT_MODAL.confirmLabel, "") != 0) {
        if (pressed_pad[PAD_CROSS]) {
            return 1;
        }
    }
    if (strcmp(CURRENT_MODAL.cancelLabel, "") != 0) {
        if (pressed_pad[PAD_CIRCLE]) {
            return 2;
        }
    }
    return 0;
}

void handleModalDialogs() {
    if (strcmp(CURRENT_MODAL.id, TOUCHPAD_CONTROL_SAVE_MODAL_ID) == 0) {
        int result = handleModalDialogAction();
        if (result == 1) {
            hideModal();
        }
    }
    if (strcmp(CURRENT_MODAL.id, TOUCHPAD_CONTROL_RESET_MODAL_ID) == 0) {
        int result = handleModalDialogAction();
        if (result == 1) {
            resetSelectedTouchPadButton();
            loadCleanSettings();
            hideModal();
        }
        if (result == 2) {
            hideModal();
        }
    }

    if (strcmp(CURRENT_MODAL.id, TOUCHPAD_CONTROL_RESET_DEFAULTS_MODAL_ID) == 0) {
        int result = handleModalDialogAction();
        if (result == 1) {
            loadDefaultTouchPadControlSettings();
            setCurrentMenuItem(getMenuItemIndex("RESET TOUCHPAD CONTROLS") - 1);
            hideModal();
        }
        if (result == 2) {
            hideModal();
        }
    }

    if (strcmp(CURRENT_MODAL.id, TOUCHPAD_CONTROL_EXIT_MODAL_ID) == 0) {
        int result = handleModalDialogAction();
        if (result == 1) {
            loadTouchPadControlSettings();
            exitTouchPadControlSettings();
            hideModal();
        }
        if (result == 2) {
            hideModal();
        }
    }
}

void drawModalDialogActions(int modalX, int modalY, int modalWidth, int modalHeight, int padding) {
    int startX = modalX + modalWidth - padding;
    int iconSize = 24;
    int iconLabelMargin = 6;
    int iconYOffset = 1;
    int buttonMargin = 20;
    for (int i = 0; i < 2 ; ++i) {
        char *label = i == 0 ? CURRENT_MODAL.confirmLabel : CURRENT_MODAL.cancelLabel;

        if (strcmp(label, "") == 0) break;

        int labelX = startX - pgf_text_width(label);
        int labelY = modalY + modalHeight - FONT_Y_SPACE - padding;
        int iconX = labelX - iconSize - iconLabelMargin;
        int iconY = labelY + iconYOffset;

        vita2d_draw_texture(i == 0 ? CROSS_BUTTON_TEXTURE : CIRCLE_BUTTON_TEXTURE, iconX, iconY);
        pgf_draw_text(labelX, labelY, MODAL_CONTENT_ACTION_COLOR, label);

        startX = iconX - buttonMargin;
    }
}

void displayModalDialogs() {
    if (strcmp(CURRENT_MODAL.id, "") != 0) {
        int padding = 20;
        int textWidth = pgf_text_width(CURRENT_MODAL.message);
        int textHeight = pgf_text_height(CURRENT_MODAL.message);
        int modalWidth = textWidth + (padding * 2) + 20;
        int modalHeight = 120 + textHeight;
        int modalX = SCREEN_HALF_WIDTH - (modalWidth / 2);
        int modalY = SCREEN_HALF_HEIGHT - (modalHeight / 2);

        vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MODAL_BG);
        drawRectangle(modalX, modalY, modalWidth, modalHeight, MODAL_BORDER_RADIUS, MODAL_CONTENT_BG);
        pgf_draw_textf(modalX + padding, modalY + padding, MODAL_CONTENT_TEXT_COLOR, CURRENT_MODAL.message);

        drawModalDialogActions(modalX, modalY, modalWidth, modalHeight, padding);
    }
}