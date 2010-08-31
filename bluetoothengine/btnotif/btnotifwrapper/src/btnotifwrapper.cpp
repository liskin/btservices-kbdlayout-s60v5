/*
* ============================================================================
*  Name        : btnotifwrapper.cpp
*  Part of     : bluetoothengine / btnotif
*  Description : Wrapper for Bluetooth Notifiers
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.1
*/

#include "btnotifwrapper.h"
#include <btnotif.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifierspartner.h>
#endif

/**  Identifier of the active object. */
const TInt KWrapperActive = 10;

// ======== MEMBER FUNCTIONS ========


// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
CBTNotifWrapper::CBTNotifWrapper( const TUid& aUid, const TUid& aChannel )
:   MEikSrvNotifierBase2(),
    iUid( aUid ),
    iChannel( aChannel ),
    iResponsePtr( NULL, 0 )
    {
    }


// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
void CBTNotifWrapper::ConstructL()
    {
    // lazy initialization: members are created and destroyed when needed.
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
CBTNotifWrapper* CBTNotifWrapper::NewLC( const TUid& aUid, const TUid& aChannel )
    {
    CBTNotifWrapper* self = new( ELeave ) CBTNotifWrapper( aUid, aChannel );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
CBTNotifWrapper::~CBTNotifWrapper()
    {
    Cancel();   // Cleans up buffers
    delete iActive;
    iBTNotif.Close();
    }

// ---------------------------------------------------------------------------
// From class MEikSrvNotifierBase2.
// Frees all the resources i.e. destruct ourselves.
// ---------------------------------------------------------------------------
//
void CBTNotifWrapper::Release()
    {
    delete this;
    }

// ---------------------------------------------------------------------------
// From class MEikSrvNotifierBase2.
// ?implementation_description
// ---------------------------------------------------------------------------
//
MEikSrvNotifierBase2::TNotifierInfo CBTNotifWrapper::RegisterL()
    {
    return Info();
    }


// ---------------------------------------------------------------------------
// From class MEikSrvNotifierBase2.
// ?implementation_description
// ---------------------------------------------------------------------------
//
MEikSrvNotifierBase2::TNotifierInfo CBTNotifWrapper::Info() const
    {
    MEikSrvNotifierBase2::TNotifierInfo info;
    info.iUid = iUid;
    info.iChannel = iChannel;
    info.iPriority = MEikSrvNotifierBase2::ENotifierPriorityVHigh;
    return info;
    }


// ---------------------------------------------------------------------------
// From class MEikSrvNotifierBase2.
// ?implementation_description
// ---------------------------------------------------------------------------
//
TPtrC8 CBTNotifWrapper::StartL(const TDesC8& aBuffer)
    {
    if( !IsSync() )
        {
        // due to the charasteristics of Bluetooth and UI operations,
        // most of Bluetooth notifiers do not support synchronous version.
        User::Leave( KErrNotSupported );
        }
    // Call notifier server and get response.
    if( !iBTNotif.Handle() )
        {
        User::LeaveIfError( iBTNotif.Connect() );
        }
    User::LeaveIfError( iBTNotif.StartNotifier( iUid, aBuffer ) );
    iBTNotif.Close();
    return KNullDesC8();
    }

// ---------------------------------------------------------------------------
// From class MEikSrvNotifierBase2.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTNotifWrapper::StartL(const TDesC8& aBuffer, TInt aReplySlot, 
    const RMessagePtr2& aMessage)
    {
    if( !iMessage.IsNull() )
        {
        aMessage.Complete( KErrAlreadyExists );
        return;
        }

    // Call notifier server and get response.
    if( !iBTNotif.Handle() )
        {
        User::LeaveIfError( iBTNotif.Connect() );
        }
    if( !iActive )
        {
        iActive = CBtSimpleActive::NewL( *this, KWrapperActive );
        }

    // We need to store the parameters locally, as aBuffer is destroyed after
    // returning from this call. We do it on the heap, so we do not permanently
    // consume memory for the buffer.

    iParamsBuf.CreateL( aBuffer );
    TInt len = aMessage.GetDesMaxLength( aReplySlot );
    
    iResponseBuf.CreateL( len );
    aMessage.ReadL( aReplySlot, iResponseBuf );

    iBTNotif.StartNotifierAndGetResponse( iActive->RequestStatus(),
                iUid, iParamsBuf, iResponseBuf );
    iActive->GoActive();
    // record the request
    iReplySlot = aReplySlot;
    iMessage = aMessage;
    }

// ---------------------------------------------------------------------------
// From class MEikSrvNotifierBase2.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTNotifWrapper::Cancel()
    {
    // Call notifier server to cancel.
    if( iActive  )
        {
        iActive->Cancel(); 
        }
    if( !iMessage.IsNull() )
        {
        iMessage.Complete( KErrCancel );
        }
    iReplySlot = 0;
    iParamsBuf.Close();
    iResponseBuf.Close();
    }

// ---------------------------------------------------------------------------
// From class MEikSrvNotifierBase2.
// Synchronous notifier update.
// ---------------------------------------------------------------------------
//
TPtrC8 CBTNotifWrapper::UpdateL(const TDesC8& aBuffer)
    {
    // Call notifier server and get response.
    TBuf8<256> response;
    if( !iBTNotif.Handle() )
        {
        User::LeaveIfError( iBTNotif.Connect() );
        }
    User::LeaveIfError( iBTNotif.UpdateNotifier( iUid, aBuffer, response ) );
    return response;
    }

// ---------------------------------------------------------------------------
// From class MEikSrvNotifierBase2.
// Asynchronous notifier update.
// ---------------------------------------------------------------------------
//
void CBTNotifWrapper::UpdateL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    (void) aReplySlot;
    (void) aBuffer;
    if( iMessage.IsNull() )
        {
        // There is no outstanding request, can't relate this to anything.
        aMessage.Complete( KErrNotFound );
        return;
        }
    // Call notifier server and get response.
    // Async updates are just for updating parameters, so they are still 
    // done synchronously between here and btnotif, as they don't 
    // require any user feedback or other response with
    // asynchronous/long cycles.
    TPtrC8 response = UpdateL( aBuffer );
    aMessage.WriteL( aReplySlot, response );
    aMessage.Complete( KErrNone );
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Callback to notify that an outstanding request has completed.
// ---------------------------------------------------------------------------
//
void CBTNotifWrapper::RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus )
    {
    ASSERT( aActive->RequestId() == KWrapperActive );
    (void) aActive;
    if( !iMessage.IsNull() )
        {
        TInt err( aStatus );
        if( !err )
            {
            err = iMessage.Write( iReplySlot, iResponseBuf );
            }
        iMessage.Complete( err );
        }
    // Clean up after usage.
    iBTNotif.Close();
    delete iActive;
    iActive = NULL;
    }


// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Cancel and clean up all requests related to the active object.
// ---------------------------------------------------------------------------
//
void CBTNotifWrapper::CancelRequest( TInt aRequestId )
    {
    ASSERT( aRequestId == KWrapperActive );
    (void) aRequestId;
    iBTNotif.CancelNotifier( iUid );
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// 
// ---------------------------------------------------------------------------
//
void CBTNotifWrapper::HandleError( CBtSimpleActive* aActive, 
        TInt aError )
    {
    // RequestCompletedL() is not leavable.
    (void) aActive;
    (void) aError;
    }

// ---------------------------------------------------------------------------
// Check if this is a synchronous notifier or not.
// ---------------------------------------------------------------------------
//
TBool CBTNotifWrapper::IsSync() const
    {
    return ( iUid == KBTGenericInfoNotifierUid );
    }
