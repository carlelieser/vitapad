#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define INCLUDE_EXTERN_RESOURCE(name) extern unsigned char _binary_resources_##name##_start; extern unsigned char _binary_resources_##name##_size; \

#define MAX_CONFIG_NAME_LENGTH 128
#define MAX_CONFIG_LINE_LENGTH 1024

#define DEFAULT_ACTIVE_DISPLAY 1
#define DEFAULT_XINPUT_MAPPING 1
#define DEFAULT_L2_REAR_TOUCHPAD 1
#define DEFAULT_L3_REAR_TOUCHPAD 1
#define DEFAULT_R2_REAR_TOUCHPAD 1
#define DEFAULT_R3_REAR_TOUCHPAD 1
#define DEFAULT_L2_X1 0
#define DEFAULT_L2_X2 600
#define DEFAULT_L2_Y1 0
#define DEFAULT_L2_Y2 300
#define DEFAULT_R2_X1 1319
#define DEFAULT_R2_X2 1919
#define DEFAULT_R2_Y1 0
#define DEFAULT_R2_Y2 300
#define DEFAULT_L3_X1 240
#define DEFAULT_L3_X2 840
#define DEFAULT_L3_Y1 589
#define DEFAULT_L3_Y2 889
#define DEFAULT_R3_X1 1079
#define DEFAULT_R3_X2 1679
#define DEFAULT_R3_Y1 589
#define DEFAULT_R3_Y2 889

typedef struct {
    const char *path;
    void *buffer;
    int size;
    int replace;
} DefaultFile;

typedef struct {
    const char *name;
    int value;
} SettingEntry;

struct VitaPadSettings {
    int ACTIVE_DISPLAY;
    int XINPUT_MAPPING;
};

struct VitaPadTouchPadControlSettings {
    int L2_REAR_TOUCHPAD;
    int L2_X1;
    int L2_X2;
    int L2_Y1;
    int L2_Y2;
    int L3_REAR_TOUCHPAD;
    int L3_X1;
    int L3_X2;
    int L3_Y1;
    int L3_Y2;
    int R2_REAR_TOUCHPAD;
    int R2_X1;
    int R2_X2;
    int R2_Y1;
    int R2_Y2;
    int R3_REAR_TOUCHPAD;
    int R3_X1;
    int R3_X2;
    int R3_Y1;
    int R3_Y2;
};

extern struct VitaPadSettings SETTINGS;
extern struct VitaPadSettings DEFAULT_SETTINGS;
extern struct VitaPadTouchPadControlSettings TOUCHPAD_CONTROL_SETTINGS;
extern struct VitaPadTouchPadControlSettings DEFAULT_TOUCHPAD_CONTROL_SETTINGS;

extern struct VitaPadTouchPadControlSettings CLEAN_TOUCHPAD_CONTROL_SETTINGS;
extern int CLEAN_TOUCHPAD_CONTROL_SETTINGS_UPDATED;

extern void toggleActiveDisplay();
extern void toggleAltLayout();

static SettingEntry SettingEntries[2];
static SettingEntry TouchPadControlSettingEntries[20];

int readSettingsBuffer(void *buffer, int size, SettingEntry *entries, int n_entries);
int readSettings(const char *path, SettingEntry *entries, int n_entries);
int writeSettings(const char *path, SettingEntry *entries, int n_entries);

void initSettings();

void loadSettings();
void saveSettings();

void loadTouchPadControlSettings();
void saveTouchPadControlSettings();

void loadDefaultSettings();
void loadDefaultTouchPadControlSettings();

void setCleanSettingsUpdated();

#endif