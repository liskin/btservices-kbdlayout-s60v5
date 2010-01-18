/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This header file contains global declarations and methods 
*                  which are visible to all BTUI classes.
*
*/

#ifndef BTUI_H
#define BTUI_H

#include <e32svr.h>     // For RDebug and Panic
#include <data_caging_path_literals.hrh> 

const TUid KBtuiUid3 = { 0x10005951 }; //Application UID for help context

//Ecom implementation UID of GS interface plugin
const TUid KGsBtuiPluginUid = { 0x1020742B };   

//Ecom implementation UID of BTUI interface plugin
const TUid KBtuiPluginUidMainView = { 0x1020745C };   			// Ecom interface implementation UID
const TUid KBtuiPluginUidPairedDevicesView = { 0x1020745D };   	// Ecom interface implementation UID
const TUid KBtuiPluginUidBlockedDevicesView = {0x20002780 };	// Ecom interface implementation UID


_LIT( KBTUIAppName, "BTUI" );           // The application name

// Literals for bitmap files ( drive, directory, filename(s) )
_LIT(KBitmapFileDrive,"z:");
// KDC_APP_BITMAP_DIR == from data_caging_path_literals.hrh


_LIT( KGSBtuiPluginResourceFileName, "z:BtuiViewResources.rsc" );

_LIT(KBtuiBmpFileName,"btui.mbm");

_LIT(KFileDrive,"z:");
_LIT(KResourceFileName, "BtuiViewResources.rsc");


enum TBTPanics {    
    EBTMainListNotEmpty = 10000,
    EBTAllFormatStingsArrayNotEmpty,
    EBTFormatStingsArrayNotEmpty,
    EBTPairedDevicesListNotEmpty,
    EBTPanicClassMemberVariableNotNull,
    EBTPanicClassMemberVariableIsNull,    
    EBTPanicIllegalValue,
    EBTPanicDevManQueueIsCorrupt,
    EBTPanicDevManQueueIsInIllegalState,
	EBTPanicTwoPairingAttemptsAtTheSameTime,
	EBTPanicCommandComplettedToDiffrentDeviceThanStarted,
	EBTPanicNullObserver
};

// MACROS
//#define PANIC(aPanic) User::Panic(KBTUIAppName,aPanic)

#endif // BTUI_H
