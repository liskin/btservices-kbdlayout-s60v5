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
* Description:  Maintain a Bluetooth devices data model for UI components.
*
*/

#include "btdevice.h"
#include "btdevmodelbase.h"
#include <btengutil.h>
#include "debug.h"
#include "btui.h"
#include "btregistryobserver.h"

// --------------------------------------------------------------------------------------------
// 1st phaze constructor
// --------------------------------------------------------------------------------------------
CBTDevModelBase::CBTDevModelBase(MBTDeviceObserver* aObserver, TBTDeviceSortOrder* aOrder)
    : iObserver(aObserver)
    {
	TRACE_FUNC_ENTRY
	iSortOrder=aOrder;
    TRACE_FUNC_EXIT    
	
    }

// --------------------------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------------------------
CBTDevModelBase::~CBTDevModelBase()
    {
    TRACE_FUNC_ENTRY
    delete iRegistryObserver;

    for(TInt i=0;i<iDeviceArray.Count();i++)
    	delete iDeviceArray[i];    
	iDeviceArray.Close();
	delete iDevice;
	iDevice=NULL;

    for(TInt i=0;i<iQueue.Count();i++)
    	delete iQueue[i];    
	iQueue.Close();
	
	if(iDevMan)
		iDevMan->Cancel();
    delete iDevMan;
    
    delete iSortOrder;
    TRACE_FUNC_EXIT        
    }
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::IsAnyDeviceConnected
// Allways false, since base class does not support connection
// --------------------------------------------------------------------------------------------    

TBool CBTDevModelBase::IsAnyDeviceConnected()
    {
    TRACE_FUNC_ENTRY
    return EFalse;
   
    }
    
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::GetDevice
// --------------------------------------------------------------------------------------------    
TInt CBTDevModelBase::GetDevice(TBTDevice& aDevice)	
    {
    TRACE_FUNC_ENTRY
	TBTDeviceOp op=aDevice.iOperation;

    TInt index = GetIndexByAddress(aDevice.iAddr,aDevice.iIndex); 

    if (index < 0 )
        {
        if(aDevice.iAddr == KNullAddress )
        	{
        	return KErrArgument;
        	} 
        else
       		{
			return KErrNotFound ;
       		} 
        }
    if(index >= iDeviceArray.Count())
    	return KErrOverflow;
            
    aDevice=*iDeviceArray[index];
    aDevice.iOperation=op;
    TRACE_FUNC_EXIT        
    return KErrNone;
    }
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::ChangeAllDevices
// --------------------------------------------------------------------------------------------        
void CBTDevModelBase::ChangeAllDevices(const TBTDeviceOp aOperation)
    {
    TRACE_FUNC_ENTRY
    TInt count = iDeviceArray.Count();  
    
    for (TInt i = 0; i < count; i++)
        {        
        //TBTDevice device;
        //device=iDeviceArray[i];
        //device.iIndex = i;
        //device.iOperation = aOperation;
        //ChangeDevice(device);
        iDeviceArray[i]->iIndex = i;
        iDeviceArray[i]->iOperation = aOperation;
        ChangeDevice(*iDeviceArray[i]);        
        }        
    TRACE_FUNC_EXIT    
    }

// --------------------------------------------------------------------------------------------
// CBTDevModelBase::GetIndexByAddress
//
// returns the index of the address, if one is not null address.
// if it is give aNullAdressIndex
// --------------------------------------------------------------------------------------------
TInt CBTDevModelBase::GetIndexByAddress(TBTDevAddr aAddr,TInt aNullAdressIndex)
	{
    TRACE_FUNC_ENTRY	
	
    if(aAddr == KNullAddress )
    	{
		TRACE_FUNC_EXIT        			    	
    	return aNullAdressIndex;
    	}
    	else
    	{
	    TInt count = iDeviceArray.Count();
	    TInt i;
	    for (i = 0; i < count; i++)
	        {
	        if (iDeviceArray[i]->iAddr == aAddr) break;
	        }
		if(i == count)
			{
			TRACE_FUNC_EXIT        		
			return KErrNotFound ;	
			}
	    
		else
			{
			TRACE_FUNC_EXIT        		
			return i;		
			}	    		
    	}	    		    
	}
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::DoChangeDevice
// --------------------------------------------------------------------------------------------            
TInt CBTDevModelBase::DoChangeDeviceL(const TBTDevice& aDevice)
    {
    TRACE_FUNC_ENTRY
    //check that the command in progress has been started       
   	__ASSERT_DEBUG(iDevice==NULL,PANIC(EBTPanicDevManQueueIsCorrupt));

    TInt index = GetIndexByAddress(aDevice.iAddr, aDevice.iIndex ); 
        
    if (index == KErrNotFound)
        {
        TRACE_INFO(_L("index not found"));
	    TRACE_FUNC_EXIT    
        if (aDevice.iAddr == KNullAddress) 
        	{
    		return KErrArgument;
        	}
        else
    		{
    		return KErrNotFound ;
    		} 
        }
    if(index >= iDeviceArray.Count())
    	{
        TRACE_INFO((_L("index is %d, max expected is %d"), index, iDeviceArray.Count()));
   		TRACE_FUNC_EXIT	   
    	return KErrOverflow;
    	}

    
    TBTDevice* device = iDeviceArray[index];
    // store pointer to the indexed TBTDevice

	// untrust trusted device to be deleted, and delete it only after that
    if( (device->iStatus & EStatusTrusted ) 
    	&& aDevice.iOperation== EOpUnpair )
        	{
        	device=new (ELeave) TBTDevice(*iDeviceArray[index]);
        	device->iOperation=EOPInternalUntust;
        	iQueue.Insert(device,0);
        	TInt rvalue=DoChangeDeviceL(*device);
        	TRACE_FUNC_EXIT	   
        	return rvalue;
        	}
    
 
    iDevice = new (ELeave) TBTDevice(*device);
    iDevice->iOperation = aDevice.iOperation;

    CBTDevice* regDevice = CBTDevice::NewL(iDevice->iAddr);

    regDevice->SetPaired( aDevice.iLinkKeyType );
    
    TBTDeviceSecurity security;
    
    if(aDevice.iOperation != EOpChangeName)
        {
        regDevice->SetDeviceNameL( BTDeviceNameConverter::ToUTF8L( iDevice->iName ) );
        
        // BTEngDevMan will delete friendly name when modify device if friendly name is not set
        // So if friendly name has been set before, it need to be set again before modify it for
        // any other purpuse e.g change security
        regDevice->SetFriendlyNameL(iDevice->iFriendlyName);
        }  
        
    switch (aDevice.iOperation)
        {
        case EOpUntrust:
        case EOPInternalUntust:        
        	security.SetNoAuthenticate(EFalse);
            security.SetNoAuthorise(EFalse);	
        	regDevice->SetGlobalSecurity(security);	        	
        	UnsetStatusFlags(iDeviceArray[index]->iStatus,EStatusTrusted );  	        
			break;
				        	
        case EOpTrust:	        
            security.SetNoAuthenticate(EFalse); 
            security.SetNoAuthorise(ETrue);	            
            security.SetBanned(EFalse);
        	regDevice->SetGlobalSecurity(security);	        	
            
            SetStatusFlags(iDeviceArray[index]->iStatus,EStatusTrusted );
            break;  

        case EOpUnblock:
         	// unblock may be done to a number of devices.
        	// So we want it to happen as quicky as possible
        	iRegistryObserver->Cancel();
          	UnsetStatusFlags( iDevice->iStatus,EStatusBlocked );
          	security.SetBanned(EFalse );
            regDevice->DeleteLinkKey();

            regDevice->SetGlobalSecurity(security);                   	
        	break;
        	
        case EOpBlock:	            
            security.SetBanned(ETrue );
            security.SetNoAuthenticate(EFalse );
            security.SetNoAuthorise(EFalse);
            regDevice->SetGlobalSecurity(security);
            regDevice->DeleteLinkKey();

			UnsetStatusFlags( iDevice->iStatus,EStatusTrusted );
        	SetStatusFlags( iDevice->iStatus,EStatusBlocked );
			break;

        case EOpUnpair:
        	// unpair may be done to a number of devices.
        	// So we want it to happen as quicky as possible
        	iRegistryObserver->Cancel();				
            regDevice->DeleteLinkKey();

            UnsetStatusFlags(iDevice->iStatus,EStatusPaired);
            security.SetNoAuthenticate(EFalse );
            security.SetNoAuthorise(EFalse );         
            regDevice->SetGlobalSecurity(security);               
            break;

        case EOpChangeName:
            if (IsNameExisting(aDevice.iName)) 
            	{
            	delete regDevice;
            	regDevice=NULL;
            	return KErrAlreadyExists;
            	}
                
            regDevice->SetFriendlyNameL(aDevice.iName);
            iDevice->iName = aDevice.iName;
            
            // set iFriendlyName to remember that friendly name has been set
            iDevice->iFriendlyName = aDevice.iName;
            break;

        default:
            delete regDevice;
            delete iDevice;
            iDevice=NULL;
			TRACE_FUNC_EXIT            
            return KErrNotSupported;
        }
    iDevMan->ModifyDevice(*regDevice);
    delete regDevice;

   	TRACE_FUNC_EXIT    
    return KErrNone;
    }
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::DoChangeDevice
// --------------------------------------------------------------------------------------------                
void CBTDevModelBase::DoCancelChangeL(const TBTDevice& /*aDevice*/)
    {
    TRACE_FUNC_ENTRY
    iDevMan->Cancel();    
	TRACE_FUNC_EXIT    
    }

// --------------------------------------------------------------------------------------------
// CBTDevModelBase::CreateDevice
// --------------------------------------------------------------------------------------------                    
TBTDevice* CBTDevModelBase::CreateDeviceL(const CBTDevice* aRegDevice,
        TNameEntry* aNameEntry)
    {
    TRACE_FUNC_ENTRY
    TRACE_BDADDR(aRegDevice->BDAddr());
    TRACE_INFO((_L("CoD %b"), aRegDevice->DeviceClass().DeviceClass()));
    TBTDevice* device = new (ELeave) TBTDevice();
    if ( aNameEntry )
        {
        device->iNameEntry = *aNameEntry;
        }
    switch( aRegDevice->DeviceClass().MajorDeviceClass() )
        {
        case EMajorDeviceComputer:
			device->iType=EDeviceComputer;
            break;
        case EMajorDevicePhone:
        	device->iType=EDevicePhone;
            break;
        case EMajorDeviceAudioDevice:
        	if ( aRegDevice->DeviceClass().MinorDeviceClass() == EMinorDeviceAVCarAudio ||
        	     aRegDevice->DeviceClass().MinorDeviceClass() == EMinorDeviceAVHandsfree)
        		{
        		device->iType=EDeviceCarkit;
        		}
        	else
        		{
        		device->iType=EDeviceAudio;
        		}
            break;
		
        case EMajorDevicePeripheral:
        	if ( aRegDevice->DeviceClass().MinorDeviceClass() == EMinorDevicePeripheralKeyboard )
        		{
				device->iType=EDeviceKeyboard;
        		}
        	else 
        		{
        		
        		if ( aRegDevice->DeviceClass().MinorDeviceClass() == EMinorDevicePeripheralPointer )
        			{
					device->iType=EDeviceMice;
        			}	
        		else
        			{
        			device->iType=EDeviceDefault;
        			}
        		}
        	break;
        	
        case EMajorDeviceImaging:
        	if ( aRegDevice->DeviceClass().MinorDeviceClass() == EMinorDeviceImagingPrinter )
        		{
				device->iType=EDevicePrinter;
        		}
        	else
        		{
        		device->iType=EDeviceDefault;
        		}
        	break;
        
        default:
        	device->iType=EDeviceDefault;
          	break;
        }
    device->iAddr = aRegDevice->BDAddr();
    
    TBTDeviceSecurity security = aRegDevice->GlobalSecurity();
    
    if(security.Banned() )
    	{
    	SetStatusFlags(device->iStatus,EStatusBlocked);
    	}
    if( IsUserAwarePaired( aRegDevice->AsNamelessDevice() ) )
    	{
    	SetStatusFlags(device->iStatus,EStatusPaired);
    	device->iLinkKeyType = aRegDevice->LinkKeyType();  	
    	}
    if( security.NoAuthorise())
    	SetStatusFlags(device->iStatus,EStatusTrusted);
	if(aRegDevice->FriendlyName().Length() >0)
		{
    	device->iName = aRegDevice->FriendlyName();
    	
    	// set iFriendlyName to remember that friendly name has been set before in registry 
    	device->iFriendlyName = aRegDevice->FriendlyName();
		}
	else
		{
		CleanupStack::PushL(device);
		device->iName = BTDeviceNameConverter::ToUnicodeL(aRegDevice->DeviceName());
		CleanupStack::Pop(device);
		}
        

	device->iDeviceClass=aRegDevice->DeviceClass();
    TRACE_FUNC_EXIT    	
    return device;
    }
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::AddDeviceL
// --------------------------------------------------------------------------------------------                    
void CBTDevModelBase::AddDeviceL(const CBTDevice* aRegDevice, 
        TNameEntry* aNameEntry, const TBTDeviceOp aOperation)
    {
    TRACE_FUNC_ENTRY
    TBTDevice* device = CreateDeviceL(aRegDevice, aNameEntry);
    device->iOperation = aOperation;
    // insert it to the RDeviceArray by order
	  iDeviceArray.InsertInOrderL(device,*iSortOrder);
    TRACE_FUNC_EXIT        
    }
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::CreateDevices
// --------------------------------------------------------------------------------------------                        
void CBTDevModelBase::CreateDevicesL(const CBTDeviceArray* aDeviceArray)
    {
    TRACE_FUNC_ENTRY
    // clear the old contents of the array
    for(TInt i=0;i<iDeviceArray.Count();i++)
    	delete iDeviceArray[i];
    iDeviceArray.Reset();
    
    TInt count = aDeviceArray->Count();
    for (TInt i = 0; i < count; i++)
        {
        // form a TBTDevice for a CBTDevice
        CBTDevice* regDevice = aDeviceArray->At(i);
		HandleNewDeviceL(regDevice, NULL);
		RenumberDeviceArray();
        }
    TRACE_FUNC_EXIT    
    }
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::IsNameExisting
// --------------------------------------------------------------------------------------------                        
TBool CBTDevModelBase::IsNameExisting(const TDesC& aName)
    {
    TRACE_FUNC_ENTRY
    for (TInt i = 0; i < iDeviceArray.Count(); i++)
        {
        if (iDeviceArray[i]->iName.Compare(aName) == 0)
        	{
        	return ETrue;
        	}
        }
    TRACE_FUNC_EXIT    
    return EFalse;
    }
// --------------------------------------------------------------------------------------------
// CBTDevModelBase::RegistryChangedL
// --------------------------------------------------------------------------------------------                            
void CBTDevModelBase::RegistryChangedL(const CBTDeviceArray* aDeviceArray)
    {
    TRACE_FUNC_ENTRY

	// Store the device seleted before refresh
    TInt bSelectedDeviceIndex = KErrNotFound;
    if (iObserver)
	{
    	bSelectedDeviceIndex=iObserver->CurrentItemIndex();
	}
	
    // the selected device before the update
    TBTDevice currentDevice;
        
    if (bSelectedDeviceIndex != KErrNotFound && bSelectedDeviceIndex < iDeviceArray.Count() )
    	{    	
    	currentDevice=*iDeviceArray[bSelectedDeviceIndex];    	
    	}    
    
	CreateDevicesL(aDeviceArray);

	TInt newIndex=GetIndexByAddress(currentDevice.iAddr,KErrNotFound);
	if (newIndex==KErrNotFound)
	{
		newIndex= bSelectedDeviceIndex;
	}

	newIndex=Min(newIndex,iDeviceArray.Count() -1 );

	// notify the listener about the new list of devices
	SendRefreshIfNoError(KErrNone,newIndex);    
	TRACE_FUNC_EXIT
    }
// ----------------------------------------------------------
// CBTDevModelBase::RenumberDeviceArray
//
// ReCalculates the indexes of internal array.
// ----------------------------------------------------------
void CBTDevModelBase::RenumberDeviceArray()
	{
	TRACE_FUNC_ENTRY
	for(TInt i=0;i<iDeviceArray.Count();i++)
		{
		iDeviceArray[i]->iIndex=i;
		}
	TRACE_FUNC_EXIT	
	}
//---------------------------------------------------------------------------------------------
// from MBTEngDevManObserver for call back on adding, modifying, deleting device completion
//---------------------------------------------------------------------------------------------
void  CBTDevModelBase::HandleDevManComplete(TInt aErr)
    {    
    TRACE_FUNC_ENTRY
    //command has been succesfully completed. If there is no command but something has
    // completed, something is very wrong.
	__ASSERT_DEBUG(iDevice != NULL || aErr != KErrNone, PANIC(EBTPanicDevManQueueIsCorrupt));    
	__ASSERT_DEBUG(iQueue.Count()>0, PANIC(EBTPanicDevManQueueIsCorrupt));    
	
    RenumberDeviceArray();
    
	// EOPInternalUntust is untrst before delete. It is not an operation of its own, so no-one is notified about it.
    if(iDevice && iDevice->iOperation== EOPInternalUntust)
    	{
    	delete(iDevice);
    	iDevice=NULL;
		delete iQueue[0];
		iQueue.Remove(0);
			    	
		HandleQueue();
		TRACE_FUNC_EXIT
		return;
    	}
	// in case of just paired device the refresh does
	// not work adequately quickly, so we have not refresh the
	// shown devices immediately.
	//
	// If we would not do it, there might be a small window, for
	// answering yes to to question if user wants to trust the
	// device. This would fail, since the device would not
	// be in the list.
    if( aErr == KErrNone && (iDevice->iOperation== EOpTrust || iDevice->iOperation== EOpUntrust ))
    	{
		TInt index=GetIndexByAddress(iDevice->iAddr);
		if(index != KErrNotFound)
			{    	
			if(iDevice->iOperation== EOpTrust )			
				{
				SetStatusFlags(iDeviceArray[index]->iStatus, EStatusTrusted);
				}				
			else
				{
				UnsetStatusFlags(iDeviceArray[index]->iStatus, EStatusTrusted);	
				}
							
	    	SendRefreshIfNoError(aErr);
			}
			
    	}
	// delete the unpaired and blocked devices from the list
	if( aErr == KErrNone && iDevice && 
			(iDevice->iOperation== EOpBlock || iDevice->iOperation== EOpUnpair ) )
		{
			TInt index=GetIndexByAddress(iDevice->iAddr);	
			if(index != KErrNotFound )
				{
					delete( iDeviceArray[index] );
					iDeviceArray.Remove(index);				
				}
			// do not send refresh if this and the next are unpair/unblock operations.
			// This is meant to hasten the screen refresh, in case of DeleteAll command
			// is issued.
			if( iQueue.Count()>1 && iQueue[1]->iOperation==iDevice->iOperation )
				{
				
				}
			else
				{
	    		SendRefreshIfNoError();	
	    		iRegistryObserver->StartIfNotRunning();				
	    		//NOTE:It is ok to attempt starting when allready running.	    		
				}
				
		}
    if(iObserver && iDevice)    
    	iObserver->NotifyChangeDeviceComplete(aErr, *iDevice);
    
    delete(iDevice);
    iDevice=NULL;
    
    iRegistryObserver->Refresh();

	delete iQueue[0];
	iQueue.Remove(0);
		    
	HandleQueue();
	TRACE_FUNC_EXIT	
    }
    
// ---------------------------------------------------------------------
// CBTDevModelBase::HandleGetDevicesComplete
// From MBTEngDevManObserver
//
// Devices are received from CBTRegistryObserver, so this is not used.
// ----------------------------------------------------------------------
void  CBTDevModelBase::HandleGetDevicesComplete(TInt /*aErr*/, CBTDeviceArray* /*aDeviceArray*/)
    {
	TRACE_FUNC_ENTRY
 
	TRACE_FUNC_EXIT    
    }
    

// ---------------------------------------------------------------------
// CBTDevModelBase::SendRefreshIfNoError
// ---------------------------------------------------------------------
void CBTDevModelBase::SendRefreshIfNoError(TInt aErr,TInt selectedItem)
	{
	TRACE_FUNC_ENTRY	
	
	//this shouldn't be reached if iObserver is NULL
	__ASSERT_DEBUG(iObserver, PANIC(EBTPanicNullObserver));
	
	if (selectedItem == KErrNotSupported )
		{
   		iObserver->RefreshDeviceList( &iDeviceArray ,
   		        Min(iObserver->CurrentItemIndex(),iDeviceArray.Count()-1  ) );    				
		}
	else
		{
	   	if (aErr == KErrNone && iObserver)
	   	    {
	   		iObserver->RefreshDeviceList( &iDeviceArray,
	   		        Min(selectedItem,iDeviceArray.Count()-1  ) );
	   	    }
		}
	
	TRACE_FUNC_EXIT   		  
	}    
// ---------------------------------------------------------------------
// CBTDevModelBase::ChangeDeviceL
//
// puts the change device command into Queue
// ---------------------------------------------------------------------	
void CBTDevModelBase::ChangeDeviceL(const TBTDevice& aDevice)
	{
	TRACE_FUNC_ENTRY
	
	TBTDevice* device=new(ELeave) TBTDevice(aDevice);		
	CleanupStack::PushL(device);

	TInt err = GetDevice(*device);
	if(err!=KErrNone)
		{
		CleanupStack::PopAndDestroy(device);
		User::Leave(err);
		}

	if(aDevice.iOperation==EOpChangeName)
		{
		device->iName=aDevice.iName;		
		}
	
	iQueue.Append(device);
	CleanupStack::Pop(device);	
	if(iQueue.Count() ==1 )
		{
			User::LeaveIfError( DoChangeDeviceL(*iQueue[0]) );
		}
	TRACE_FUNC_EXIT
	}

// ---------------------------------------------------------------------
// CBTDevModelBase::ChangeDevice
//
// Calls the ChangeDeviceL and traps leaves and calls error callback,
// if they occur.
// ---------------------------------------------------------------------	
void CBTDevModelBase::ChangeDevice(const TBTDevice& aDevice)
	{
	TRACE_FUNC_ENTRY		
	TRAPD(err,
		ChangeDeviceL(aDevice);
	);
	if(err!=KErrNone)
	{
		HandleLeave(err,&aDevice);	
	}
	TRACE_FUNC_EXIT
	}

// ---------------------------------------------------------------------
// CBTDevModelBase::CancelChange
//
// cancels the change from queue, only calls DoCancelChange,
// if the command is actually in progress.
// ---------------------------------------------------------------------		
void CBTDevModelBase::CancelChange(const TBTDevice& aDevice)	
	{
	TRACE_FUNC_ENTRY	
	// retrieve the device based on index, in
	// case the address is not filled in.
	TBTDevice device=aDevice;
	GetDevice(device);
	device.iOperation=aDevice.iOperation;
				
	// delete any operations to device from queueu
	if(iQueue.Count()>0 )
		{
		// Before calling DoCancelChangeL check if first operation on the 
		// queue is the one to be cancelled - otherwise crash may occure 
		// as per TSW EMZA-7EUHYE
		if(iQueue[0]->iAddr== device.iAddr &&
			iQueue[0]->iOperation== device.iOperation )
			TRAP_IGNORE(DoCancelChangeL(device));
		
		for (TInt i = iQueue.Count() - 1; i >= 0; i--)
			{
			if(iQueue[i]->iAddr== device.iAddr &&
				iQueue[i]->iOperation== device.iOperation )
				{
				delete iQueue[i];
				iQueue.Remove(i);
				}
			}
		}
	// failed cancel is not reported forward.		
	TRACE_FUNC_EXIT			
	}
	
// ---------------------------------------------------------------------
// CBTDevModelBase::DeviceChangeInProgress
// ---------------------------------------------------------------------
TBool CBTDevModelBase::DeviceChangeInProgress()
	{
	TRACE_FUNC_ENTRY
	TRACE_FUNC_EXIT
	return iQueue.Count() !=0;
	}

// ---------------------------------------------------------------------
// CBTDevModelBase::HandleQueue
//
// Starts executing the next devicechange(if any).
// Currently Executed command must be deleted from iQueue and iDevice
// (and corresponding places in subclasses), before calling this method.
// ---------------------------------------------------------------------
	
void CBTDevModelBase::HandleQueue()
	{
	TRACE_FUNC_ENTRY
	
	RenumberDeviceArray();
	if(iQueue.Count() >0 )
		{
		TRAPD(err2,	DoChangeDeviceL(*iQueue[0]); );
		if (err2 !=KErrNone)
			{
			HandleLeave(err2,iQueue[0]);
			}
		}
	TRACE_FUNC_EXIT		
	}

// ---------------------------------------------------------------------
// CBTDevModelBase::HandleLeave
// ---------------------------------------------------------------------
void CBTDevModelBase::HandleLeave(TInt aErr,const TBTDevice* aDevice )
	{
	TRACE_FUNC_ENTRY
	iRegistryObserver->StartIfNotRunning();
	iRegistryObserver->Refresh();
	if(aDevice == NULL )
		{		
		iObserver->NotifyChangeDeviceComplete(aErr,TBTDevice() );	
		} 
	else
		{		
		iObserver->NotifyChangeDeviceComplete(aErr, *aDevice );	
		} 
	delete iDevice;
	iDevice=NULL;
	iDevMan->Cancel();

	if(aDevice != NULL && iQueue.Count() > 0 && 
		(iQueue[0]->iAddr == aDevice->iAddr ) && 
		(iQueue[0]->iOperation == aDevice->iOperation ) )
		{
		delete iQueue[0];
		iQueue.Remove(0);
		}	
	
	HandleQueue();
	TRACE_FUNC_EXIT
	}
