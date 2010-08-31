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
* Description:  Repository of remote Bluetooth devices
*
*/

#include <btservices/btdevrepository.h>
#include "btdevrepositoryimpl.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBtDevRepository::CBtDevRepository()
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBtDevRepository::ConstructL()
    {
    iImpl = CBtDevRepositoryImpl::NewL();
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CBtDevRepository* CBtDevRepository::NewL()
    {
    CBtDevRepository* self = NULL;
    self = new (ELeave) CBtDevRepository();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
EXPORT_C CBtDevRepository::~CBtDevRepository()
    {
    delete iImpl;
    }

// ---------------------------------------------------------------------------
// AddObserverL.
// Delegate to the implementor
// ---------------------------------------------------------------------------
//
EXPORT_C void CBtDevRepository::AddObserverL( MBtDevRepositoryObserver* aObserver )
    {
    iImpl->AddObserverL( aObserver );
    }

// ---------------------------------------------------------------------------
// RemoveObserver.
// Delegate to the implementor
// ---------------------------------------------------------------------------
//
EXPORT_C void CBtDevRepository::RemoveObserver( MBtDevRepositoryObserver* aObserver )
    {
    iImpl->RemoveObserver( aObserver );
    }

// ---------------------------------------------------------------------------
// AllDevices
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CBtDevRepository::IsInitialized() const
    {
    return iImpl->IsInitialized();
    }

// ---------------------------------------------------------------------------
// AllDevices
// ---------------------------------------------------------------------------
//
EXPORT_C const RDevExtensionArray& CBtDevRepository::AllDevices() const
    {
    return iImpl->AllDevices();
    }

// ---------------------------------------------------------------------------
// Device
// ---------------------------------------------------------------------------
//
EXPORT_C const CBtDevExtension* CBtDevRepository::Device( 
        const TBTDevAddr& aAddr ) const
    {
    return iImpl->Device( aAddr );
    }

// ---------------------------------------------------------------------------
// Device
// ---------------------------------------------------------------------------
//
EXPORT_C void CBtDevRepository::ReInitialize()
    {
    return iImpl->ReInitialize();
    }
