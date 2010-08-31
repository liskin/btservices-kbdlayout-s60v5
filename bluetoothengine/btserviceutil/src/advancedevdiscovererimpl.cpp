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
#include "advancedevdiscovererimpl.h"
#include "basicdevdiscoverer.h"


// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::CAdvanceDevDiscovererImpl
// ----------------------------------------------------------
//
CAdvanceDevDiscovererImpl::CAdvanceDevDiscovererImpl(
        CBtDevRepository& aDevRep,
        MDevDiscoveryObserver& aObserver )
    :iDevRep( aDevRep ), iObserver( aObserver )
    {
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::ConstructL
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::ConstructL()
    {
    iBasicDiscoverer = CBasicDevDiscoverer::NewL( *this );   
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::NewL
// ----------------------------------------------------------
//
CAdvanceDevDiscovererImpl* CAdvanceDevDiscovererImpl::NewL(
        CBtDevRepository& aDevRep,
        MDevDiscoveryObserver& aObserver )
    {
    CAdvanceDevDiscovererImpl* self = 
            new (ELeave) CAdvanceDevDiscovererImpl( aDevRep, aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::
// ----------------------------------------------------------
//
CAdvanceDevDiscovererImpl::~CAdvanceDevDiscovererImpl()
    {
    delete iBasicDiscoverer;
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::SetObserver
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::SetObserver( MDevDiscoveryObserver& aObserver )
    {
    iObserver = aObserver;
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::Cancel()
    {
    iBasicDiscoverer->Cancel();
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::DiscoverDeviceL(
        CAdvanceDevDiscoverer::TDevDiscoveryFilter aFilter, 
        TBTMajorDeviceClass aDeviceClass )
    {
    iBasicDiscoverer->DiscoverDeviceL( aDeviceClass );
    iFilter = aFilter;
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::DiscoverDeviceL
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::DiscoverDeviceL( 
    const RBTDevAddrArray& aPriorityList )
    {
    // to be implemented when BTUI requires this feature.
    ( void ) aPriorityList;
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::HandleNextDiscoveryResult
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::HandleNextDiscoveryResultL( 
        const TInquirySockAddr& aAddr, const TDesC& aName )
    {
    TBool filtered = EFalse;
    if ( iFilter )
        {
        // For filtering, we need to examine the properties of
        // this device from repository.        
        const CBtDevExtension* devExt = iDevRep.Device( aAddr.BTAddr() );
        // No filter is needed if this device is not in registry.
        if ( devExt )
            {
            if ( iFilter & CAdvanceDevDiscoverer::ExcludeUserAwareBonded)
                {
                // client does not want to discover devices that have been
                // bonded already.
                if ( devExt->IsUserAwareBonded() )
                    {
                    filtered = ETrue;
                    }
                }
            }
        }
    if ( !filtered )
        {
        iObserver.HandleNextDiscoveryResultL( aAddr, aName );
        }
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::HandleDiscoveryCompleted
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::HandleDiscoveryCompleted( TInt aErr )
    {
    iObserver.HandleDiscoveryCompleted( aErr );
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::RequestCompletedL
// Inform caller for received device and issue next EIR/Name request
// if the request was successful.
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus )
    {
    (void) aActive;
    (void) aStatus;
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::CancelRequest
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::CancelRequest( TInt aId )
    {
    (void) aId;
    }

// ----------------------------------------------------------
// CAdvanceDevDiscovererImpl::HandleError
// ----------------------------------------------------------
//
void CAdvanceDevDiscovererImpl::HandleError( CBtSimpleActive* aActive, TInt aError )
    {
    (void) aActive;
    (void) aError;
    }

// End of File
