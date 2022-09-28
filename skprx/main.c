#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/udcd.h>
#include <psp2kern/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/motion.h>
#include <taihen.h>
#include <stdio.h>
#include <string.h>
#include "uapi/vitaPadUapi.h"
#include "usbDescriptors.h"
#include "log.h"

#define PROPRIETARY_DRIVER_ID "VITAPAD"
#define PROPRIETARY_USB_PID	0x1337

struct GamepadReport {
	uint8_t report_id;
	uint16_t buttons;
	int8_t left_x;
	int8_t left_y;
	int8_t right_x;
	int8_t right_y;
} __attribute__((packed));

#define EVF_CONNECTED		(1 << 0)
#define EVF_DISCONNECTED	(1 << 1)
#define EVF_EXIT		(1 << 2)
#define EVF_INT_REQ_COMPLETED	(1 << 3)
#define EVF_ALL_MASK		(EVF_INT_REQ_COMPLETED | (EVF_INT_REQ_COMPLETED - 1))

static SceUID USB_THREAD_ID;
static SceUID USB_EVENT_FLAG_ID;
static int USB_ATTACHED = 2;

static int PROPRIETARY_DRIVER_REGISTERED = 0;
static int PROPRIETARY_DRIVER_ACTIVATED = 0;
static int SHOULD_EXIT_THREAD = 0;

static int L2_PRESSED;
static int R2_PRESSED;
static int L3_PRESSED;
static int R3_PRESSED;

static int sendDescHidReport(void)
{
	static SceUdcdDeviceRequest req = {
		.endpoint = &endpoints[0],
		.data = hid_report_descriptor,
		.size = sizeof(hid_report_descriptor),
		.isControlRequest = 0,
		.onComplete = NULL,
		.transmitted = 0,
		.returnCode = 0,
		.next = NULL,
		.unused = NULL,
		.physicalAddress = NULL
	};

	return ksceUdcdReqSend(&req);
}

static int sendStringDescriptor(int index)
{
	static SceUdcdDeviceRequest req = {
		.endpoint = &endpoints[0],
		.data = &string_descriptors[0],
		.size = sizeof(string_descriptors[0]),
		.isControlRequest = 0,
		.onComplete = NULL,
		.transmitted = 0,
		.returnCode = 0,
		.next = NULL,
		.unused = NULL,
		.physicalAddress = NULL
	};

	return ksceUdcdReqSend(&req);
}

static int sendInitialHidReport(uint8_t report_id)
{
	static struct GamepadReport gamepad __attribute__((aligned(64))) = {
		.report_id = 1,
		.buttons = 0,
		.left_x = 0,
		.left_y = 0,
		.right_x = 0,
		.right_y = 0
	};

	static SceUdcdDeviceRequest req = {
		.endpoint = &endpoints[0],
		.data = &gamepad,
		.size = sizeof(gamepad),
		.isControlRequest = 0,
		.onComplete = NULL,
		.transmitted = 0,
		.returnCode = 0,
		.next = NULL,
		.unused = NULL,
		.physicalAddress = NULL
	};

	return ksceUdcdReqSend(&req);
}

static void handleHidReportComplete(SceUdcdDeviceRequest *req)
{
	ksceKernelSetEventFlag(USB_EVENT_FLAG_ID, EVF_INT_REQ_COMPLETED);
}

static void fillGamepadReport(const SceCtrlData *pad, struct GamepadReport *gamepad)
{
	gamepad->report_id = 1;
	gamepad->buttons = 0;

	if (pad->buttons & SCE_CTRL_SQUARE)
		gamepad->buttons |= 1 << 0;
	if (pad->buttons & SCE_CTRL_CROSS)
		gamepad->buttons |= 1 << 1;
	if (pad->buttons & SCE_CTRL_CIRCLE)
		gamepad->buttons |= 1 << 2;
	if (pad->buttons & SCE_CTRL_TRIANGLE)
		gamepad->buttons |= 1 << 3;

	if (pad->buttons & SCE_CTRL_LTRIGGER)
		gamepad->buttons |= 1 << 4;
	if (pad->buttons & SCE_CTRL_RTRIGGER)
		gamepad->buttons |= 1 << 5;

	if (L2_PRESSED == 1)
		gamepad->buttons |= 1 << 6;
	if (R2_PRESSED == 1)
		gamepad->buttons |= 1 << 7;

	if (pad->buttons & SCE_CTRL_SELECT)
		gamepad->buttons |= 1 << 8;
	if (pad->buttons & SCE_CTRL_START)
		gamepad->buttons |= 1 << 9;

	if (L3_PRESSED == 1)
		gamepad->buttons |= 1 << 10;
	if (R3_PRESSED == 1)
		gamepad->buttons |= 1 << 11;

	if (pad->buttons & SCE_CTRL_UP)
		gamepad->buttons |= 1 << 12;
	if (pad->buttons & SCE_CTRL_DOWN)
		gamepad->buttons |= 1 << 13;
	if (pad->buttons & SCE_CTRL_RIGHT)
		gamepad->buttons |= 1 << 14;
	if (pad->buttons & SCE_CTRL_LEFT)
		gamepad->buttons |= 1 << 15;

	gamepad->left_x = (int8_t)pad->lx - 128;
	gamepad->left_y = (int8_t)pad->ly - 128;
	gamepad->right_x = (int8_t)pad->rx - 128;
	gamepad->right_y = (int8_t)pad->ry - 128;
}

static int sendHidReport()
{
	static struct GamepadReport gamepad __attribute__((aligned(64))) = { 0 };
	SceCtrlData pad;

	ksceCtrlPeekBufferPositive(0, &pad, 1);
	fillGamepadReport(&pad, &gamepad);
	ksceKernelCpuDcacheAndL2WritebackRange(&gamepad, sizeof(gamepad));

	static SceUdcdDeviceRequest req = {
		.endpoint = &endpoints[1],
		.data = &gamepad,
		.size = sizeof(gamepad),
		.isControlRequest = 0,
		.onComplete = handleHidReportComplete,
		.transmitted = 0,
		.returnCode = 0,
		.next = NULL,
		.unused = NULL,
		.physicalAddress = NULL
	};

	return ksceUdcdReqSend(&req);
}

static int processUdcdRequest(int recipient, int arg, SceUdcdEP0DeviceRequest *req, void *user_data)
{
	if (arg < 0)
		return -1;

	uint8_t req_dir = req->bmRequestType & USB_CTRLTYPE_DIR_MASK;
	uint8_t req_type = req->bmRequestType & USB_CTRLTYPE_TYPE_MASK;
	uint8_t req_recipient = req->bmRequestType & USB_CTRLTYPE_REC_MASK;

	if (req_dir == USB_CTRLTYPE_DIR_DEVICE2HOST) {
		switch (req_type) {
		case USB_CTRLTYPE_TYPE_STANDARD:
			switch (req_recipient) {
			case USB_CTRLTYPE_REC_DEVICE:
				switch (req->bRequest) {
				case USB_REQ_GET_DESCRIPTOR: {
					uint8_t descriptor_type = (req->wValue >> 8) & 0xFF;
					uint8_t descriptor_idx = req->wValue & 0xFF;

					switch (descriptor_type) {
					case USB_DT_STRING:
						sendStringDescriptor(descriptor_idx);
						break;
					}
					break;
				}

				}
				break;
			case USB_CTRLTYPE_REC_INTERFACE:
				switch (req->bRequest) {
				case USB_REQ_GET_DESCRIPTOR: {
					uint8_t descriptor_type = (req->wValue >> 8) & 0xFF;
					uint8_t descriptor_idx = req->wValue & 0xFF;

					switch (descriptor_type) {
					case HID_DESCRIPTOR_REPORT:
						sendDescHidReport();
						break;
					}
				}

				}
				break;
			}
			break;
		case USB_CTRLTYPE_TYPE_CLASS:
			switch (recipient) {
			case USB_CTRLTYPE_REC_INTERFACE:
				switch (req->bRequest) {
				case HID_REQUEST_GET_REPORT: {
					uint8_t report_type = (req->wValue >> 8) & 0xFF;
					uint8_t report_id = req->wValue & 0xFF;

					if (report_type == 1)
						sendInitialHidReport(report_id);
					break;
				}

				}
				break;
			}
			break;
		}
	} else if (req_dir == USB_CTRLTYPE_DIR_HOST2DEVICE) {
		switch (req_type) {
		case USB_CTRLTYPE_TYPE_CLASS:
			switch (req_recipient) {
			case USB_CTRLTYPE_REC_INTERFACE:
				switch (req->bRequest) {
				case HID_REQUEST_SET_IDLE:
					break;
				}
				break;
			}
			break;
		}
	}

	return 0;
}

static int attachUdcd(int usb_version, void *user_data)
{
	ksceUdcdReqCancelAll(&endpoints[1]);
	ksceUdcdClearFIFO(&endpoints[1]);
	ksceKernelSetEventFlag(USB_EVENT_FLAG_ID, EVF_CONNECTED);
    USB_ATTACHED = 1;
	return 0;
}

static void detachUdcd(void *user_data)
{
	ksceKernelSetEventFlag(USB_EVENT_FLAG_ID, EVF_DISCONNECTED);
    USB_ATTACHED = 0;
}

static int changeUdcdSetting() {
    return 0;
}

static void configureUdcd() {

}

static void startProprietaryDriver() {

}

static void stopProprietaryDriver() {

}

SceUdcdDriver PROPRIETARY_UDCD_DRIVER = {
	PROPRIETARY_DRIVER_ID,
	2,
	&endpoints[0],
	&interfaces[0],
	&devdesc_hi,
	&config_hi,
	&devdesc_full,
	&config_full,
	&string_descriptors[0],
	&string_descriptors[0],
	&string_descriptors[0],
	&processUdcdRequest,
	&changeUdcdSetting,
	&attachUdcd,
	&detachUdcd,
	&configureUdcd,
	&startProprietaryDriver,
	&stopProprietaryDriver,
	0,
	0,
	NULL
};

static int USB_THREAD(SceSize args, void *argp)
{
    ksceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    while (1) {
        int EVF_OUT;
        int response = ksceKernelWaitEventFlagCB(USB_EVENT_FLAG_ID, EVF_ALL_MASK, SCE_EVENT_WAITOR | SCE_EVENT_WAITCLEAR_PAT, &EVF_OUT, NULL);
        if (response < 0 || SHOULD_EXIT_THREAD == 1) {
            break;
        }
        sendHidReport();
    }

    return 0;
}

int createGamepadThread() {
    USB_THREAD_ID = ksceKernelCreateThread("VITAPAD_USB_THREAD", USB_THREAD, 0x3C, 0x1000, 0, 0x10000, 0);
    return USB_THREAD_ID;
}

int removeGamepadThread() {
    return ksceKernelDeleteThread(USB_THREAD_ID);
}

int startGamepadThread() {
    ksceKernelStartThread(USB_THREAD_ID, 0, NULL);
}

int createGamepadFlag() {
    USB_EVENT_FLAG_ID = ksceKernelCreateEventFlag("VITAPAD_EVENT_FLAG", 0, 0, NULL);
    return USB_EVENT_FLAG_ID;
}

int removeGamepadFlag() {
    return ksceKernelDeleteEventFlag(USB_EVENT_FLAG_ID);
}

int registerGamepad() {
    PROPRIETARY_DRIVER_REGISTERED = 1;
    return ksceUdcdRegister(&PROPRIETARY_UDCD_DRIVER);
}

int unregisterGamepad() {
    PROPRIETARY_DRIVER_REGISTERED = 0;
    return ksceUdcdUnregister(&PROPRIETARY_UDCD_DRIVER);
}

int startDriver() {
    return ksceUdcdStart(PROPRIETARY_DRIVER_ID, 0, 0);
}

int stopDriver() {
    return ksceUdcdStop(PROPRIETARY_DRIVER_ID, 0, 0);
}

int activateDriver() {
    return ksceUdcdActivate(PROPRIETARY_USB_PID);
}

int deactivateDriver() {
    return ksceUdcdDeactivate();
}

int disconnectDriver() {
    return ksceKernelSetEventFlag(USB_EVENT_FLAG_ID, EVF_DISCONNECTED);
}

int stopUsbDrivers() {
    ksceUdcdStop("USB_MTP_Driver", 0, NULL);
    ksceUdcdStop("USBPSPCommunicationDriver", 0, NULL);
    ksceUdcdStop("USBSerDriver", 0, NULL);
    ksceUdcdStop("USBDeviceControllerDriver", 0, NULL);
}

int resetGamepad() {
    unregisterGamepad();
    removeGamepadThread();
    removeGamepadFlag();
}

void vitaPadStart(void)
{
    log_reset();

    SHOULD_EXIT_THREAD = 0;
    USB_ATTACHED = 2;

    deactivateDriver();
    stopUsbDrivers();

    registerGamepad();
    startGamepadThread();
    ksceUdcdStart("USBDeviceControllerDriver", 0, NULL);
    startDriver();
    activateDriver();

    ksceKernelClearEventFlag(USB_EVENT_FLAG_ID, ~EVF_ALL_MASK);

	PROPRIETARY_DRIVER_ACTIVATED = 1;
}

void vitaPadStop(void)
{
    SHOULD_EXIT_THREAD = 1;

    ksceKernelClearEventFlag(USB_EVENT_FLAG_ID, EVF_ALL_MASK);
    ksceKernelSetEventFlag(USB_EVENT_FLAG_ID, EVF_EXIT);
    ksceKernelWaitThreadEnd(USB_THREAD_ID, NULL, NULL);
	stopDriver();
    stopUsbDrivers();

    ksceUdcdStart("USBDeviceControllerDriver", 0, NULL);
    ksceUdcdStart("USB_MTP_Driver", 0, NULL);
	ksceUdcdActivate(0x4E4);

	PROPRIETARY_DRIVER_ACTIVATED = 0;
}

void vitaPadUpdateL2Pressed(int pressed){
    L2_PRESSED = pressed;
}

void vitaPadUpdateR2Pressed(int pressed){
    R2_PRESSED = pressed;
}

void vitaPadUpdateL3Pressed(int pressed){
    L3_PRESSED = pressed;
}

void vitaPadUpdateR3Pressed(int pressed){
    R3_PRESSED = pressed;
}

int vitaPadUsbAttached() {
    return USB_ATTACHED;
}

void _start() __attribute__((weak, alias("module_start")));

int module_start(SceSize argc, const void *args)
{
    createGamepadThread();
    createGamepadFlag();
    registerGamepad();

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    vitaPadStop();
    resetGamepad();

	return SCE_KERNEL_STOP_SUCCESS;
}
