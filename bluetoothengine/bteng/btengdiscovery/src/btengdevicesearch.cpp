/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Helper class implementation for performing device discovery.
*
*/


#include "btengdevicesearch.h"
#include "btengdiscovery.h"
#include "debug.h"

/**
 * the request ID for device search using RNotifer.
 */
const TInt KDevSearchAoReqId = 40;

/*
 * the request ID for EIR service uuids using host resolver
 */
const TInt KDevEirServiceListAoReqId = 41;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngDeviceSearch::CBTEngDeviceSearch()
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngDeviceSearch::ConstructL()
    {
    User::LeaveIfError( iNotifier.Connect() );
    iActive = CBTEngActive::NewL( *this, KDevSearchAoReqId, 
                                   CActive::EPriorityStandard );
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngDeviceSearch* CBTEngDeviceSearch::NewL()
    {
    CBTEngDeviceSearch* self = new( ELeave ) CBTEngDeviceSearch();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngDeviceSearch::~CBTEngDeviceSearch()
    {
    delete iActive;
    iNotifier.Close();
    iHostResolver.Close();
    iSocketServer.Close();
    }

// ---------------------------------------------------------------------------
// Start the device discovery notifier.
// ---------------------------------------------------------------------------
//
TInt CBTEngDeviceSearch::StartSearch( CBTDevice* aDevice, TNameEntry* aNameEntry,
    const TBTDeviceClass& aDeviceClass, MBTEngSdpResultReceiver* aNotifier )
    {
    TRACE_FUNC_ENTRY
    TInt err ( KErrNone );
    if( iActive->IsActive() )
        {
        err = KErrInUse;
        }
    if( !err && !iNotifier.Handle() )
        {
        err = iNotifier.Connect();
        }
    if( !err )
        {
        iClientReq = EDeviceSearch;
        iResultNotifier = aNotifier;
        iNameEntry = aNameEntry;
        iDevice = aDevice;
            // Selection parameters
        TBTDeviceSelectionParams params;
        params.SetDeviceClass( aDeviceClass );
        iSelectionPckg = TBTDeviceSelectionParamsPckg( params );
            // Response parameters
        TBTDeviceResponseParams result;
        iResponsePckg = TBTDeviceResponseParamsPckg( result );
        iActive->SetRequestId( KDevSearchAoReqId );
        iNotifier.StartNotifierAndGetResponse( iActive->RequestStatus(), 
                                                KDeviceSelectionNotifierUid, 
                                                iSelectionPckg, iResponsePckg );
        iActive->GoActive();
        }
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// Cancel an ongoing device discovery.
// ---------------------------------------------------------------------------
//
void CBTEngDeviceSearch::CancelSearch()
    {
    TRACE_FUNC_ENTRY
    if( iActive->IsActive() && iClientReq == EDeviceSearch)
        {
        iActive->Cancel();
        if (iActive->RequestId() == KDevSearchAoReqId)
            {
            iNotifier.Close();
            NotifyClient(KErrAbort);
            }
        else if (iActive->RequestId() == KDevEirServiceListAoReqId)
            {
            iHostResolver.Close();
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Get cached EIR data from host resolver and extract service uuids
// ---------------------------------------------------------------------------
//
TInt CBTEngDeviceSearch::GetEirServiceUUIDs( const TBTDevAddr& aAddr, 
        TNameEntry* aNameEntry, MBTEngSdpResultReceiver* aNotifier)
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    if( iActive->IsActive() )
        {
        err = KErrInUse;
        }
    if (!err)
        {
        iClientReq = EGetDeviceEir;
        iResultNotifier = aNotifier;
        iNameEntry = aNameEntry;
        err = DoGetDeviceEir(aAddr);
        }
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// Cancel an ongoing EIR services retieval.
// ---------------------------------------------------------------------------
//
void CBTEngDeviceSearch::CancelGetEirServiceUUIDs()
    {
    TRACE_FUNC_ENTRY
    if( iActive->IsActive() && iClientReq == EGetDeviceEir)
        {
        iActive->Cancel();
        iHostResolver.Close();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Callback to notify that an outstanding request has completed.
// ---------------------------------------------------------------------------
//
void CBTEngDeviceSearch::RequestCompletedL( CBTEngActive* aActive, 
    TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "status: %d") , aStatus ) )
    (void) aActive;
    if ( aActive->RequestId() == KDevSearchAoReqId )
        {
        HandleDeviceSelectionResultL(aStatus);
        }
    else if ( aActive->RequestId() == KDevEirServiceListAoReqId )
        {
        HandleDeviceEirDataResult( aStatus );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles cancelation of an outstanding request
// ---------------------------------------------------------------------------
//
void CBTEngDeviceSearch::CancelRequest( TInt aRequestId )
    {
    TRACE_FUNC_ARG( ( _L( "reqID %d" ), aRequestId ) )
    if ( aRequestId == KDevSearchAoReqId )
        {
        iNotifier.CancelNotifier( KDeviceSelectionNotifierUid );
        }
    else if ( aRequestId == KDevEirServiceListAoReqId )
        {
        iHostResolver.Cancel();
        }
    TRACE_FUNC_EXIT 
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Callback to notify that an error has occurred in RunL.
// ---------------------------------------------------------------------------
//
void CBTEngDeviceSearch::HandleError( CBTEngActive* aActive, 
    TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "error: %d") , aError ) )
    (void) aActive;
    iNotifier.Close();
    iHostResolver.Close();
    NotifyClient(aError);
    }

// ---------------------------------------------------------------------------
// get device eir from cache
// ---------------------------------------------------------------------------
//
TInt CBTEngDeviceSearch::DoGetDeviceEir(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC_ENTRY
    TInt err (KErrNone);
    if (!iSocketServer.Handle())
        {
        err = iSocketServer.Connect();
        }
    
    TProtocolDesc pInfo;
    if (!err)
        {
        _LIT(KBTLinkManagerProtocol, "BTLinkManager");
        err =  iSocketServer.FindProtocol( KBTLinkManagerProtocol(), pInfo );
        }
    if (!err)
        {
        err = iHostResolver.Open(iSocketServer, pInfo.iAddrFamily, pInfo.iProtocol);
        }
    if( !err )
        {
        iInquirySockAddr = TInquirySockAddr();
        iInquirySockAddr.SetBTAddr(aAddr );
        iInquirySockAddr.SetAction( KHostResCache );
        iActive->SetRequestId( KDevEirServiceListAoReqId );
        iHostResolver.GetByAddress( iInquirySockAddr, *iNameEntry, iActive->RequestStatus() );
        iActive->GoActive();
        }
    TRACE_FUNC_EXIT
    return err;
    }

void CBTEngDeviceSearch::HandleDeviceSelectionResultL(TInt aErr)
    {
    TRACE_FUNC_ENTRY
    TInt err (aErr);
    iNotifier.Close();
    if (!err)
        {
        // Copy information to returned device
        iDevice->SetDeviceAddress( iResponsePckg().BDAddr() );
        iDevice->SetDeviceNameL( BTDeviceNameConverter::ToUTF8L( 
                                                iResponsePckg().DeviceName() ) );
        iDevice->SetDeviceClass( iResponsePckg().DeviceClass() );  
        }
    if (!err && iNameEntry)
        {
        err = DoGetDeviceEir(iResponsePckg().BDAddr());
        }
    // if an error happens, or the caller doesn't need the EIR part, complete
    // the request. Otherwise the client will be called after getting EIR 
    // completes.
    if (err || !iNameEntry)
        {
        NotifyClient(err);
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// extracting service uuids from the device EIR data returned by host resolver
// ---------------------------------------------------------------------------
//
void CBTEngDeviceSearch::HandleDeviceEirDataResult(TInt aErr)
    {
    TRACE_FUNC_ENTRY
    iHostResolver.Close();
    
    if ( iClientReq == EDeviceSearch)
        {
        // Device search has already completed correctly, so 
        // we don't pass GetEir error 
        NotifyClient( KErrNone );
        }
    else
        {
        NotifyClient( aErr );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Callback to notify that the caller's request has completed.
// ---------------------------------------------------------------------------
//
void CBTEngDeviceSearch::NotifyClient(TInt aErr)
    {
    TRACE_FUNC_ENTRY
    if( iResultNotifier && iClientReq == EDeviceSearch)
        {
        if (iNameEntry)
            {
            iResultNotifier->DeviceSearchComplete( iDevice, iNameEntry, aErr );
            }
        else
            {
            iResultNotifier->DeviceSearchComplete( iDevice, aErr );
            }
        }
    if (iResultNotifier && iClientReq == EGetDeviceEir)
        {
        iResultNotifier->GetEirServiceUUIDsComplete(iNameEntry, aErr);
        }
    TRACE_FUNC_EXIT
    }

