/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  The main source file for the Mouse demo, altered for the La Fortuna
 *  board by js1n15, 2018
 */

#include "Mouse.h"

/** Buffer to hold the previously generated Mouse HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevMouseHIDReportBuffer[sizeof(USB_MouseReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Mouse_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Mouse,
				.ReportINEndpoint             =
					{
						.Address              = MOUSE_EPADDR,
						.Size                 = MOUSE_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevMouseHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevMouseHIDReportBuffer),
			},
	};

/* Current magnitude of mouse movement */
int mouse_delta = 5;

/* Previous direction of mouse movement */
int direction = _BV(SWN);

int position = 0;

int mouse_task(int state);
int ruota_task(int state);
void update_magnitude(uint8_t dir);

int main(void)
{
	os_init();
	mouse_init();

	os_add_task( mouse_task,	10,	1);
	os_add_task( ruota_task, 10,1);

	sei();

	for(;;){}
}

int ruota_task(int state) {
	/* scan switches for input */
	scan_switches();

	/* get rotary encoder position, for later use in scrolling? */
	position += os_enc_delta();
	return state;
}

/* Wrapper function for RIOS task (Mouse USB) */
int mouse_task(int state) {
	HID_Device_USBTask(&Mouse_HID_Interface);
	USB_USBTask();
	return state;
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void mouse_init(void) {
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	display_string("USB device connected\n");
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	display_string("USB device disconnected\n");
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = HID_Device_ConfigureEndpoints(&Mouse_HID_Interface);

	USB_Device_EnableSOFEvents();

	display_string(ConfigSuccess ? "USB device configured\n" : "USB device configuration failure occurred\n");	
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Mouse_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Mouse_HID_Interface);
}

/** Adapted for the La Fortuna board
 * 	HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_MouseReport_Data_t* MouseReport = (USB_MouseReport_Data_t*)ReportData;

	if (get_switch_press(_BV(SWN))) {
		display_string("Up ");
		update_magnitude(_BV(SWN));

		MouseReport->Y = -2 - mouse_delta;
	} else if (get_switch_press(_BV(SWS))) {
		display_string("Down ");
		update_magnitude(_BV(SWS));

	  	MouseReport->Y = 2 + mouse_delta;	
	}

	if (get_switch_press(_BV(SWW))) {
		display_string("Left ");
		update_magnitude(_BV(SWW));

	  	MouseReport->X = -2 - mouse_delta;
	} else if (get_switch_press(_BV(SWE))) {
		display_string("Right ");
		update_magnitude(_BV(SWE));

	  	MouseReport->X =  2 + mouse_delta;		
	}

	/* Left click */
	if (get_switch_press(_BV(SWC))) {
		display_string("Left-click ");
	  	MouseReport->Button |= (1 << 0);		
	}

	/* Right click */
	if (get_switch_rpt(_BV(SWC))) {
		display_string("Right-click ");
	  	MouseReport->Button |= (1 << 1);		
	}

	*ReportSize = sizeof(USB_MouseReport_Data_t);
	return true;
}

/* Update mouse_delta of mouse movement with given direction */
void update_magnitude(uint8_t dir) {
	/* If direction maintained, increase mouse_delta (capped at 50) */
	if (direction == dir) {
		if (mouse_delta + 5 > 50) {
			mouse_delta = 50;
		} else {
			mouse_delta += 5;
		}
	} else {
		/* Reset mouse_delta because direction has changed */
		mouse_delta = 5;
		direction = dir;
	}
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	// Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}


