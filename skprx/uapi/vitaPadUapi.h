#ifndef VITAPAD_UAPI_H
#define VITAPAD_UAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#define VITAPAD_ERROR_DRIVER_NOT_REGISTERED		0x91337000
#define VITAPAD_ERROR_DRIVER_NOT_ACTIVATED		0x91337001
#define VITAPAD_ERROR_DRIVER_ALREADY_ACTIVATED	0x91337002


void vitaPadStart(int altLayout);
void vitaPadStop(void);
void vitaPadPreventSleep(void);
void vitaPadUpdateL2Pressed(int pressed);
void vitaPadUpdateR2Pressed(int pressed);
void vitaPadUpdateL3Pressed(int pressed);
void vitaPadUpdateR3Pressed(int pressed);

int vitaPadUsbAttached(void);
int kuCtrlReadBufferPositive(SceCtrlData *pad_data, int count);

#ifdef __cplusplus
}
#endif


#endif
