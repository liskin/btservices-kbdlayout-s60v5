/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef BTQTCONSTANTS_H
#define BTQTCONSTANTS_H

#include <btengconstants.h>
#include <btserversdkcrkeys.h>


// RSSI value range: -127dB ~ +20dB
const int RssiMinRange = -127;

//const int RssiMaxRange = 20; // maybe useful in the future

const int RssiMediumStrength = -75;

const int RssiHighStrength = -46;

const int RssiInvalid = RssiMinRange - 1;

enum PowerStateQtValue {
    BtPowerOff = 0,
    BtPowerOn,     
    BtPowerUnknown  // only for error situations
};

enum DisconnectOption {
    ServiceLevel = 0,
    PhysicalLink, 
    AllOngoingConnections,
    DisconUnknown
};

enum VisibilityMode {
    BtHidden = 0x10,  // using a different number space than TBTVisibilityMode
    BtVisible,
    BtTemporary,
    BtVisibilityUnknown
};

// used for mapping between UI row and VisibilityMode item
enum VisibilityModeUiRowMapping {
    UiRowBtHidden = 0,
    UiRowBtVisible,
    UiRowBtTemporary,
    UiRowBtUnknown
};


inline VisibilityMode QtVisibilityMode(TBTVisibilityMode btEngMode)
{
    VisibilityMode mode; 
    switch(btEngMode) {
    case EBTVisibilityModeHidden:
        mode = BtHidden;
        break;
    case EBTVisibilityModeGeneral:
        mode = BtVisible;
        break;
    case EBTVisibilityModeTemporary:
        mode = BtTemporary;
        break;
    default:
        mode = BtVisibilityUnknown;
    }
    return mode;
}

inline TBTVisibilityMode  BtEngVisibilityMode(VisibilityMode btQtMode)
{
    TBTVisibilityMode mode; 
    switch(btQtMode) {
    case BtHidden:
        mode = EBTVisibilityModeHidden;
        break;
    case BtVisible:
        mode = EBTVisibilityModeGeneral;
        break;
    case BtTemporary:
        mode = EBTVisibilityModeTemporary;
        break;
    default:
        mode = (TBTVisibilityMode)KErrUnknown;
    }
    return mode;
}

inline PowerStateQtValue QtPowerMode(TBTPowerStateValue btEngMode)
{
    PowerStateQtValue mode; 
    switch(btEngMode) {
    case EBTPowerOff:
        mode = BtPowerOff;
        break;
    case EBTPowerOn:
        mode = BtPowerOn;
        break;
    default:
        mode = BtPowerUnknown;  // error
    }
    return mode;
}

inline TBTPowerStateValue BtEngPowerState(PowerStateQtValue qtPowerState)
{
    TBTPowerStateValue btEngPowerState;
    switch (qtPowerState){
    case BtPowerOff:
        btEngPowerState = EBTPowerOff;
        break;
    case BtPowerOn:
        btEngPowerState = EBTPowerOn;
        break;
    default:
        btEngPowerState = (TBTPowerStateValue)KErrUnknown;
    }
    return btEngPowerState;
}

#endif // BTQTCONSTANTS_H
