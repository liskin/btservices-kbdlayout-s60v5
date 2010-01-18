/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares Bluetooth notifiers SDK API.
*
*/



#ifndef BTNOTIFIERAPI_H
#define BTNOTIFIERAPI_H

// CONSTANTS

/**
* @file btnotifierapi.h 
* The UID of the Bluetooth Power Mode Notifier. This notifier checks the Bluetooth
* power status, If Bluetooth is off, a dialog is poped up asking user's 
* acceptance of switching on Bluetooth. The notifier is launched via 
* the notifier framework through RNotifier API. 
*
* Sample code of using RNotifier:
*
*  RNotifier notifier;
*  TRequestStatus status;
*  TPckgBuf <TBool> param, result;
*  notifier.StartNotifierAndGetResponse (status, KPowerModeSettingNotifierUid, param, result);
*/

const TUid KPowerModeSettingNotifierUid = {0x100059E2}; 

#endif // BTNOTIFIERAPI_H

// End of File
