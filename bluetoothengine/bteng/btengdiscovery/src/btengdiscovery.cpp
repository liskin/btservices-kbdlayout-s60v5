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
* Description:  Bluetooth Engine API for device discovery functionality.
*
*/



#include <featmgr.h>

#include "btengdiscovery.h"
#include "btengsdpdbhandler.h"
#include "btengsdpquery.h"
#include "btengdevicesearch.h"
#include "debug.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngDiscovery::CBTEngDiscovery( MBTEngSdpResultReceiver* aNotifier )
:   iResultNotifier( aNotifier )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngDiscovery::ConstructL()
    {
    TRACE_FUNC_ENTRY
    // Check if BT is supported at all
    FeatureManager::InitializeLibL();
    TBool btSupported = FeatureManager::FeatureSupported( KFeatureIdBt );
    FeatureManager::UnInitializeLib();
    if( !btSupported )
        {
        TRACE_INFO( ( _L( "[BTENGDISC]\t ConstructL: BT not supported" ) ) )
        User::Leave( KErrNotSupported );
        }

    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CBTEngDiscovery* CBTEngDiscovery::NewL( MBTEngSdpResultReceiver* aNotifier )
    {
    CBTEngDiscovery* self = CBTEngDiscovery::NewLC( aNotifier );
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
EXPORT_C CBTEngDiscovery* CBTEngDiscovery::NewLC( MBTEngSdpResultReceiver* aNotifier )
    {
    CBTEngDiscovery* self = new( ELeave ) CBTEngDiscovery( aNotifier );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngDiscovery::~CBTEngDiscovery()
    {
    TRACE_FUNC_ENTRY
    delete iSdpDbHandler;
    delete iSdpQueryHandler;
    delete iDevSearchHandler;
    }


// ---------------------------------------------------------------------------
// Searches for remote BT devices, by launching the device discovery notifier.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::SearchRemoteDevice( CBTDevice* aDevice, 
    TUint aServiceClass )
    {
    TRACE_FUNC_ENTRY
    TInt err = SearchRemoteDevice(aDevice, NULL, aServiceClass);
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// device search version supporting EIR data retrieval
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::SearchRemoteDevice( CBTDevice* aDevice,
        TNameEntry* aNameEntry, TUint aServiceClass )
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    if( !iDevSearchHandler )
        {
        TRAP( err, iDevSearchHandler = CBTEngDeviceSearch::NewL() );
        }
    if( !err )
        {
        err = iDevSearchHandler->StartSearch( aDevice, aNameEntry, aServiceClass, 
                                               iResultNotifier );
        }
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// Cancels an ongoing search for remote BT devices.
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngDiscovery::CancelSearchRemoteDevice()
    {
    TRACE_FUNC_ENTRY
    if( iDevSearchHandler )
        {
        iDevSearchHandler->CancelSearch();
        }
    }

// ---------------------------------------------------------------------------
// Gets the service uuids in the cached EIR data of the device
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::GetEirServiceUUIDs( const TBTDevAddr& aAddr, 
        TNameEntry* aNameEntry)
    {
    TRACE_FUNC_ENTRY
    __ASSERT_DEBUG( aNameEntry != NULL, User::Panic(_L("null pointer in GetEirServiceUUIDs"),KErrArgument));
    TInt err = KErrNone;
    if( !iDevSearchHandler )
        {
        TRAP( err, iDevSearchHandler = CBTEngDeviceSearch::NewL() );
        }
    if( !err )
        {
        err = iDevSearchHandler->GetEirServiceUUIDs(aAddr, aNameEntry, 
              iResultNotifier );
        }
    TRACE_FUNC_EXIT
    return err;    
    }

// ---------------------------------------------------------------------------
// cancels ongoing EIR service list retrieval.
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngDiscovery::CancelGetEirServiceUUIDs()
    {
    if( iDevSearchHandler )
        {
        iDevSearchHandler->CancelGetEirServiceUUIDs();
        }    
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::RegisterSdpRecord( const TUUID& aService, 
    const TUint aChannel, TSdpServRecordHandle& aHandle )
    {
    TRACE_FUNC_ENTRY
    TInt err = CheckSdpDbHandler();
    if( !err )
        {
        TRAP( err, iSdpDbHandler->RegisterSdpRecordL( aService, aChannel, aHandle ) );
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::DeleteSdpRecord( const TSdpServRecordHandle aHandle )
    {
    TRACE_FUNC_ENTRY
    TInt err = CheckSdpDbHandler();
    if( !err )
        {
        TRAP( err, iSdpDbHandler->DeleteSdpRecordL( aHandle ) );
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngDiscovery::SetNotifier( MBTEngSdpResultReceiver* aNotifier )
    {
    ASSERT( aNotifier );
    iResultNotifier = aNotifier;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::RemoteSdpQuery( const TBTDevAddr& aAddr, 
    const TUUID& aService )
    {
    TRACE_FUNC_ENTRY
    TInt err = CheckSdpQueryHandler();
    if( !err )
        {
        TRAP( err, iSdpQueryHandler->RemoteSdpQueryL( aAddr, aService ) );
        }
    TRACE_FUNC_EXIT
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::RemoteSdpQuery( const TBTDevAddr& aAddr, 
    const TSdpServRecordHandle aHandle, const TSdpAttributeID aAttrId )
    {
    TRACE_FUNC_ENTRY
    TInt err = CheckSdpQueryHandler();
    if( !err )
        {
        TRAP( err, iSdpQueryHandler->RemoteSdpQueryL( aAddr, aHandle, aAttrId ) );
        }
    TRACE_FUNC_EXIT
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::RemoteSdpQuery( const TBTDevAddr& aAddr, 
    const TUUID& aService, const TSdpAttributeID aAttrId )
    {
    TRACE_FUNC_ENTRY
    TInt err = CheckSdpQueryHandler();
    if( !err )
        {
        TRAP( err, iSdpQueryHandler->RemoteSdpQueryL( aAddr, aService, aAttrId ) );
        }
    TRACE_FUNC_EXIT
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::RemoteProtocolChannelQuery( const TBTDevAddr& aAddr, 
    const TUUID& aService )
    {
    return RemoteSdpQuery( aAddr, aService, KSdpAttrIdProtocolDescriptorList );
    }


// ---------------------------------------------------------------------------
// Cancels an ongoing search for remote SDP services.
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngDiscovery::CancelRemoteSdpQuery()
    {
    TRACE_FUNC_ENTRY
    if( iSdpQueryHandler )
        {
        iSdpQueryHandler->Cancel();
        }
    }
    
// ---------------------------------------------------------------------------
// Closes a remote SDP connection. The method should be called after RemoteSdpQuery
// and RemoteProtocolChannelQuery methods are completed and the CBTEngDiscovery obejct
// is still kept alive.
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngDiscovery::CloseRemoteSdpConnection()
    {
    TRACE_FUNC_ENTRY
    if( iSdpQueryHandler )
        {
        delete iSdpQueryHandler;
        iSdpQueryHandler = NULL;
        }
    }

// ---------------------------------------------------------------------------
// Parses the element type at specified index from the specified result array.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::ParseNextSdpAttrValueType( 
    RSdpResultArray& aResultArray, TInt aIndex, TSdpElementType& aType )
    {
    if( aIndex >= aResultArray.Count() )
        {
            // Check if this points to a valid position, within range [0, count-1].
        return KErrEof;
        }
    aType = aResultArray[ aIndex ].iAttrType;
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Parses the element type at specified index from the specified result array.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDiscovery::ParseRfcommChannel( RSdpResultArray& aResultArray, 
    TInt& aChannel )
    {
    TInt err = KErrNone;
    TInt index = 0;
    TSdpElementType type = ETypeNil;
    while( index < aResultArray.Count() )
        {
        err = ParseNextSdpAttrValueType( aResultArray, index, type );
            // Could be refined to check for the RFCOMM identifier???
        if( !err && type == ETypeUint )
            {
            aChannel = aResultArray[ index ].iAttrValue.iValNumeric;
            break;
            }
        index++;
        }
    return err;
    }
    

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngDiscovery::CheckSdpDbHandler()
    {
    TInt err = KErrNone;
    if( !iSdpDbHandler )
        {
            // Trap is necessary here as this is a non-leaving fuction.
        TRAP( err, iSdpDbHandler = CBTEngSdpDbHandler::NewL() );
        }
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngDiscovery::CheckSdpQueryHandler()
    {
    if( !iResultNotifier )
        {
        return KErrNotReady;
        }
    TInt err = KErrNone;
    if( !iSdpQueryHandler )
        {
            // Trap is necessary here as this is a non-leaving fuction.
        TRAP( err, iSdpQueryHandler = CBTEngSdpQuery::NewL( iResultNotifier ) );
        }
    return err;
    }


// ---------------------------------------------------------------------------
// Default implementation of EIR version DeviceSearchComplete
// ---------------------------------------------------------------------------
//
EXPORT_C void MBTEngSdpResultReceiver::DeviceSearchComplete( CBTDevice* aDevice, 
        TNameEntry* aNameEntry, 
        TInt aErr)
    {
    TRACE_FUNC_ENTRY
    (void) aDevice;
    (void) aNameEntry,
    (void) aErr;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Default implementation of GetEirServiceUUIDsComplete
// ---------------------------------------------------------------------------
//
EXPORT_C void MBTEngSdpResultReceiver::GetEirServiceUUIDsComplete(
        TNameEntry* aNameEntry, 
        TInt aErr )
    {
    TRACE_FUNC_ENTRY
    (void) aNameEntry,
    (void) aErr;    
    TRACE_FUNC_EXIT
    }

