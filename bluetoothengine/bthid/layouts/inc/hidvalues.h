/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  HID related enums
*
*/


#ifndef HIDVALUES_H
#define HIDVALUES_H

#include <e32std.h>

/**
 * Constants as defined in "USB HID Usage Tables", Version 1.11, USB
 * Implementers' Forum, June 2001.  Used throughout for parsing report
 * descriptors and interpreting reports.
 */

/**
 * HID usage page ID values, as given in the document "USB HID Usage
 * Tables", Version 1.11, USB Implementers' Forum, June 2001
 */
enum THidUsagePages
    {
    EUsagePageUndefined             = 0x00,
    EUsagePageGenericDesktop        = 0x01,
    EUsagePageSimulationControls    = 0x02,
    EUsagePageVRControls            = 0x03,
    EUsagePageSportControls         = 0x04,
    EUsagePageGameControls          = 0x05,
    EUsagePageGenericDeviceControls = 0x06,
    EUsagePageKeyboard              = 0x07,
    EUsagePageLEDs                  = 0x08,
    EUsagePageButton                = 0x09,
    EUsagePageOrdinal               = 0x0A,
    EUsagePageTelephony             = 0x0B,
    EUsagePageConsumer              = 0x0C,
    EUsagePageDigitizer             = 0x0D,
    EUsagePagePIDPage               = 0x0F,
    EUsagePageUnicode               = 0x10,
    EUsagePageAlphanumericDisplay   = 0x14,
    EUsagePageMedicalInstruments    = 0x40,
    EUsagePageMonitorMin            = 0x80,
    EUsagePageMonitorMax            = 0x83,
    EUsagePagePowerMin              = 0x84,
    EUsagePagePowerMax              = 0x87,
    EUsagePageBarCodeScanner        = 0x8C,
    EUsagePageScale                 = 0x8D,
    EUsagePageMagStripe             = 0x8E,
    EUsagePagePOS                   = 0x8F,
    EUsagePageCameraControl         = 0x90,
    EUsagePageArcade                = 0x91,
    EUsagePageVendorSpecific		= 0xFF01
    };

/**
 * HID usage ID values for the Generic Desktop usage page, as given in
 * the document "USB HID Usage Tables", Version 1.11, USB Implementers'
 * Forum, June 2001
 */
enum THidGenericDesktopUsages
    {
    EGenericDesktopUsagePointer       = 0x01,
    EGenericDesktopUsageMouse         = 0x02,
    EGenericDesktopUsageJoystick      = 0x04,
    EGenericDesktopUsagePad           = 0x05,
    EGenericDesktopUsageKeyboard      = 0x06,
    EGenericDesktopUsageKeypad        = 0x07,
    EGenericDesktopUsageMultiAxisCtrl = 0x08,
    EGenericDesktopUsageX             = 0x30,
    EGenericDesktopUsageY             = 0x31,
    EGenericDesktopUsageZ             = 0x32,
    EGenericDesktopUsageRx            = 0x33,
    EGenericDesktopUsageRy            = 0x34,
    EGenericDesktopUsageRz            = 0x35,
    EGenericDesktopUsageSlider        = 0x36,
    EGenericDesktopUsageDial          = 0x37,
    EGenericDesktopUsageWheel         = 0x38,
    EGenericDesktopUsageHatSwitch     = 0x39,
    EGenericDesktopUsageCountedBuffer = 0x3A,
    EGenericDesktopUsageByteCount     = 0x3B,
    EGenericDesktopUsageMotionWakeup  = 0x3C,
    EGenericDesktopUsageStart         = 0x3D,
    EGenericDesktopUsageSelect        = 0x3E
    };

enum THidTelephonyUsages
	{
	ETelephonyUsageHookSwitch       = 0x20,
	ETelephonyUsagePhoneMute		= 0x2F,
	ETelephonyUsagePoC              = 0x33
	};

enum THidConsumerUsages
	{
	EConsumerUsagePlay              = 0xB0,
	EConsumerUsageFastForward       = 0xB3,
	EConsumerUsageRewind            = 0xB4,
	EConsumerUsageScanNext          = 0xB5,
	EConsumerUsageScanPrev          = 0xB6,
	EConsumerUsageStop              = 0xB7,
	EConsumerUsageRandomPlay        = 0xB9,
	EConsumerUsagePlayPause         = 0xCD,
	EConsumerUsageVolumeInc         = 0xE9,
	EConsumerUsageVolumeDec         = 0xEA,
	EConsumerUsageMute              = 0xE2
	};

enum THidVendorSpecificUsages
	{
	EVendorSpecificUsagePoC = 0x01,
	EVendorSpecificUsageHeadplugDetection = 0x02
	};
// ----------------------------------------------------------------------

#endif
