/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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

// INCLUDE FILES
#include <e32cmn.h>
#include "btdevmodel.h"
#include "btpairedmodel.h"
#include "btblockedmodel.h"
#include "debug.h"

// --------------------------------------------------------------------------------------------
// CBTDevModel::CBTDevModel
// --------------------------------------------------------------------------------------------                            
CBTDevModel::CBTDevModel()
    {
    }
// --------------------------------------------------------------------------------------------
// CBTDevModel::NewL
// --------------------------------------------------------------------------------------------                            
EXPORT_C CBTDevModel* CBTDevModel::NewL(const TBTDeviceGroup aGroup,
                                        MBTDeviceObserver* aObserver,
                                        TBTDeviceSortOrder* aOrder 
                                         )
    {
    CBTDevModel* self = new (ELeave) CBTDevModel();
    CleanupStack::PushL(self);
    
    TBTDeviceSortOrder *order=NULL;    
    if(aOrder == NULL)
    	{
    	order=new(ELeave) TBTDeviceSortOrder(TBTDeviceComparison::CompareAphabetically);    	
    	// Order ownership goes to btdevmodel - don't push it to stack
    	self->ConstructL(aGroup, aObserver, order);
    	}    	
    else
		{
    	order=aOrder;		
		self->ConstructL(aGroup, aObserver, order);    	
		}        
    CleanupStack::Pop(self);
    return self;
    }    
// --------------------------------------------------------------------------------------------
// CBTDevModel::~CBTDevModel
//
// Destructor
// --------------------------------------------------------------------------------------------

CBTDevModel::~CBTDevModel()
    {
	TRACE_FUNC_ENTRY
    delete iDevModel;
    }
// --------------------------------------------------------------------------------------------
// CBTDevModel::ConstructL
//
// Constructs CBTPairedModel or CBTBlockedModel, depending on aGroup
// --------------------------------------------------------------------------------------------                                
void CBTDevModel::ConstructL(const TBTDeviceGroup aGroup,
                             MBTDeviceObserver* aObserver,
                             TBTDeviceSortOrder* aOrder  )
    { 
	TRACE_FUNC_ENTRY    
    iGroup=aGroup;
    switch (aGroup)
        {
        case EGroupPaired:
            iDevModel = CBTPairedModel::NewL(aObserver, aOrder);
            break;
        
        case EGroupBlocked:
            iDevModel = CBTBlockedModel::NewL(aObserver, aOrder);
            break;
        default:
            break;
        }
	TRACE_FUNC_EXIT
    }

EXPORT_C TBool CBTDevModel::IsAnyDeviceConnected()
	{
	return iDevModel->IsAnyDeviceConnected();
	}
// --------------------------------------------------------------------------------------------
// CBTDevModel::GetDevice
// wrapper function
// --------------------------------------------------------------------------------------------                                
EXPORT_C TInt CBTDevModel::GetDevice(TBTDevice& aDevice)
    {
    return iDevModel->GetDevice(aDevice);
    }
// --------------------------------------------------------------------------------------------
// CBTDevModel::CreateDevice 
// wrapper function
// --------------------------------------------------------------------------------------------                                
EXPORT_C TBTDevice* CBTDevModel::CreateDeviceL(const CBTDevice* aRegDevice, 
        TNameEntry* aNameEntry)
	{
		return iDevModel->CreateDeviceL(aRegDevice, aNameEntry);
	}

// --------------------------------------------------------------------------------------------
// CBTDevModel::GetDeviceType
// wrapper function
// --------------------------------------------------------------------------------------------                                    
EXPORT_C void CBTDevModel::ChangeAllDevices(const TBTDeviceOp aOperation)
    {
    iDevModel->ChangeAllDevices(aOperation);
    }
// --------------------------------------------------------------------------------------------
// CBTDevModel::ChangeDevice
// wrapper function
// --------------------------------------------------------------------------------------------    
EXPORT_C void CBTDevModel::ChangeDevice(const TBTDevice& aDevice)
    {
    iDevModel->ChangeDevice(aDevice);    
    }
// --------------------------------------------------------------------------------------------
// CBTDevModel::ChangeDevice
// --------------------------------------------------------------------------------------------       
EXPORT_C void CBTDevModel::ChangeDevice(const CBTDevice* aDevice,
        TNameEntry* aNameEntry, TBTDeviceOp aOp)
    {
    	TRACE_FUNC_ENTRY
		TBTDevice *tmp=NULL;      	
    	TInt err = KErrNone;
		if(aOp == EOpPair)
    		{
    		if(iGroup !=EGroupPaired )
    			{
    			err = KErrNotSupported;
    			}
    		else
    			{
    			TInt err2 = KErrNone;
    			TRAP(err, err2 = static_cast<CBTPairedModel*>(iDevModel)->PairDeviceL( aDevice, aNameEntry));
    			err = (err2 != KErrNone) ? err2 : err;
    			}
    		}
		else 
			{
			TRAP(err, tmp=iDevModel->CreateDeviceL(aDevice, aNameEntry ));
			if(err == KErrNone)
				{
				tmp->iOperation=aOp;
				iDevModel->ChangeDevice(*tmp );
				}
			}
		if(err != KErrNone)
		    {
		    iDevModel->HandleLeave(err,tmp);
		    }
    	TRACE_FUNC_EXIT
    }        
// --------------------------------------------------------------------------------------------
// CBTDevModel::CancelChange
// wrapper function
// --------------------------------------------------------------------------------------------        
EXPORT_C void CBTDevModel::CancelChange(const TBTDevice& aDevice)
    {
    iDevModel->CancelChange(aDevice);
    }

// --------------------------------------------------------------------------------------------
// CBTDevModel::DeviceChangeInProgress
// wrapper function
// --------------------------------------------------------------------------------------------        
EXPORT_C TBool CBTDevModel::DeviceChangeInProgress()
	{
	return iDevModel->DeviceChangeInProgress();
	}
// --------------------------------------------------------------------------------------------
// TBTDeviceComparison::CompareAphabetically
// --------------------------------------------------------------------------------------------
EXPORT_C TInt TBTDeviceComparison::CompareAphabetically(
	  const TBTDevice& aFirstDevice, const TBTDevice& aSecondDevice) 
	{  	
	// This funtion is used when sorting devicelists, so the tracing has been omnitted
	// in order decreas the amount of trash in the logs.
	TInt rvalue= aFirstDevice.iName.CompareC(aSecondDevice.iName);
	if(rvalue !=0)
		{
		return rvalue;
		}		
	if( aFirstDevice.iAddr <= aSecondDevice.iAddr )
		{
		return  -1;
		}			
	else
		{
		return 1;			
		}		
	}

// --------------------------------------------------------------------------------------------
// TBTDeviceComparison::ReverseCompareAphabetically
// --------------------------------------------------------------------------------------------	
EXPORT_C TInt TBTDeviceComparison::ReverseCompareAphabetically(
        const TBTDevice& aFirstDevice, const TBTDevice& aSecondDevice) 
	{  	
	// This funtion is used when sorting devicelists, so the tracing has been omnitted
	// in order decreas the amount of trash in the logs.
	TInt rvalue= aSecondDevice.iName.CompareC(aFirstDevice.iName);
	if(rvalue !=0)
		{
		return rvalue;
		}		
	if(aFirstDevice.iAddr <= aSecondDevice.iAddr )
		{
		return 1;
		}			
	else
		{
		return -1;			
		}		
	}    
