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

#include "btnotifsecuritymanager.h"
#include "btnotifbasepairinghandler.h"
#include "btnotificationmanager.h"
#include "bluetoothnotification.h"
#include "btnotifconnectiontracker.h"
#include "bluetoothtrace.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifBasePairingHandler::CBTNotifBasePairingHandler( CBTNotifSecurityManager& aParent, const TBTDevAddr& aAddr)
    : iAddr( aAddr ), iParent( aParent )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::BaseConstructL( )
    {
    iActive = CBtSimpleActive::NewL( *this, 0 );  
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifBasePairingHandler::~CBTNotifBasePairingHandler()
    {
    delete iActive;
    }

// ---------------------------------------------------------------------------
// Message passes through only if the result is for the same device this 
// object is for.
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::HandlePairServerResult( const TBTDevAddr& aAddr, TInt aResult )
    {
    if ( aAddr == iAddr )
        {
         DoHandlePairServerResult( aResult );
        }
    }

// ---------------------------------------------------------------------------
// Message passes through only if the result is for the same device this 
// object is for.
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::HandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev )
    {
    if ( aDev.Address() == iAddr )
        {
        DoHandleRegistryNewPairedEvent( aDev );
        }
    }

// ---------------------------------------------------------------------------
// Default impl of virtual function. do nothing
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::CancelOutgoingPair()
    {
    }

// ---------------------------------------------------------------------------
// Default impl does not offer a known PIN code for pairing 
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::GetPinCode(
        TBTPinCode& aPin, const TBTDevAddr& aAddr, TInt aMinPinLength )
    {
    aPin().iLength = 0;
    (void) aAddr;
    (void) aMinPinLength;
    }

// ---------------------------------------------------------------------------
// Invalidate iPairResultSet
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::UnSetPairResult()
    {
    iPairResultSet = EFalse;
    }

// ---------------------------------------------------------------------------
// Save the result and validate the flag
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::SetPairResult( TInt aResult )
    {
    iPairResult = aResult;
    iPairResultSet = ETrue;
    }

// ---------------------------------------------------------------------------
// Returns the flag
// ---------------------------------------------------------------------------
//
TBool CBTNotifBasePairingHandler::IsPairResultSet()
    {
    return iPairResultSet;
    }

// ---------------------------------------------------------------------------
// Invalidate iPairResultSet
// ---------------------------------------------------------------------------
//
void CBTNotifBasePairingHandler::ShowPairingResultNoteL(TInt aResult)
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    CBluetoothNotification* notification = 
            iParent.ConnectionTracker().NotificationManager()->GetNotification();
    User::LeaveIfNull( notification ); // For OOM exception, leaves with KErrNoMemory
    TBTDialogResourceId resourceId = EPairingSuccess;
    if(KErrNone != aResult)
        {
        resourceId = EPairingFailureOk;
        }
    notification->SetNotificationType( TBluetoothDialogParams::ENote, resourceId );
    const CBtDevExtension* dev = iParent.BTDevRepository().Device(iAddr);
    if(dev)
        {
        User::LeaveIfError(notification->SetData( TBluetoothDeviceDialog::EDeviceName, dev->Alias()));
        User::LeaveIfError(notification->SetData( TBluetoothDeviceDialog::EDeviceClass, dev->Device().DeviceClass().DeviceClass()));
        }
    else
        {
        TBTDeviceName name;
        iAddr.GetReadable(name);
        User::LeaveIfError(notification->SetData( TBluetoothDialogParams::EAddress, name ));
        User::LeaveIfError(notification->SetData( TBluetoothDeviceDialog::EDeviceClass, 0)); // No device class
        }
    iParent.ConnectionTracker().NotificationManager()->QueueNotificationL( notification);
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }



