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
* Description:  the base class of pairing handling 
*
*/

#include "btengpairman.h"
#include "btengpairbase.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngPairBase::CBTEngPairBase( CBTEngPairMan& aParent, const TBTDevAddr& aAddr)
    : iAddr( aAddr ), iParent( aParent )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngPairBase::BaseConstructL( )
    {
    iActive = CBTEngActive::NewL(*this, 0, CActive::EPriorityStandard);  
    TRACE_BDADDR( iAddr );
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngPairBase::~CBTEngPairBase()
    {
    CancelNotifier();
    iNotifier.Close();
    delete iActive;
    }

// ---------------------------------------------------------------------------
// Message passes through only if the result is for the same device this 
// object is for.
// ---------------------------------------------------------------------------
//
void CBTEngPairBase::HandlePairServerResult( const TBTDevAddr& aAddr, TInt aResult )
    {
    if ( aAddr == iAddr )
        {
        TRACE_FUNC_ENTRY
        DoHandlePairServerResult( aResult );
        TRACE_FUNC_EXIT
        }
    }

// ---------------------------------------------------------------------------
// Message passes through only if the result is for the same device this 
// object is for.
// ---------------------------------------------------------------------------
//
void CBTEngPairBase::HandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev )
    {
    if ( aDev.Address() == iAddr )
        {
        TRACE_FUNC_ENTRY
        DoHandleRegistryNewPairedEvent( aDev );
        TRACE_FUNC_EXIT
        }
    }

// ---------------------------------------------------------------------------
// Default impl of virtual function. do nothing
// ---------------------------------------------------------------------------
//
void CBTEngPairBase::CancelOutgoingPair()
    {
    }

// ---------------------------------------------------------------------------
// Cancel outstanding notifier
// ---------------------------------------------------------------------------
//
void CBTEngPairBase::CancelNotifier()
    {
    if( iActive && iActive->IsActive() && 
        iActive->RequestId() == EDevicePairUserNotification )
        {
        TRACE_FUNC_ENTRY
        iNotifier.CancelNotifier( KBTPairedDeviceSettingNotifierUid );
        iActive->Cancel();
        TRACE_FUNC_EXIT
        }
    }

// ---------------------------------------------------------------------------
// Show the pairing result and ask the user to authorize the device.
// ---------------------------------------------------------------------------
//
void CBTEngPairBase::ShowPairingNoteAndAuthorizeQuery()
    {
    TRACE_FUNC_ENTRY
    if ( iActive->IsActive() )
        {
        // In profress of showing note, won't interrupt it:
        return;
        }
    TInt err ( KErrNone );
    if (!iNotifier.Handle())
        {
        err = iNotifier.Connect();
        }
    if ( !err )
        {
        // Inform the user of the pairing status.
        iActive->SetRequestId( EDevicePairUserNotification );
        iAuthoPckg().iPairingStatus = iPairResult;
        iAuthoPckg().iRemoteAddr = iAddr;
        iNotifier.StartNotifierAndGetResponse( iActive->RequestStatus(), 
            KBTPairedDeviceSettingNotifierUid, 
            iAuthoPckg, iAuthoPckg ); 
        iActive->GoActive();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Invalidate iPairResultSet
// ---------------------------------------------------------------------------
//
void CBTEngPairBase::UnSetPairResult()
    {
    iPairResultSet = EFalse;
    }

// ---------------------------------------------------------------------------
// Save the result and validate the flag
// ---------------------------------------------------------------------------
//
void CBTEngPairBase::SetPairResult( TInt aResult )
    {
    iPairResult = aResult;
    iPairResultSet = ETrue;
    }

// ---------------------------------------------------------------------------
// Returns the flag
// ---------------------------------------------------------------------------
//
TBool CBTEngPairBase::IsPairResultSet()
    {
    return iPairResultSet;
    }

// ---------------------------------------------------------------------------
// check AO validity and its request information
// ---------------------------------------------------------------------------
//
TBool CBTEngPairBase::IsNotifyingPairResult()
    {
    return iActive && iActive->IsActive() && 
        iActive->RequestId() == EDevicePairUserNotification ;
    }

