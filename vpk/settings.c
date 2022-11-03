#include <psp2/io/stat.h>
#include <string.h>
#include <stdlib.h>
#include "settings.h"

INCLUDE_EXTERN_RESOURCE(settings_txt);
INCLUDE_EXTERN_RESOURCE(default_settings_txt);
INCLUDE_EXTERN_RESOURCE(touchpad_controls_txt);
INCLUDE_EXTERN_RESOURCE(default_touchpad_controls_txt);

#define DEFAULT_FILE(path, name, replace) { path, (void *)&_binary_resources_##name##_start, (int)&_binary_resources_##name##_size, replace }

static DefaultFile default_files[] = {
        DEFAULT_FILE("ux0:VitaPad/settings.txt", settings_txt, 0),
        DEFAULT_FILE("ux0:VitaPad/default-settings.txt", default_settings_txt, 0),
        DEFAULT_FILE("ux0:VitaPad/touchpad-controls.txt", touchpad_controls_txt, 0),
        DEFAULT_FILE("ux0:VitaPad/default-touchpad-controls.txt", default_touchpad_controls_txt, 0)
};

DefaultFile SETTINGS_FILE = DEFAULT_FILE("ux0:VitaPad/settings.txt", settings_txt, 0);
DefaultFile DEFAULT_SETTINGS_FILE = DEFAULT_FILE("ux0:VitaPad/default-settings.txt", default_settings_txt, 0);
DefaultFile TOUCHPAD_CONTROL_SETTINGS_FILE = DEFAULT_FILE("ux0:VitaPad/touchpad-controls.txt", touchpad_controls_txt, 0);
DefaultFile DEFAULT_TOUCHPAD_CONTROL_SETTINGS_FILE = DEFAULT_FILE("ux0:VitaPad/default-touchpad-controls.txt", default_touchpad_controls_txt, 0);

struct VitaPadSettings SETTINGS = {DEFAULT_ACTIVE_DISPLAY, DEFAULT_XINPUT_MAPPING};
struct VitaPadSettings DEFAULT_SETTINGS = {DEFAULT_ACTIVE_DISPLAY, DEFAULT_XINPUT_MAPPING};
struct VitaPadTouchPadControlSettings TOUCHPAD_CONTROL_SETTINGS = {DEFAULT_L2_REAR_TOUCHPAD, DEFAULT_L2_X1, DEFAULT_L2_X2, DEFAULT_L2_Y1, DEFAULT_L2_Y2, DEFAULT_L3_REAR_TOUCHPAD, DEFAULT_L3_X1, DEFAULT_L3_X2, DEFAULT_L3_Y1, DEFAULT_L3_Y2, DEFAULT_R2_REAR_TOUCHPAD, DEFAULT_R2_X1, DEFAULT_R2_X2, DEFAULT_R2_Y1, DEFAULT_R2_Y2, DEFAULT_R3_REAR_TOUCHPAD, DEFAULT_R3_X1, DEFAULT_R3_X2, DEFAULT_R3_Y1, DEFAULT_R3_Y2};
struct VitaPadTouchPadControlSettings DEFAULT_TOUCHPAD_CONTROL_SETTINGS = {DEFAULT_L2_REAR_TOUCHPAD, DEFAULT_L2_X1, DEFAULT_L2_X2, DEFAULT_L2_Y1, DEFAULT_L2_Y2, DEFAULT_L3_REAR_TOUCHPAD, DEFAULT_L3_X1, DEFAULT_L3_X2, DEFAULT_L3_Y1, DEFAULT_L3_Y2, DEFAULT_R2_REAR_TOUCHPAD, DEFAULT_R2_X1, DEFAULT_R2_X2, DEFAULT_R2_Y1, DEFAULT_R2_Y2, DEFAULT_R3_REAR_TOUCHPAD, DEFAULT_R3_X1, DEFAULT_R3_X2, DEFAULT_R3_Y1, DEFAULT_R3_Y2};

struct VitaPadTouchPadControlSettings CLEAN_TOUCHPAD_CONTROL_SETTINGS;
int CLEAN_TOUCHPAD_CONTROL_SETTINGS_UPDATED = 0;

static SettingEntry SettingEntries[] = {
        {"ACTIVE_DISPLAY", (int *)&SETTINGS.ACTIVE_DISPLAY},
        {"XINPUT_MAPPING", (int *)&SETTINGS.XINPUT_MAPPING},
};

static SettingEntry TouchPadControlSettingEntries[] = {
        {"L2_REAR_TOUCHPAD", (int *)&TOUCHPAD_CONTROL_SETTINGS.L2_REAR_TOUCHPAD},
        {"L2_X1", (int *)&TOUCHPAD_CONTROL_SETTINGS.L2_X1},
        {"L2_X2", (int *)&TOUCHPAD_CONTROL_SETTINGS.L2_X2},
        {"L2_Y1", (int *)&TOUCHPAD_CONTROL_SETTINGS.L2_Y1},
        {"L2_Y2", (int *)&TOUCHPAD_CONTROL_SETTINGS.L2_Y2},
        {"L3_REAR_TOUCHPAD", (int *)&TOUCHPAD_CONTROL_SETTINGS.L3_REAR_TOUCHPAD},
        {"L3_X1", (int *)&TOUCHPAD_CONTROL_SETTINGS.L3_X1},
        {"L3_X2", (int *)&TOUCHPAD_CONTROL_SETTINGS.L3_X2},
        {"L3_Y1", (int *)&TOUCHPAD_CONTROL_SETTINGS.L3_Y1},
        {"L3_Y2", (int *)&TOUCHPAD_CONTROL_SETTINGS.L3_Y2},
        {"R2_REAR_TOUCHPAD", (int *)&TOUCHPAD_CONTROL_SETTINGS.R2_REAR_TOUCHPAD},
        {"R2_X1", (int *)&TOUCHPAD_CONTROL_SETTINGS.R2_X1},
        {"R2_X2", (int *)&TOUCHPAD_CONTROL_SETTINGS.R2_X2},
        {"R2_Y1", (int *)&TOUCHPAD_CONTROL_SETTINGS.R2_Y1},
        {"R2_Y2", (int *)&TOUCHPAD_CONTROL_SETTINGS.R2_Y2},
        {"R3_REAR_TOUCHPAD", (int *)&TOUCHPAD_CONTROL_SETTINGS.R3_REAR_TOUCHPAD},
        {"R3_X1", (int *)&TOUCHPAD_CONTROL_SETTINGS.R3_X1},
        {"R3_X2", (int *)&TOUCHPAD_CONTROL_SETTINGS.R3_X2},
        {"R3_Y1", (int *)&TOUCHPAD_CONTROL_SETTINGS.R3_Y1},
        {"R3_Y2", (int *)&TOUCHPAD_CONTROL_SETTINGS.R3_Y2},
};

static void trim(char *str) {
    int len = strlen(str);
    int i;

    for (i = len - 1; i >= 0; i--) {
        if (str[i] == 0x20 || str[i] == '\t') {
            str[i] = 0;
        } else {
            break;
        }
    }
}

static int GetLine(char *buf, int size, char *str) {
    uint8_t ch = 0;
    int n = 0;
    int i = 0;
    uint8_t *s = (uint8_t *)str;

    while (1) {
        if (i >= size)
            break;

        ch = ((uint8_t *)buf)[i];

        if (ch < 0x20 && ch != '\t') {
            if (n != 0) {
                i++;
                break;
            }
        } else {
            *str++ = ch;
            n++;
        }

        i++;
    }

    trim((char *)s);

    return i;
}

static int getDecimal(const char *str) {
    return strtol(str, NULL, 0);
}

static int readEntry(const char *line, SettingEntry *entries, int n_entries) {
    while (*line == ' ' || *line == '\t')
        line++;

    if (line[0] == '#') {
        return 0;
    }

    char *p = strchr(line, '#');
    if (p) {
        *p = '\0';
    }

    // Get token
    p = strchr(line, '=');
    if (!p)
        return -1;

    char name[MAX_CONFIG_NAME_LENGTH];
    int len = p - line;
    if (len > MAX_CONFIG_NAME_LENGTH - 1)
        len = MAX_CONFIG_NAME_LENGTH - 1;
    strncpy(name, line, len);
    name[len] = '\0';

    trim(name);
    char *string = p + 1;

    while (*string == ' ' || *string == '\t')
        string++;

    trim(string);

    int i;
    for (i = 0; i < n_entries; i++) {
        if (strcasecmp(name, entries[i].name) == 0) {
            *(uint32_t *)entries[i].value = getDecimal(string);
            break;
        }
    }

    return 1;
}

int readSettingsBuffer(void *buffer, int size, SettingEntry *entries, int n_entries) {
    int res = 0;
    char line[MAX_CONFIG_LINE_LENGTH];
    char *p = buffer;

    // Skip UTF-8 bom
    uint32_t bom = 0xBFBBEF;
    if (memcmp(p, &bom, 3) == 0) {
        p += 3;
        size -= 3;
    }

    do {
        memset(line, 0, sizeof(line));
        res = GetLine(p, size, line);

        if (res > 0) {
            readEntry(line, entries, n_entries);
            size -= res;
            p += res;
        }
    } while (res > 0);

    return 0;
}

int allocateReadFile(const char *file, void **buffer) {
    SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
    if (fd < 0)
        return fd;

    int size = sceIoLseek32(fd, 0, SCE_SEEK_END);
    sceIoLseek32(fd, 0, SCE_SEEK_SET);

    *buffer = malloc(size);
    if (!*buffer) {
        sceIoClose(fd);
        return 0;
    }

    int read = sceIoRead(fd, *buffer, size);
    sceIoClose(fd);

    return read;
}

int readSettings(const char *path, SettingEntry *entries, int n_entries) {
    void *buffer = NULL;
    int size = allocateReadFile(path, &buffer);
    if (size < 0)
        return size;

    readSettingsBuffer(buffer, size, entries, n_entries);

    free(buffer);

    return 0;
}

static int writeEntry(SceUID fd, SettingEntry *entry) {
    if (!entry->value)
        return -1;

    int result;
    if ((result = sceIoWrite(fd, entry->name, strlen(entry->name))) < 0)
        return result;

    if ((result = sceIoWrite(fd, " = ", 3)) < 0)
        return result;

    char *val;
    char buffer[33];

    itoa(*(int *)entry->value, buffer, 10);
    result = sceIoWrite(fd, buffer, strlen(buffer));

    if (result < 0)
        return result;

    if ((sceIoWrite(fd, "\n", 1)) < 0)
        return result;

    return 0;
}

int writeSettings(const char *path, SettingEntry *entries, int n_entries) {
    SceUID fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
        return fd;

    int i;
    for (i = 0; i < n_entries; i++) {
        int result = writeEntry(fd, entries+i);
        if (result != 0) {
            return result;
        }
    }

    sceIoClose(fd);

    return 0;
}

int writeFile(const char *file, const void *buf, int size) {
    SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
        return fd;

    int written = sceIoWrite(fd, buf, size);

    sceIoClose(fd);
    return written;
}


void initSettings() {
    sceIoMkdir("ux0:VitaPad", 0777);
    SceIoStat stat;
    memset(&stat, 0, sizeof(stat));
    for (int i = 0; i < (sizeof(default_files) / sizeof(DefaultFile)); i++) {
        SceIoStat stat;
        memset(&stat, 0, sizeof(stat));
        if (sceIoGetstat(default_files[i].path, &stat) < 0 || (default_files[i].replace && (int)stat.st_size != default_files[i].size))
            writeFile(default_files[i].path, default_files[i].buffer, default_files[i].size);
    }
}

void loadSettings() {
    memset(&SETTINGS, 0, sizeof(SETTINGS));
    readSettings(SETTINGS_FILE.path, SettingEntries, sizeof(SettingEntries) / sizeof(SettingEntry));
}

void saveSettings() {
    writeSettings(SETTINGS_FILE.path, SettingEntries, sizeof(SettingEntries) / sizeof(SettingEntry));
}

void loadDefaultSettings() {
    memset(&SETTINGS, 0, sizeof(SETTINGS));
    readSettings(DEFAULT_SETTINGS_FILE.path, SettingEntries, sizeof(SettingEntries) / sizeof(SettingEntry));
    writeSettings(SETTINGS_FILE.path, SettingEntries, sizeof(SettingEntries) / sizeof(SettingEntry));
}

void loadTouchPadControlSettings() {
    memset(&TOUCHPAD_CONTROL_SETTINGS, 0, sizeof(TOUCHPAD_CONTROL_SETTINGS));
    readSettings(TOUCHPAD_CONTROL_SETTINGS_FILE.path, TouchPadControlSettingEntries, sizeof(TouchPadControlSettingEntries) / sizeof(SettingEntry));
}

void saveTouchPadControlSettings() {
    writeSettings(TOUCHPAD_CONTROL_SETTINGS_FILE.path, TouchPadControlSettingEntries, sizeof(TouchPadControlSettingEntries) / sizeof(SettingEntry));
}

void loadDefaultTouchPadControlSettings() {
    memset(&TOUCHPAD_CONTROL_SETTINGS, 0, sizeof(TOUCHPAD_CONTROL_SETTINGS));
    readSettings(DEFAULT_TOUCHPAD_CONTROL_SETTINGS_FILE.path, TouchPadControlSettingEntries, sizeof(TouchPadControlSettingEntries) / sizeof(SettingEntry));
    writeSettings(TOUCHPAD_CONTROL_SETTINGS_FILE.path, TouchPadControlSettingEntries, sizeof(TouchPadControlSettingEntries) / sizeof(SettingEntry));
}

void toggleActiveDisplay() {
    SETTINGS.ACTIVE_DISPLAY = SETTINGS.ACTIVE_DISPLAY ? 0 : 1;
}

void toggleAltLayout() {
    SETTINGS.XINPUT_MAPPING = SETTINGS.XINPUT_MAPPING ? 0 : 1;
}

void setCleanSettingsUpdated(int updated) {
    CLEAN_TOUCHPAD_CONTROL_SETTINGS_UPDATED = updated;
}