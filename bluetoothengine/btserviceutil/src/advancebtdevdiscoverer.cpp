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
* Description:  CAdvanceDevDiscoverer class: the interface class for
*               searching in-range Bluetooth devices using Host Resolver.
*
*/

#include <btservices/advancedevdiscoverer.h>
#include "advancedevdiscovererimpl.h"

// ----------------------------------------------------------
// CAdvanceDevDiscoverer::CAdvanceDevDiscoverer
// ----------------------------------------------------------
//
CAdvanceDevDiscoverer::CAdvanceDevDiscoverer()
    {
    }

// ----------------------------------------------------------
// CAdvanceDevDiscoverer::CAdvanceDevDiscoverer
// ----------------------------------------------------------
//
void CAdvanceDevDiscoverer::ConstructL( 
        CBtDevRepository& aDevRep,
        MDevDiscoveryObserver& aObserver )
    {
    iImpl = CAdvanceDevDiscovererImpl::NewL( aDevRep, aObserver );
    }

// ----------------------------------------------------------
// CAdvanceDevDiscoverer::NewL
// ----------------------------------------------------------
//
EXPORT_C CAdvanceDevDiscoverer* CAdvanceDevDiscoverer::NewL(
        CBtDevRepository& aDevRep,
        MDevDiscoveryObserver& aObserver )
    {
    CAdvanceDevDiscoverer* self = new (ELeave) CAdvanceDevDiscoverer();
    CleanupStack::PushL( self );
    self->ConstructL( aDevRep, aObserver );
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
EXPORT_C CAdvanceDevDiscoverer::~CAdvanceDevDiscoverer()
    {
    delete iImpl;
    }

// ----------------------------------------------------------
// CAdvanceDevDiscoverer::SetObserver
// ----------------------------------------------------------
//
EXPORT_C void CAdvanceDevDiscoverer::SetObserver(
        MDevDiscoveryObserver& aObserver )
    {
    iImpl->SetObserver( aObserver );
    }

// ----------------------------------------------------------
// CAdvanceDevDiscoverer::DiscoverDeviceL
// ----------------------------------------------------------
//
EXPORT_C void CAdvanceDevDiscoverer::DiscoverDeviceL(
        TDevDiscoveryFilter aFilter,
        TBTMajorDeviceClass aDeviceClass )
    {
    iImpl->DiscoverDeviceL( aFilter, aDeviceClass );
    }

// ----------------------------------------------------------
// CAdvanceDevDiscoverer::CancelDiscovery
// ----------------------------------------------------------
//
EXPORT_C void CAdvanceDevDiscoverer::CancelDiscovery()
    {
    iImpl->Cancel();
    }

// End of File
