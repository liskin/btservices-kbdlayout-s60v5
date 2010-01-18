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
* Description:  Maintain a Bluetooth devices data model for paired devices view
*
*/


#include "btpairedmodel.h"
#include <btengutil.h>
#include "debug.h"
#include "btui.h"

const TInt KMaxCheckedConnections=20;


// ---------------------------------------------------------------------
// CBTPairedModel::CBTPairedModel
// ---------------------------------------------------------------------
//
CBTPairedModel::CBTPairedModel(MBTDeviceObserver* aObserver, TBTDeviceSortOrder* aOrder)
	: CBTDevModelBase(aObserver, aOrder)
    {
	TRACE_FUNC_ENTRY    
    TRACE_FUNC_EXIT	
    }

// ---------------------------------------------------------------------
// CBTPairedModel::NewL
// ---------------------------------------------------------------------
//
CBTPairedModel* CBTPairedModel::NewL(MBTDeviceObserver* aObserver, TBTDeviceSortOrder* aOrder )
    {
    CBTPairedModel* self = new (ELeave) CBTPairedModel(aObserver, aOrder);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
     
    return self;       
    }
    
// --------------------------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------------------------
//
CBTPairedModel::~CBTPairedModel()
    {
	TRACE_FUNC_ENTRY    
	
    delete iConnMan;
    delete iPairingDevice;
    delete iLinks;
    delete iBtEngSettings;
    iSocketServ.Close();    
    
    TRACE_FUNC_EXIT    
    }

// ---------------------------------------------------------------------
// CBTPairedModel::ConstructL
// ----------------------------------------------------------------------
//
void CBTPairedModel::ConstructL()
    {
	TRACE_FUNC_ENTRY
	
    // get all devices from btregistry to cache - local copy of CBTDeviceArray
    iDevMan = CBTEngDevMan::NewL(this);
    iConnMan = CBTEngConnMan::NewL(this);
    iSearchPattern.FindAll(); 
   	iRegistryObserver = CBTRegistryObserver::NewL(this, iSearchPattern);
	iRegistryObserver->Refresh();
	User::LeaveIfError( iSocketServ.Connect() );
	iLinks=CBluetoothPhysicalLinks::NewL(*this,iSocketServ);
	
	iBtEngSettings = CBTEngSettings::NewL(this);
	
    TRACE_FUNC_EXIT	
    }
    
// ---------------------------------------------------------------------
// CBTPairedModel::IsAnyDeviceConnected
// ----------------------------------------------------------------------
//
TBool CBTPairedModel::IsAnyDeviceConnected()
    {        
	TRACE_FUNC_ENTRY

	FlagPhysicallyConnectedDevices(iDeviceArray);
    
	for(TInt i = 0; i < iDeviceArray.Count(); i++)
		{
		if(iDeviceArray[i]->iStatus & (EStatusPhysicallyConnected|EStatusBtuiConnected))
			{
			TRACE_FUNC_EXIT				
			return ETrue;
			}        
		}
					
	TRACE_FUNC_EXIT				
	
    return EFalse;
    }
    
// ---------------------------------------------------------------------
// CBTPairedModel::DoChangeDevice
// ---------------------------------------------------------------------

TInt CBTPairedModel::DoChangeDeviceL(const TBTDevice& aDevice)
    {
    TRACE_FUNC_ENTRY
    TRACE_BDADDR(aDevice.iAddr)
   	// check that the device actually still exists
   	if(aDevice.iOperation != EOpPair )
   		{
   		User::LeaveIfError( GetIndexByAddress(aDevice.iAddr, aDevice.iIndex ) );	
   		}
   	
    // disconnect device to be unpaired, or blocked
    if((aDevice.iOperation == EOpBlock || aDevice.iOperation == EOpUnpair ))
    	{    	
    	if( iState != EDisconnectBeforeOperationState)
    		{
    			// disconnect by CBTEngConnMan if connected that way.
    			if (aDevice.iStatus & static_cast<TInt>(EStatusBtuiConnected ) )
    				{
    				TInt code=KErrNone;
		    		TRAPD(err,
		    		iState=EDisconnectBeforeOperationState;
			    		iDisconnectDevice=new(ELeave) TBTDevice(aDevice);
			    		code=iConnMan->Disconnect(aDevice.iAddr,EBTDiscImmediate);    				
			    		);
					// if the return code is KErrNone then there will be
					// a callback. However if it fails, there will not be any.
					// So the execution will go on inside this method, if there was an error.
					if(err == KErrNone && code==KErrNone )
						{
						return KErrNone;
						}
						
    				}
    			else // try disconnecting from the link layer
    				{
    				TInt code=KErrNone;
    				TRAPD(err,
					iState=EDisconnectBeforeOperationState;
						iDisconnectDevice=new(ELeave) TBTDevice(aDevice);
						code=iLinks->Disconnect(aDevice.iAddr);
						);
					// if the return code is KErrNone then there will be
					// a callback. However if it fails, there will not be any.
					// So the execution will go on inside this method, if there was an error.
					if(err == KErrNone && code==KErrNone )
						{
						return KErrNone;
						}
    				}
    		}
   		else    
			iState=EIdleState;
    
    	}
    	
    if(	aDevice.iOperation ==EOpPair)
    	{
    	DoPairDeviceL();
    	TRACE_FUNC_EXIT 
    	return KErrNone;    	
    	}
    
    TInt err = CBTDevModelBase::DoChangeDeviceL(aDevice);
    if(iDisconnectDevice!= NULL)
    	{
    	delete iDisconnectDevice;
    	iDisconnectDevice=NULL;
    	}
    
    if (err != KErrNone)
        {      
        TBTDevAddr addr;
        if(aDevice.iAddr == KNullAddress )
        	{        	        	
        	if(aDevice.iIndex < 0 || aDevice.iIndex >= iDeviceArray.Count())
        		return KErrArgument;
        	else
        		addr=iDeviceArray[aDevice.iIndex]->iAddr;
        	}
        	else
        		addr=aDevice.iAddr;
        	
        switch (aDevice.iOperation)
            {
            case EOpConnect:
            	{                    	   		            		               
                err = iConnMan->Connect(addr, aDevice.iDeviceClass);
                break;
            	}
            case EOpDisconnect:
            	{
            	iDisconnectDevice=new(ELeave) TBTDevice(aDevice);            		            		           	
				if (aDevice.iStatus & static_cast<TInt>(EStatusBtuiConnected ) )            	
					{
					err = iConnMan->Disconnect(addr, EBTDiscImmediate);	
					}
				else
					{
					err=iLinks->Disconnect(aDevice.iAddr);
					}
                
                break;
            	}
            default:
            	User::Leave(err);
                break;
            }
        }
        
    TRACE_FUNC_EXIT        
    
    return err;
    }
    
// ---------------------------------------------------------------------
// CBTPairedModel::DoCancelChange
// ---------------------------------------------------------------------
//    
void CBTPairedModel::DoCancelChangeL(const TBTDevice& aDevice)
    {
    TRACE_FUNC_ENTRY
    
	if(aDevice.iIndex >= iDeviceArray.Count())
        {
        TRACE_INFO((_L("index is %d, max expected is %d"), aDevice.iIndex, iDeviceArray.Count()));
        TRACE_FUNC_EXIT    
		return; 
        }
    
    // in case it was stopped
    iRegistryObserver->StartIfNotRunning();
    switch (aDevice.iOperation)
        {
        case EOpPair:
        	if( iState==EDeleteDeviceState || iState==EAddDeviceState )
        		{
        		iDevMan->Cancel();
        		}        		
        	else
        		{
        		iConnMan->CancelPairDevice();
        		}            	        	
            delete iPairingDevice;
            iPairingDevice=NULL;
            iState=EIdleState;
            iRegistryObserver->Refresh();
            break;
            
        case EOpConnect:
        	{	
        	
            TBTDevAddr addr;
            if(aDevice.iIndex == KErrNotFound) 
            	{
            	addr=aDevice.iAddr;
            	}                
            else
            	{
            	addr=iDeviceArray[aDevice.iIndex]->iAddr;
            	}
            	
            iConnMan->CancelConnect(addr);
            break;
        	}
        default:
            CBTDevModelBase::CancelChange(aDevice);
            return;
        }
    iRegistryObserver->StartIfNotRunning();
    
	if(iQueue.Count()>0)
		{
		delete iQueue[0];
		iQueue.Remove(0);		
		HandleQueue();
		}
	
    TRACE_FUNC_EXIT	       
    }
    
// ---------------------------------------------------------------------
// CBTPairedModel::HandleNewDevice
//
// Adds the device if it is not banned
// ---------------------------------------------------------------------
//
void CBTPairedModel::HandleNewDeviceL(const CBTDevice* aRegDevice,
        TNameEntry* aNameEntry)
    {
    TRACE_FUNC_ENTRY
    
    // It is needed to filter out blocked devices,
    // since otherwise blocked and paired devices would
    // be listed.
    if( ! aRegDevice->GlobalSecurity().Banned()     	
            && IsUserAwarePaired( aRegDevice->AsNamelessDevice() ) ) 
        {
        AddDeviceL(aRegDevice, aNameEntry, EOpNone);
        }

    TRACE_FUNC_EXIT		    
    }
    
// -----------------------------------------------------------------------
// CBTPairedModel::CreateDevice
//
// This implementaion will add the connectable status from the superclass.
// -----------------------------------------------------------------------  
//
TBTDevice* CBTPairedModel::CreateDeviceL(const CBTDevice* aRegDevice,
        TNameEntry* aNameEntry)
    {
    TRACE_FUNC_ENTRY
    
    TBTDevice* device = CBTDevModelBase::CreateDeviceL(aRegDevice, aNameEntry);
    
    // add EStatusBtuiConnected status if the device has a profile connection
    TBTEngConnectionStatus connStatus;
    iConnMan->IsConnected(device->iAddr, connStatus);
    
    if(connStatus == EBTEngConnecting || connStatus == EBTEngConnected)
    	{
    	SetStatusFlags(device->iStatus,EStatusBtuiConnected);
    	}
    
    // add connectable status
    TBool connectable=EFalse;
    iConnMan->IsConnectable(aRegDevice->BDAddr(), aRegDevice->DeviceClass(), connectable );
    
    if( connectable) 
    	{
    	SetStatusFlags(device->iStatus, EStatusConnectable );
    	}
    	
	TRACE_FUNC_EXIT 
	       
    return device;
    }
    
// ---------------------------------------------------------------------
// CBTPairedModel::ConnectComplete
// from MBTEngConnObserver
// ----------------------------------------------------------------------
//
void CBTPairedModel::ConnectComplete(TBTDevAddr& aAddr, TInt aErr, RBTDevAddrArray* aConflicts)
    {
    TRACE_FUNC_ENTRY
    
    TRACE_INFO((_L("aErr=%d"), aErr))

    TInt index = GetIndexByAddress(aAddr);
    TBool requestIssuedFromPhone = EFalse;
    TBTDevice connectedDevice;
    
    // was the device connected from request by the ui
	if(iQueue.Count()>0 && iQueue[0]->iOperation==EOpConnect && 
	  iQueue[0]->iAddr == aAddr)
		{
		requestIssuedFromPhone = ETrue;
		if(index >=0) // is it from paired list
			{			
			connectedDevice=*iDeviceArray[index];
			connectedDevice.iOperation=EOpConnect;
			}
		else
			{
			connectedDevice=*iQueue[0];		
			}
    	
    	}
    else
    	{
    	requestIssuedFromPhone = EFalse;
		if(index >=0) // is it from paired list
			{
			connectedDevice=*iDeviceArray[index];
			connectedDevice.iOperation=EOpConnect;
			}
		else
			{
	    	// it was not. Try to fill what can be filled.
			connectedDevice.iAddr=aAddr;
			connectedDevice.iIndex=KErrNotFound;
			connectedDevice.iOperation = EOpConnect;			
			}    	
    	}        
    
    if (aErr == KErrAlreadyExists )
        { 
        // gather the names of the conflicting devices.
        RBTDevNameArray nameArray;
        for (TInt i = 0; aConflicts!= NULL && i < aConflicts->Count(); i++)
            {
            //remove duplicates
            if(i>0&& (*aConflicts)[i]==(*aConflicts)[i-1])
            	{
            	continue;
            	}
            TInt di = GetIndexByAddress((*aConflicts)[i]);
            
            if(di >=0 )
            	{
            	nameArray.Append(&iDeviceArray[di]->iName);
            	}            	
            }
        if(iObserver )
        	{
    		iObserver->NotifyChangeDeviceComplete(aErr, connectedDevice, &nameArray);        	
        	}            
        nameArray.Reset();
        }
    else 
        {
        if (aErr == KErrNone)
            {
            // we will set the connect status of the device
            // if it is found in paired devices.
            if( index >= 0 )
            	{
            	SetStatusFlags(iDeviceArray[index]->iStatus,EStatusBtuiConnected );
            	}
            }
        else        
            {
            // we will unset the connect status of the device if connection failed &
            // it is found in paired devices.
            if( index >= 0 )
                {                   
                // add EStatusBtuiConnected status if the device has a profile connection
                TBTEngConnectionStatus connStatus;
                iConnMan->IsConnected(connectedDevice.iAddr, connStatus);
   
                if (connStatus != EBTEngConnecting && connStatus != EBTEngConnected)
                   {
                   TRACE_INFO(_L("Unset EStatusBtuiConnected"))
                   UnsetStatusFlags(iDeviceArray[index]->iStatus,EStatusBtuiConnected );
                   }                
                }
            }
		if(iObserver)
			{
			//Show only phone issued request complete notes
			//Do not show notes for device issued request when BTUI is active
			if ( requestIssuedFromPhone ) 
			    {
        	    iObserver->NotifyChangeDeviceComplete(aErr, connectedDevice);
			    }
        	
        	SendRefreshIfNoError(aErr);
			}		
        }           
    
    // Process the next command if the connect was
    //initiated by this UI.    
	if(iQueue.Count()>0 && iQueue[0]->iOperation==EOpConnect && 
       iQueue[0]->iAddr == aAddr)        
        {
        delete iQueue[0];
        iQueue.Remove(0);		
        HandleQueue();
        }
    else
        {
        if( iQueue.Count()==0 ) 
            {
            TRACE_INFO((_L("op Q is empty")));
            }
        else
            {
            TRACE_INFO((_L("unexpected op Q contents, count is %d, op is %d"), iDeviceArray.Count(), iQueue[0]->iOperation));
            }
        }
    TRACE_FUNC_EXIT	
    }
    
// ---------------------------------------------------------------------
// CBTPairedModel::PowerStateChanged
// from MBTEngSettingObserver
// ----------------------------------------------------------------------
//
void CBTPairedModel::PowerStateChanged( TBTPowerStateValue )
	{
	TRACE_FUNC_ENTRY
	
	// This is implemented for case like when user choses offlne mode, Bluetooth is swithced off
	// very fast. Bt audio components don't have enough time to close audio connection.
	// BTUI needs to update UI display.
	
	TInt count = iDeviceArray.Count();  
    
    for (TInt i = 0; i < count; i++)
    	{
     	UnsetStatusFlags(iDeviceArray[i]->iStatus, EStatusBtuiConnected );
    	}
	
	SendRefreshIfNoError(KErrNone);
	
	TRACE_FUNC_EXIT
	}
    
// ----------------------------------------------------
// CBTPairedModel::VisibilityModeChanged
// from MBTEngSettingObserver
// ----------------------------------------------------
//    
void CBTPairedModel::VisibilityModeChanged( TBTVisibilityMode  )
    {
	TRACE_FUNC_ENTRY
	
	
	TRACE_FUNC_EXIT	
    }
        
// ---------------------------------------------------------------------
// CBTPairedModel::DisconnectComplete
// from MBTEngConnObserver
// ----------------------------------------------------------------------
//
void CBTPairedModel::DisconnectComplete(TBTDevAddr& aAddr, TInt aErr)
    {
    TRACE_FUNC_ENTRY
    
    // the disconnect was part of a unpairing or blocking operation
	if(iState == EDisconnectBeforeOperationState && iDisconnectDevice && aAddr == iDisconnectDevice->iAddr )
		{

		TRAPD(err,
			DoChangeDeviceL(*iDisconnectDevice);		
		);
		iState=EIdleState;		
		if(err != KErrNone)
			{
			HandleLeave(err,iDisconnectDevice);
			}
		TRACE_FUNC_EXIT
		return;
		}		    
    
    TInt index = GetIndexByAddress(aAddr);
    
    // the disconnected device was not the devicein the list,
    // so we do not tell anyone about it.
    if ( iDisconnectDevice == NULL || iDisconnectDevice->iAddr != aAddr )
    	{
    	if (index >=0 )
    	{
    		UnsetStatusFlags(iDeviceArray[index]->iStatus, EStatusBtuiConnected );
    	}
    	
    	SendRefreshIfNoError(KErrNone);

		TBTDevice disconnectedDevice;
		disconnectedDevice.iAddr = aAddr;
		// Notify needed when there is active disconnect query dialog
    	iObserver->NotifyChangeDeviceComplete(KErrDisconnected, disconnectedDevice);
		
    	TRACE_FUNC_EXIT  
    	return;
    	}
    	
	if(index >=0 )
		{
	    // set it to "not connected" no matter disconnection works or not
	    UnsetStatusFlags(iDeviceArray[index]->iStatus, EStatusBtuiConnected );
	    iDeviceArray[index]->iOperation = EOpDisconnect;
	    iObserver->NotifyChangeDeviceComplete(aErr, *iDeviceArray[index]);		
		}
	else
		{
		iDisconnectDevice->iIndex=index;
		iObserver->NotifyChangeDeviceComplete(aErr, *iDisconnectDevice);		
		}

	delete iDisconnectDevice;
	iDisconnectDevice=NULL;    	

   	SendRefreshIfNoError(aErr);
   	// continue processing queue only if the disconnected device is
   	// the device with we requested disconnect to.
   	// this has to be checked so that we do not end on having multiple
   	// queue processors at the same time
	if(iQueue.Count()>0 && iQueue[0]->iOperation==EOpDisconnect && 
	  iQueue[0]->iAddr == aAddr)           	
		{
		delete iQueue[0];
		iQueue.Remove(0);		
		HandleQueue();
		}
    else
        {
        if( iQueue.Count()==0 ) 
            {
            TRACE_INFO((_L("op Q is empty")));
            }
        else
            {
            TRACE_INFO((_L("unexpected op Q contents, count is %d, op is %d"), iDeviceArray.Count(), iQueue[0]->iOperation));
            }
        }
    TRACE_FUNC_EXIT   	
    }
    
// ---------------------------------------------------------------------
// CBTPairedModel::PairingComplete
// from MBTEngConnObserver
// ----------------------------------------------------------------------
//
void CBTPairedModel::PairingComplete(TBTDevAddr& aAddr, TInt aErr)
	{
	TRACE_FUNC_ENTRY
	
	TRAPD(err,
		PairingCompleteL(aAddr,aErr);
		);
		
	if(err!=KErrNone)
	{
		TBTDevice tmp;
		tmp.iAddr=aAddr;					
		tmp.iIndex=GetIndexByAddress(aAddr);
		tmp.iOperation=EOpPair;
		HandleLeave(err,&tmp );

		delete iPairingDevice;
		iPairingDevice=NULL;				
	}
	
	TRACE_FUNC_EXIT
	}
	
// ---------------------------------------------------------------------
// CBTPairedModel::PairingCompleteL
// Leaving version of PairingComplete
// ----------------------------------------------------------------------
//	
void CBTPairedModel::PairingCompleteL(TBTDevAddr& aAddr, TInt aErr)
    {
    TRACE_FUNC_ENTRY
    
	// The device that was attempted to be paired is 
	// different from the the device that the pairing is finnished.
	// This cannot happen, since CBTConnMan will report
	// only pairing started by the listener and the UI
	// does not allow making two pairings at the same time
    __ASSERT_DEBUG(aAddr==iPairingDevice->BDAddr(),
      PANIC(EBTPanicCommandComplettedToDiffrentDeviceThanStarted));
    (void) aAddr;

    // bail out if the operation queue is not what we expect
    if( iQueue.Count() == 0 
        || iQueue[0]->iOperation != EOpPair 
        || iQueue[0]->iAddr != aAddr )        
        {
        if( iQueue.Count()==0 ) 
            {
            TRACE_INFO((_L("op Q is empty")));
            }
        else
            {
            TRACE_INFO((_L("unexpected op Q contents, count is %d, op is %d"), iDeviceArray.Count(), iQueue[0]->iOperation));
            }
        TRACE_FUNC_EXIT
        return;
        }

    iState=EIdleState;		
	TBTDevice* tmpDevice=NULL;
	tmpDevice=CreateDeviceL(iPairingDevice, &( iQueue[0]->iNameEntry ) );
	tmpDevice->iOperation=EOpPair;    		
	
    // new paired device must be inserted to the array, since
    // user may be doing trust/ connect operations before
    // it is refreshed from registry
    TBool deleleteTmpDevice=ETrue;
	
	if( aErr == KErrNone  )    	
		{
		SetStatusFlags(tmpDevice->iStatus,EStatusPaired);
		TInt index;
		index=GetIndexByAddress(iPairingDevice->BDAddr());
		
		if(index == KErrNotFound)
			{
			iDeviceArray.InsertInOrderL(tmpDevice,*iSortOrder );
  			  			
    		RenumberDeviceArray();

    		// iPairingDevice is now owned by iDeviceArray. 
    		// So it must not be deleted by his function
    		deleleteTmpDevice=EFalse;
    		
    		SendRefreshIfNoError(aErr,GetIndexByAddress(iPairingDevice->BDAddr()));
			}					
		}

	
    iObserver->NotifyChangeDeviceComplete(aErr, *tmpDevice);         
    
    /*
     * We issue an background refresh in case 
     * registry is updated from btnotif
     */
	iRegistryObserver->StartIfNotRunning();
	iRegistryObserver->Refresh();
    // delete if the ownership has not changed
    if(deleleteTmpDevice )
        {
    	delete tmpDevice;
    	tmpDevice=NULL;
        }

	delete iPairingDevice;    
    iPairingDevice=NULL;       
	delete iQueue[0];
	iQueue.Remove(0);
    
    HandleQueue();    		    

    TRACE_FUNC_EXIT    
    }
    
// ---------------------------------------------------------------------
// BTPairedModel::HandleDevManComplete
// from MBTEngDevModelBase
// ----------------------------------------------------------------------
//
void  CBTPairedModel::HandleDevManComplete(TInt aErr)
	{
	TRACE_FUNC_ENTRY
	
	// if this is handled we can remove this
	if(iDisconnectDevice)
		{
		delete iDisconnectDevice;
		iDisconnectDevice=NULL;
		}
		
	if(iPairingDevice)
		{
		// add the device just deleted	
		if(iState==EDeleteDeviceState && (aErr==KErrNone || aErr == KErrNotFound))
			{
				iPairingDevice->DeleteLinkKey();
				TBTDeviceSecurity sec=iPairingDevice->GlobalSecurity();
				sec.SetBanned(EFalse);
				sec.SetNoAuthorise(EFalse);
				iDevMan->AddDevice(*iPairingDevice);
				iState=EAddDeviceState;
				TRACE_FUNC_EXIT	
				return;
			}
		// if the device is added allready, or if deletion failed
		// we will go on with pairing the device
		if(iState==EAddDeviceState||(iState==EDeleteDeviceState&& aErr != KErrNone))
			{
			aErr = iConnMan->PairDevice(iPairingDevice->BDAddr(), iPairingDevice->DeviceClass());
			iState=EPairDeviceState;
			// NOTE: now PairingComplete will be called and not this method.
			TRACE_FUNC_EXIT	
			return;
			}
		}
	// these are not handled by superclass		
	if(iQueue.Count()>0 &&
	    ( iQueue[0]->iOperation==EOpConnect || 
	     iQueue[0]->iOperation==EOpDisconnect ) )
		{
		iObserver->NotifyChangeDeviceComplete(aErr, *iQueue[0]);
		}
			
	CBTDevModelBase::HandleDevManComplete(aErr);
	
    TRACE_FUNC_EXIT	
	}
	
// ---------------------------------------------------------------------
// CBTPairedModel::DoPairDevice
// ---------------------------------------------------------------------
//
void  CBTPairedModel::DoPairDeviceL()
	{
	TRACE_FUNC_ENTRY
	
	// we are starting a new command. Checks that no earlier commands 
	// have been executed, but not finished properly.
	__ASSERT_DEBUG(iState==EIdleState,PANIC(EBTPanicDevManQueueIsInIllegalState));
	// check that there is no other pairing going on.
	__ASSERT_DEBUG(iPairingDevice!=NULL,PANIC(EBTPanicTwoPairingAttemptsAtTheSameTime));
	
	iRegistryObserver->Cancel();
	
	// devices to be paired are first deleted
	// then added back. Only after that the paring itself is
	// started
	TBTRegistrySearch dele;
	dele.FindAddress(iPairingDevice->BDAddr());
	TInt err = iDevMan->DeleteDevices(dele);

	iState=EDeleteDeviceState;
   if (err)
       {
       HandleDevManComplete(err);
       }
       
    TRACE_FUNC_EXIT
	}
	
// ---------------------------------------------------------------------
// CBTPairedModel::PairDevice
//
// Puts the given aPairingDevice to iPairingDevice and
// adds and empty device with iOperation=EOpPair as placeholder to iQueue.
// the pairing itself will be handled by DoChangeDevice that
// cals then 
// ---------------------------------------------------------------------
//
TInt CBTPairedModel::PairDeviceL(const CBTDevice* aPairingDevice,
        TNameEntry* aNameEntry)	
	{
	TRACE_FUNC_ENTRY
	
	// cannot put two pairings to queue. 
	// this is not possible since ui does not allow it.
	if(iPairingDevice)
		return KErrInUse;
		
	TBTDevice*  device=new(ELeave) TBTDevice();
	device->iOperation=EOpPair;
	device->iAddr=aPairingDevice->BDAddr();
	if ( aNameEntry )
	    {
	    device->iNameEntry = *aNameEntry;
	    }
	CleanupStack::PushL(device);
	iPairingDevice=aPairingDevice->CopyL();
	// put the device to queue directly
	iQueue.AppendL(device);
	TInt err = KErrNone;
	if(iQueue.Count() ==1 )
		{
		err = DoChangeDeviceL(*iQueue[0]);
		}
	CleanupStack::Pop(device);	
    TRACE_FUNC_EXIT			
	return err;
	
	}
	
// ---------------------------------------------------------------------
// CBTPairedModel::HandleDisconnectCompleteL
// Disconnection by CBluetoothPhysicalLinks, for those
// connections not opened by btui. 
// ---------------------------------------------------------------------
//
void CBTPairedModel::HandleDisconnectCompleteL(TInt aErr)
	{
	 TRACE_FUNC_ENTRY 
	 
	 if(iDisconnectDevice)
	 	{
	 	DisconnectComplete(iDisconnectDevice->iAddr,aErr);
	 	}
	 	
	TRACE_FUNC_EXIT
	}
	
// ---------------------------------------------------------------------
// CBTPairedModel::FlagPhysicallyConnectedDevices
//
// Marks the EStatusPhysicallyConnected attribute to the devices
// that have active connection.
// ---------------------------------------------------------------------
//
void CBTPairedModel::FlagPhysicallyConnectedDevices(RDeviceArray &aDeviceArray)
	{
	TRACE_FUNC_ENTRY 
	
	RBTDevAddrArray a;
	iLinks->Enumerate(a,KMaxCheckedConnections);
	
	TInt i=0; 
	
	for(i=0;i<aDeviceArray.Count();i++)
		{
        FlagPhysicallyConnectedDevices( *aDeviceArray[i], a );
		}
	a.Close();	
	
	TRACE_FUNC_EXIT 	
	}
	
void CBTPairedModel::FlagPhysicallyConnectedDevices(TBTDevice& aDevice, RBTDevAddrArray& aBTDeviceArray)
	{
	TRACE_FUNC_ENTRY 
	
	TInt i2=0;
	
    UnsetStatusFlags( aDevice.iStatus, EStatusPhysicallyConnected );		
    for( i2=0; i2<aBTDeviceArray.Count(); i2++ )
        {
        if(aDevice.iAddr == aBTDeviceArray[i2])
            {
            SetStatusFlags( aDevice.iStatus,EStatusPhysicallyConnected );		
            break;				
            }
        }
	
	TRACE_FUNC_EXIT 	
	}
	
// ---------------------------------------------------------------------
// CBTPairedModel::GetDevice
//
// Adds the EPhysicallyConnected attribute devices that have any active
// bluetooth connection.
// ---------------------------------------------------------------------	
//
TInt CBTPairedModel::GetDevice(TBTDevice& aDevice)
	{	
	TRACE_FUNC_ENTRY 	
	
	TInt rvalue=CBTDevModelBase::GetDevice(aDevice);
	RBTDevAddrArray a;
	iLinks->Enumerate(a,KMaxCheckedConnections);
    FlagPhysicallyConnectedDevices(aDevice, a);
	a.Close();	
	
	TRACE_FUNC_EXIT 
	
	return rvalue;	
	}
	
// ---------------------------------------------------------------------
// CBTPairedModel::HandleLeave
// from CBTDevModelBase
// ---------------------------------------------------------------------
//
void CBTPairedModel::HandleLeave(TInt aErr,const TBTDevice* aDevice )
	{
	TRACE_FUNC_ENTRY
	
	iConnMan->CancelPairDevice();

	delete iPairingDevice;
	iPairingDevice=NULL;

	delete iDisconnectDevice;
	iDisconnectDevice=NULL;
	
	iRegistryObserver->StartIfNotRunning();
	iRegistryObserver->Refresh();
	
	iState=EIdleState;
	
	CBTDevModelBase::HandleLeave(aErr,aDevice );
	
	TRACE_FUNC_EXIT
	}
