/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/
#ifndef DEVUI_CONST_H_
#define DEVUI_CONST_H_

#include <btdevice.h>
// Order must match the one in resources (R_BT_DEFAULT_DEVICE_NAMES)
//
enum TBTDefaultDevNameIndex
    {
    EBTDeviceNameIndexComputer = 0,
    EBTDeviceNameIndexPhone,
    EBTDeviceNameIndexAudio,
    EBTDeviceNameIndexKeyboard,
    EBTDeviceNameIndexMouse,
    EBTDeviceNameIndexPrinter,
    EBTDeviceNameIndexCarKit,
    EBTDeviceNameIndexDefault
    };

// Enum for items in search filter
enum TBTSearchFilterItem
    {
    EBTSearchForAll = 0,
    EBTSearchForAudio,
    EBTSearchForPhone,
    EBTSearchForComputer,
    EBTSearchForInput
    };

// Order must match the one in resources (R_BTNOTIF_ICONS)
//
enum TDeviceIconIndex
    {
    EDeviceIconComputer = 0,
    EDeviceIconPhone,
    EDeviceIconAudio,
    EDeviceIconDefault,
    EDeviceIconBlank,
    EDeviceIconKeyboard,
    EDeviceIconMouse,
    EDeviceIconPrinter,
    EDeviceIconCarkit,
    EDeviceIconPaired, 
    EDeviceIconBlocked,
    EDeviceIconRssiLow, 
    EDeviceIconRssiMed,
    EDeviceIconRssiGood,
    };

struct TDeviceIconFormat
    {
    TDeviceIconIndex iIdx;
    const TText* iFormat;
    };

// Order must match the one in TDeviceIconIndex
//
const TDeviceIconFormat KDeviceIconFormatTable[] =
    {
    {EDeviceIconComputer, _S("0\t")},  //computer
    {EDeviceIconPhone,    _S("1\t")},  // phone
    {EDeviceIconAudio,    _S("2\t")},  // audio
    {EDeviceIconDefault,  _S("3\t")},  // default
    {EDeviceIconBlank,    _S("4\t")},  // blank
    {EDeviceIconKeyboard, _S("5\t")},  // keyboard
    {EDeviceIconMouse,    _S("6\t")},  // mouse
    {EDeviceIconPrinter,  _S("7\t")},  // printer
    {EDeviceIconCarkit,   _S("8\t")},  // carkit
    {EDeviceIconPaired,   _S("\t9")},  // paired
    {EDeviceIconBlocked,  _S("\t10")}, // blocked
    {EDeviceIconRssiLow,  _S("\t11")}, // RssiLow
    {EDeviceIconRssiMed,  _S("\t12")}, // RssiMed
    {EDeviceIconRssiGood, _S("\t13")}  // RssiGood
    };

// Total number of different device icons
const TInt KDeviceIconCount = 
    sizeof(KDeviceIconFormatTable) / sizeof(TDeviceIconFormat);

/*
 * The classification of a device's icon and default name
 * corresponding to its major and minor device classes.
 */
struct TDeviceRowLayout
    {
    TInt iMajorDevClass;
    TInt iMinorDevClass;
    TDeviceIconIndex iIconIndex;
    TBTDefaultDevNameIndex iDefaultNameIndex;
    };

/*
 * Device classification mapping table.
 * 
 * Notice 1: Considering runtime searching efficiency, if certain major or minor 
 * device classes shall use the default icon and name, it is not recommended 
 * to present in this table. 
 * 
 * Notice 2: Regulated by searching algorithm, when devices carrying the same 
 * major device class may have different icons and names for different minor 
 * device classes, the unclassified minor device class must present at last 
 * among these belonging to the same major device class.
 */
const TDeviceRowLayout KDeviceRowLayoutTable[] = 
    {
    {EMajorDeviceComputer,    0,                              EDeviceIconComputer, EBTDeviceNameIndexComputer },
    {EMajorDevicePhone,       0,                              EDeviceIconPhone,    EBTDeviceNameIndexPhone }, 
    {EMajorDeviceAudioDevice, EMinorDeviceAVCarAudio,         EDeviceIconCarkit,   EBTDeviceNameIndexCarKit }, 
    {EMajorDeviceAudioDevice, 0,                              EDeviceIconAudio,    EBTDeviceNameIndexAudio }, 
    {EMajorDeviceImaging,     EMinorDeviceImagingPrinter,     EDeviceIconPrinter,  EBTDeviceNameIndexPrinter }, 
    {EMajorDevicePeripheral,  EMinorDevicePeripheralKeyboard, EDeviceIconKeyboard, EBTDeviceNameIndexKeyboard }, 
    {EMajorDevicePeripheral,  EMinorDevicePeripheralPointer,  EDeviceIconMouse,    EBTDeviceNameIndexMouse  }, 
    };

const TInt KDeviceRowLayoutTableSize = (sizeof(KDeviceRowLayoutTable) / sizeof(TDeviceRowLayout));

#endif /*DEVUI_CONST_H_*/
