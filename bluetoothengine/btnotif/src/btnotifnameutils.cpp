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
 * Description:  Declares Bluetooth notifiers base class.
 *
 */

#include <AknUtils.h>
#include <StringLoader.h>
#include <utf.h>

#include <BTNotif.rsg>

#include "btNotifDebug.h"
#include "btnotifnameutils.h"

/**
 * A function for strip dangerous chars out of BT name 
 * @param  a device name. After return, it contains the stripped name.
 * @return void
 */
void BtNotifNameUtils::StripDeviceName(TBTDeviceName& aDeviceNameForStrip)
    {	   
    aDeviceNameForStrip.Trim();
    // Filter out listbox non-friendly control chars.
    //
    AknTextUtils::StripCharacters(aDeviceNameForStrip, KAknStripListControlChars);  	             

    // Filter out sub-string-separator to pretend unexpected text layout.
    //
    TChar subStringSeparator = TChar(0x0001);
    TBuf<1> tempCharBuf;
    tempCharBuf.Append(subStringSeparator);
    AknTextUtils::StripCharacters(aDeviceNameForStrip, tempCharBuf); 
    }

/**
 * Gets the device name to be displayed in note or dialog.
 * @param aName contains the name to be displayed
 * @param aDev the device that may have a friendly or device name, or no name at all.
 */
void BtNotifNameUtils::GetDeviceDisplayName(TBTDeviceName& aName, const CBTDevice* aDev)
    {
    if(aDev && aDev->IsValidFriendlyName() )
        {
        aName.Copy( aDev->FriendlyName() );
        }
    else
        {
        GetDeviceName(aName, aDev);
        }
    }

/**
 * Gets the device name from aDev if its friendly name or device name is valid; otherwise 
 * the predefined default BT name.
 * @param aName contains the device name as the result 
 * @param aDev the device that may have a device name, or no name at all.
 */
void BtNotifNameUtils::GetDeviceName(TBTDeviceName& aName, const CBTDevice* aDev)
    {
    aName.Zero();
    TInt err (CnvUtfConverter::EErrorIllFormedInput);
    if(aDev && aDev->IsValidDeviceName())
        {
        err = CnvUtfConverter::ConvertToUnicodeFromUtf8(aName, aDev->DeviceName());
        }
    // if there are illformated chars in the device name, default name should be used for display.
    if (err == CnvUtfConverter::EErrorIllFormedInput || aName.Length() == 0)
        {
        StringLoader::Load(aName, R_BT_DIALOG_DEF_NAME);
        }
    StripDeviceName(aName);
    }

/**
 * Sets the device name.  This handles the conversion from unicode
 * to UTF8.
 * @param aDev the device that the name should be set for
 * @param aName the name to set
 */
void BtNotifNameUtils::SetDeviceNameL(const TBTDeviceName& aName, CBTDevice& aDev)
    {
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifierBase::SetDeviceNameL - Name: '%S' length: %d"), &aName, aName.Length() ));

    // Rationalise the name to remove whitespace and control characters
    // then set the name if there's anything left
    TBTDeviceName devName(aName);
    StripDeviceName(devName);
    if (aName.Length())
        {
        aDev.SetDeviceNameL(BTDeviceNameConverter::ToUTF8L(devName)); 
        }	
    }


