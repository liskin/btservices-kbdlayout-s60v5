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
* Description:  class for searching BT devices
*
*/
#include "basicdevdiscoverer.h"
#include <btservices/devdiscoveryobserver.h>
#include "btserviceutilconsts.h"

// ----------------------------------------------------------
// CBasicDevDiscoverer::CBasicDevDiscoverer
// ----------------------------------------------------------
//
CBasicDevDiscoverer::CBasicDevDiscoverer( MDevDiscoveryObserver& aObserver )
    : iObserver( aObserver )
    {
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::ConstructL
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::ConstructL()
    {
    User::LeaveIfError( iSocketServer.Connect() );
    iActive = CBtSimpleActive::NewL( *this, BtServiceUtil::EBluetoothInquiry );
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::NewL
// ----------------------------------------------------------
//
CBasicDevDiscoverer* CBasicDevDiscoverer::NewL( MDevDiscoveryObserver& aObserver )
    {
    CBasicDevDiscoverer* self = new (ELeave) 
            CBasicDevDiscoverer( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::
// ----------------------------------------------------------
//
CBasicDevDiscoverer::~CBasicDevDiscoverer()
    {
    delete iActive;
    Reset();
    iSocketServer.Close();
    iDevices.Close();
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::SetObserver
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::SetObserver( MDevDiscoveryObserver& aObserver )
    {
    iObserver = aObserver;
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::Cancel()
    {
    iActive->Cancel();
    iHostResolver.Close();
    iDevices.ResetAndDestroy();
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::DiscoverDeviceL(TBTMajorDeviceClass aDeviceClass )
    {
    // This class supports only one request at the time:
    if ( iActive->IsActive() )
        {
        User::Leave( KErrInUse );
        }
    Reset();
    iMajorDeviceClassFilter = aDeviceClass;
    _LIT( KLinkMgrDes, "BTLinkManager" );
    // Associate with bluetooth Link Manager.
    TProtocolName protocol( KLinkMgrDes );
    TProtocolDesc pInfo;
    User::LeaveIfError( iSocketServer.FindProtocol( protocol, pInfo));
    User::LeaveIfError( iHostResolver.Open(iSocketServer, 
            pInfo.iAddrFamily, pInfo.iProtocol));
    iActive->SetRequestId( BtServiceUtil::EBluetoothInquiry );
    iInquirySockAddr.SetAction( KHostResInquiry + KHostResEir + KHostResIgnoreCache );
    // We always do Generic Inquiry.
    // Limited Inquiry could be added here in future on business need.
    iInquirySockAddr.SetIAC(KGIAC);
    iHostResolver.GetByAddress( iInquirySockAddr, iEntry, iActive->RequestStatus() );
    iActive->GoActive();
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::RequestCompletedL
// Inform caller for received device and issue next EIR/Name request
// if the request was successful.
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus )
    {
    TInt errToObserver( aStatus );
    // position in array iDevices: the device item the observer will be notified of.
    TInt devToNotify ( KErrNotFound ); 
    
    if ( aActive->RequestId() == BtServiceUtil::EBluetoothInquiry )
        {
        if ( aStatus == KErrNone )
            {
            TInt pos = HandleInquiryResultL();
            // continue to inquiry for more devices in range
            iHostResolver.Next( iEntry, iActive->RequestStatus() );
            iActive->GoActive();
            if ( pos > KErrNotFound && iDevices[pos]->iName.Length() > 0 )
                {
                devToNotify = pos;
                }
            }
        else if( iDevices.Count() > 0 )
            {
            // an error from inquiry operation. 
            // we move to next step to get device names if some devices have been
            // found but without a name.
            iPagingNamePos = iDevices.Count() - 1;
            aActive->SetRequestId( BtServiceUtil::EBluetoothPageDeviceName );
            PageNextDeviceName();
            }
        }
    else if ( aActive->RequestId() ==  BtServiceUtil::EBluetoothPageDeviceName )
        {
        errToObserver = KErrNone;
        devToNotify = iPagingNamePos;
        // the name in iEntry was reset before paging operation, so we
        // can rely on this length() at this time:
        if ( aStatus == KErrNone && iEntry().iName.Length() > 0 )
            {
            iDevices[iPagingNamePos]->iName = iEntry().iName;
            }
            // the return error is not checked here. We continue to page the rest
            // device names.
            --iPagingNamePos;
            PageNextDeviceName();
        }
    // request ID is BtServiceUtil::EAsyncNotifyDeviceSearchCompleted
    else 
        {
        iObserver.HandleDiscoveryCompleted( errToObserver );
        return;
        }
    
    // AO not active means that this is neither inquiring nor paging name.
    // Schedule an operation completion callback:
    if ( !iActive->IsActive() && 
            aActive->RequestId() != BtServiceUtil::EAsyncNotifyDeviceSearchCompleted )
        {
        // We inform the client of operation completion asynchronously, so that
        // we will not end up with problems, e.g., invalid memory,
        // if the client issues more request in the callback context.
        aActive->SetRequestId( BtServiceUtil::EAsyncNotifyDeviceSearchCompleted );
        aActive->RequestStatus() = KRequestPending;
        TRequestStatus* sta = &aActive->RequestStatus();
        User::RequestComplete( sta, errToObserver );
        aActive->GoActive();
        }
    
    // This could be possible in both inquiry and paging operations.
    if ( devToNotify > KErrNotFound )
        {
        // This device record is not used any more after we have informed client.
        // Extract it and push to cleanup for detroy.
        // This is to prevent peak memory usage in case of a great number of 
        // devices being in range.
        CDeviceSearchRecord* rec = iDevices[devToNotify];
        iDevices.Remove( devToNotify );
        CleanupStack::PushL( rec );
        iObserver.HandleNextDiscoveryResultL( rec->iAddr, rec->iName );
        CleanupStack::PopAndDestroy( rec );
        }
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::DoCancelRequest
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::CancelRequest( TInt aId )
    {
    // host resolver needs to be cancelled.
    // For request BtServiceUtil::EAsyncNotifyDeviceSearchCompleted, we 
    // are doing self-completing. Thus, nothing is needed now.
    if ( aId == BtServiceUtil::EBluetoothInquiry || 
         aId == BtServiceUtil::EBluetoothPageDeviceName )
        {
        iHostResolver.Cancel();
        }
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::HandleError
// Inform UI from error occured.
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::HandleError( CBtSimpleActive* aActive, TInt aError )
    {
    // We might have issued an request to Host Resolver in RequestCompleted().
    // Cancel AO just in case:
    aActive->Cancel();
    Reset();
    // We cannot proceed more. Inform client:
    iObserver.HandleDiscoveryCompleted( aError );
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::PageNextDeviceName()
    {
    // reset the name in entry so that previous result will
    // not propogate if the next paging operation fails.
    iEntry().iName.Zero();
    for (; iPagingNamePos > -1; --iPagingNamePos )
        {
        // Get the next in-range device that has no device name yet 
        // This is practically meaning that the device would be
        // < v2.1
        if( iDevices[iPagingNamePos]->iName.Length() == 0 )
            {
            iInquirySockAddr.SetAction( KHostResName + KHostResIgnoreCache );
            TBTDevAddr btaddr = iDevices[iPagingNamePos]->iAddr.BTAddr();
            iInquirySockAddr.SetBTAddr( iDevices[iPagingNamePos]->iAddr.BTAddr() );
            iInquirySockAddr.SetIAC(KGIAC);
            iHostResolver.GetByAddress( iInquirySockAddr, iEntry, iActive->RequestStatus() );
            iActive->GoActive();
            break;
            }
        }
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::HandleInquiryResultL
// Inform of properties of the found BTdevice,   
// which passes the search filter. Its name would be retrived 
// later if not contained by the first round of inquiry.   
// ----------------------------------------------------------
//
TInt CBasicDevDiscoverer::HandleInquiryResultL()
    {
    TInquirySockAddr& sa = TInquirySockAddr::Cast( iEntry().iAddr );
    // parse the inquiry result if this device passes the filters:
    if ( iMajorDeviceClassFilter == EMajorDeviceMisc 
         || sa.MajorClassOfDevice() == iMajorDeviceClassFilter )
        {
        CDeviceSearchRecord* record = NewInstanceL( sa );
        CleanupStack::PushL( record );
        iDevices.InsertL(record, 0 );
        CleanupStack::Pop( record );
        
        TBTDeviceName devName;
        TBluetoothNameRecordWrapper eir( iEntry() );
        TInt length = eir.GetDeviceNameLength();
        TBool isComplete( EFalse );
        if( length > 0 )
            {
            User::LeaveIfError( eir.GetDeviceName( record->iName, isComplete) );
            }
        return 0;
        }
    return KErrNotFound;
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::CompleteDiscovery
// ----------------------------------------------------------
//
CDeviceSearchRecord* CBasicDevDiscoverer::NewInstanceL( 
        const TInquirySockAddr& aAddr, const TDesC& aName )
    {
    CDeviceSearchRecord* record = new (ELeave) CDeviceSearchRecord();
    record->iAddr = aAddr;
    record->iName = aName;
    return record;
    }

// ----------------------------------------------------------
// CBasicDevDiscoverer::Reset
// ----------------------------------------------------------
//
void CBasicDevDiscoverer::Reset()
    {
    // Free the cache of host Resolver.
    iHostResolver.Close();
    // Clean previous in-range devices whose proximity status
    // may have been changed.
    iDevices.ResetAndDestroy();
    }

// End of File
