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
* Description:  Monitors BTRegistry device changes and sends the 
*				 new status to listener
*
*/

#include "btregistryobserver.h"
#include "debug.h"

#include <bt_subscribe.h>
const TInt KDeviceArrayDefaultSize=10;
// ---------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------
CBTRegistryObserver::CBTRegistryObserver(MBTRegistryObserver* aObserver, 
    const TBTRegistrySearch aPattern) 
    	: CActive(CActive::EPriorityHigh), 
      	  iObserver(aObserver),   iSearchPattern(aPattern)
    {    
    TRACE_FUNC_ENTRY
    CActiveScheduler::Add(this);
    // The priority will be EPriorityHigh only initially.
    // after the first successfull retrieval of the devicelist
    // the priority will be changed to EPriorityLow.
    
    // This will make the 1st devicelist drawn as quickly as possible,
    // but the subsequent refrehes will not decrease the responsiveness
    // of the Btui.
    iAnotherEventPending=EFalse;
    
    TRACE_FUNC_EXIT
    }
// ---------------------------------------------------------------------
// CBTRegistryObserver::NewL
// ---------------------------------------------------------------------
CBTRegistryObserver* CBTRegistryObserver::NewL(MBTRegistryObserver* aObserver,
	  const TBTRegistrySearch aPattern)
    {
    CBTRegistryObserver* self = new (ELeave) CBTRegistryObserver(aObserver, aPattern);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
// ---------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------    
CBTRegistryObserver::~CBTRegistryObserver()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iProperty.Close();
    delete iDevMan;
    
    DeleteAllocatedDevices();
    
    delete iRegDeviceArray;
    iRegDeviceArray=NULL;
    
    TRACE_FUNC_EXIT
    }
// ---------------------------------------------------------------------
// CBTRegistryObserver::ConstructL()
// ---------------------------------------------------------------------
void CBTRegistryObserver::ConstructL()
    {
    TRACE_FUNC_ENTRY
	iRegDeviceArray = new (ELeave) CBTDeviceArray(KDeviceArrayDefaultSize);    
    iDevMan =  CBTEngDevMan::NewL(this);
    iDevMan->SetPriority(CActive::EPriorityHigh);
    
    iProperty.Attach(KPropertyUidBluetoothCategory, KPropertyKeyBluetoothGetRegistryTableChange);
    StartIfNotRunning();
    TRACE_FUNC_EXIT
    }
// ---------------------------------------------------------------------
// CBTRegistryObserver::Refresh
// This will simulate registry change event.
// ---------------------------------------------------------------------    
void CBTRegistryObserver::Refresh()
	{
	TRACE_FUNC_ENTRY
	
	__ASSERT_DEBUG(iDevMan, User::Invariant());
	
    if (iDevMan->IsActive())
    	{
    	// An attempt to start a new search, during a prior
    	// one. 
    	iAnotherEventPending=ETrue;
    	return;
    	}   
	Cancel();

    DeleteAllocatedDevices();
    iDevMan->GetDevices(iSearchPattern, iRegDeviceArray);
    
	StartIfNotRunning();    
    
    TRACE_FUNC_EXIT    
	}
	
// ---------------------------------------------------------------------
// CBTRegistryObserver::StartIfNotRunning
// this have to be called again each time there has been registry event
// ---------------------------------------------------------------------
void CBTRegistryObserver::StartIfNotRunning()
    {
    TRACE_FUNC_ENTRY
    if (!IsActive())
        {
        TRACE_INFO(_L("Start()"))
        iIsStopped = EFalse;
        iProperty.Subscribe(iStatus);
        SetActive();
        }
    TRACE_FUNC_EXIT        
    }
// ---------------------------------------------------------------------
// CBTRegistryObserver::RunL
// From CAtive
// This is called when registry has changed. This will not
// be called when BTRegistry date retrieval is complete, since
// HandleGetDevicesComplete is called when RBTregistry data retrieval
// is complete
// ----------------------------------------------------------------------

void CBTRegistryObserver::RunL()
    {
    TRACE_FUNC_ENTRY
    if(iDevMan && iDevMan->IsActive())
    	{
    	// An attempt to start a new search, during a prior
    	// one. We will not start a new search, but we
    	// will do it after the current search is complete
    	iAnotherEventPending=ETrue;
    	StartIfNotRunning();
		TRACE_FUNC_EXIT    	
    	return;
    	}
    if(!IsActive())
    	SetPriority(CActive::EPriorityLow);
    
    

    TInt registryTable;
    iProperty.Get(registryTable);
    iAnotherEventPending=EFalse;
    
    if(iDevMan && iStatus.Int() == KErrNone && registryTable == KRegistryChangeRemoteTable)
        {
        TRACE_INFO(_L("Remote table change"))
        
        DeleteAllocatedDevices();
        iDevMan->GetDevices(iSearchPattern, iRegDeviceArray);
        }
    StartIfNotRunning();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------
// CBTRegistryObserver::HandleGetDevicesComplete
// From MBTEngDevManObserver
//
// This function will not filter out refreshes with no changes,
// since the connect status may have been changed in those cases, and
// this class does not know about that
//
// It is worth to mention that CBTDeviceContainer will not refresh parts
// of the screen that are not changed.
// ---------------------------------------------------------------------
void  CBTRegistryObserver::HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* aDeviceArray)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("aErr = %d"), aErr))
    
    // the initial view refresh is done. We will decrese
    // the priority of this component to quarantee UI
    // responsiveness
    if(!iDevMan->IsActive())
    	iDevMan->SetPriority(CActive::EPriorityLow);
    
    TRAP_IGNORE(
		if( aErr != KErrNotFound)
			iObserver->RegistryChangedL(aDeviceArray );
		else
			{
			// handle the empty response case. The DevMan may issue
			// NULL in that case 
			iObserver->RegistryChangedL(iRegDeviceArray );	
			}
	);	

    DeleteAllocatedDevices();

	if(iAnotherEventPending)	
		{
		// Current results are not sent further, since
		// there has been changes, witch may have rendered the current
		// results allready obsolate. Therefore a refresh is made instead
    
		iAnotherEventPending=EFalse;
        iDevMan->GetDevices(iSearchPattern, iRegDeviceArray);
		}
    TRACE_FUNC_EXIT		
    }

// ---------------------------------------------------------------------
// CBTRegistryObserver::DeleteAllocatedDevices
// ---------------------------------------------------------------------
void CBTRegistryObserver::DeleteAllocatedDevices()
	{
	TRACE_FUNC_ENTRY
	if(iRegDeviceArray)
		{
        iRegDeviceArray->ResetAndDestroy();
        }
    TRACE_FUNC_EXIT		
	}
// ---------------------------------------------------------------------
// CBTRegistryObserver::Cancel
// From CActive
// ---------------------------------------------------------------------
    
void CBTRegistryObserver::Cancel()
    {
    TRACE_FUNC_ENTRY
    iIsStopped = ETrue;
    iAnotherEventPending=EFalse;    
    CActive::Cancel();
    if (iDevMan) iDevMan->Cancel();
    DeleteAllocatedDevices();
    TRACE_FUNC_EXIT    
    }
// ---------------------------------------------------------------------
// CBTRegistryObserver::DoCancel
// From CActive
// ---------------------------------------------------------------------
void CBTRegistryObserver::DoCancel()
	{
	TRACE_FUNC_ENTRY
    iProperty.Cancel();	
    TRACE_FUNC_EXIT
	}
