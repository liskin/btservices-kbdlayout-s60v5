/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Maintain a Bluetooth devices data model for Blocked devices view.
*
*/


// INCLUDE FILES
#include "btblockedmodel.h"
#include "debug.h"

// ---------------------------------------------------------------------
// constructor
// ----------------------------------------------------------------------        
CBTBlockedModel::CBTBlockedModel(MBTDeviceObserver* aObserver,TBTDeviceSortOrder* aOrder )
    : CBTDevModelBase(aObserver, aOrder)
    {
    TRACE_FUNC
    }
// ---------------------------------------------------------------------
// CBTBlockedModel::NewL
// ----------------------------------------------------------------------        
CBTBlockedModel* CBTBlockedModel::NewL(MBTDeviceObserver* aObserver, TBTDeviceSortOrder* aOrder )
    {
    CBTBlockedModel* self = new (ELeave) CBTBlockedModel(aObserver, aOrder);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;    
    }
// ---------------------------------------------------------------------
// destructor
// ----------------------------------------------------------------------        
CBTBlockedModel::~CBTBlockedModel() 
    {    
    TRACE_FUNC
    }
// ---------------------------------------------------------------------
// CBTBlockedModel::ConstructL
// ----------------------------------------------------------------------        
void CBTBlockedModel::ConstructL()
    {
    TRACE_FUNC_ENTRY
    // get all devices from btregistry to iDeviceArray - a TBTDevice array 
    iDevMan = CBTEngDevMan::NewL(this);
    iSearchPattern.FindAll();
   	iRegistryObserver = CBTRegistryObserver::NewL(this, iSearchPattern);
	iRegistryObserver->Refresh();
    TRACE_FUNC_EXIT
    }
// ---------------------------------------------------------------------
// CBTBlockedModel::HandleNewDeviceL
// from CBTDevModel
// ----------------------------------------------------------------------    
void CBTBlockedModel::HandleNewDeviceL(const CBTDevice* aRegDevice,
        TNameEntry* aNameEntry)
    {
    TRACE_FUNC_ENTRY
    TBTDeviceSecurity security = aRegDevice->GlobalSecurity();
    if (security.Banned())
        {
        AddDeviceL(aRegDevice, aNameEntry, EOpBlock);
        }
    TRACE_FUNC_EXIT        
    }

