/*
 * ==============================================================================
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
 * Description:
 * 
 * ==============================================================================
 */

#include <stringresourcereader.h>
#include <data_caging_path_literals.hrh> 
#include <bautils.h>

#include <dcmo.rsg>
#include <btfeaturescfg.h>

#include "btctrldcmoadapter.h"

_LIT(KBluetoothDcmoPanicName, "BT DCMO Adapter");

_LIT( KBluetoothControlProperty, "Bluetooth" );
_LIT( KDisableBluetoothDescription, "Used to enable/disable the Bluetooth connectivity." ); // Description
_LIT( KRuntimeResourceFileName, "z:dcmo.rsc" );

TDCMOStatus CBluetoothCtrlDcmoAdapter::MapFeatureControlError(TInt aErrorCode)
    {
    TDCMOStatus status = EDcmoFail;
    
    switch (aErrorCode)
        {
        case KErrNone:
            status = EDcmoSuccess;
            break;
        case KErrPermissionDenied:
        case KErrAccessDenied:
            status = EDcmoAccessDenied;
            break;
        case KErrNotFound:
            status = EDcmoNotFound;
            break;
        default:
            break;
        }
    return status;
    }

// Construction and destruction functions

CBluetoothCtrlDcmoAdapter* CBluetoothCtrlDcmoAdapter::NewL(TAny* aParams)
	{
    CBluetoothCtrlDcmoAdapter* self = new (ELeave ) CBluetoothCtrlDcmoAdapter(aParams);  
	return self;
	}

CBluetoothCtrlDcmoAdapter::~CBluetoothCtrlDcmoAdapter()
	{	
	}

CBluetoothCtrlDcmoAdapter::CBluetoothCtrlDcmoAdapter(TAny* aInitParams)
    :  iInitParams(reinterpret_cast<CDCMOInterface::TDCMOInterfaceInitParams*>(aInitParams))
	{
	}

// Implementation of CDCMOInterface

//
// See excerpts from OMA DCMO spec below
//
TDCMOStatus CBluetoothCtrlDcmoAdapter::SetDCMOPluginIntAttributeValueL(TDCMONode aId, TInt aValue)
{
	TDCMOStatus status = EDcmoFail;
	
	switch(aId)
	{
        /*
        This node is used with Exec command to enable the Device Capability to transfer the Device Capability 
        from Disabled State to Enabled state.
        */
		case EEnable:
		    {
			BluetoothFeatures::TEnterpriseEnablementMode mode = BluetoothFeatures::EDisabled;
			switch (aValue)
				{
				case 0:
					mode = BluetoothFeatures::EDisabled;
					break;
				case 1:
					mode = BluetoothFeatures::EEnabled;
					break;
				case 2:
					mode = BluetoothFeatures::EDataProfilesDisabled;
					break;
				default:
					User::Leave(KErrArgument);
				}
		    TRAPD(err,BluetoothFeatures::SetEnterpriseEnablementL(mode));
	        status = MapFeatureControlError(err);
		    }
		    break;
		default:
		    break;
	}
	
	return status;
}

//
// See excerpts from OMA DCMO spec below
//
TDCMOStatus CBluetoothCtrlDcmoAdapter::GetDCMOPluginIntAttributeValueL(TDCMONode aId, TInt& aValue)
{
	TDCMOStatus status = EDcmoSuccess;
	
	switch(aId)
	{
	    /*
	    This node specifies the group name, defined in section 6.1 “Grouping of properties” of [DPE-TS],
	    for a collection of Device Capabilities
	    */
		case EGroup: 
		    aValue = EConnectivity;
		    break;
		/*
		This leaf node indicates whether the Device Capability is removable and whether it is currently
        attached to the device. 
        If the leaf exists then the Device Capability is removable. 
        If the value of this node is “True” the Device Capability is currently attached to the device; 
        if the value of this node is “False” the Device Capability is currently detached from the device;
        
        NB: Returning EFalse here prevents the node from being inserted into DDF, so Bluetooth is NON-removable media.
		*/  
		case EAttached:
		    aValue = EFalse;
		    break;
		/*
        This leaf node indicates whether the Device Capability is enabled regardless whether 
        the Device Capability is attached or not. 
        If the value of this node is “True” the Device Capability is in Enabled State. 
        If the value of is “False” the Device Capability is in Disabled State;
        The ‘Attached’ property is independent of ‘Enabled’ property. A Device Capability MAY have ‘True’ 
        as value for ‘Enabled’ node while having ‘False’ as value for the ‘Attached’ node. That means 
        the Device Capability is still not available and can’t be used until it is attached to the Device, 
        but will be useable once the Device Capability is attached.
		*/
		case EEnabled:
			{
			BluetoothFeatures::TEnterpriseEnablementMode mode = BluetoothFeatures::EnterpriseEnablementL();
			switch (mode)
				{
				case BluetoothFeatures::EDisabled:
					aValue = 0;
					break;
				case BluetoothFeatures::EEnabled:
					aValue = 1;
					break;
				case BluetoothFeatures::EDataProfilesDisabled:
					aValue = 2;
					break;
				default:
					User::Leave(KErrArgument);
				}
			}
		    break; 
		/*
        This leaf node specifies whether the user is able to enable a Device Capability. 
        If the node is not present or the value is False, the user is allowed to enable the Device Capability.
        If the node is present and the value is True, the user is not allowed to enable the Device Capability.
		*/
		case EDenyUserEnable:
		    aValue = ETrue;
		    break;
		/*
        This leaf node specifies whether the user is notified with the DCMO Operation result when 
        enable/disable Primitive is executed. 
        If the node is not present or the value is ‘False’, the user will not be notified about the result of 
        the operation. 
        If the node is present and the value is ‘True’, the user will be notified about the result of 
        the operation.		
		*/
		case ENotifyUser:
		    aValue = ETrue;
		    break;
		default:
		    break;
	}
	
	return status;
}

TDCMOStatus CBluetoothCtrlDcmoAdapter::SetDCMOPluginStrAttributeValueL(TDCMONode   /* aId */, 
                                                                       const TDes& /* aStrValue */)
{
	// Nothing to do
	return EDcmoFail;	
}

TDCMOStatus CBluetoothCtrlDcmoAdapter::GetDCMOPluginStrAttributeValueL(TDCMONode aId, TDes& aStrValue) 
{
	TDCMOStatus status = EDcmoSuccess;
	switch(aId)
	{
	    /*
	    This leaf node specifies the property name, which is defined in section 6.2 “DPE vocabulary List” 
	    of [DPE-TS], for a Device Capability.
	    */
		case EProperty: 
		    __ASSERT_DEBUG(aStrValue.MaxLength() >= KBluetoothControlProperty().Length(),Panic(CBluetoothCtrlDcmoAdapter::EPropertyBufferTooSmall));
		    aStrValue = KBluetoothControlProperty().Left(aStrValue.MaxLength());
		    break;
		/*
		This leaf node specifies the description for this Device Capability.
		*/
		case EDescription:
            __ASSERT_DEBUG(aStrValue.MaxLength() >= KDisableBluetoothDescription().Length(),Panic(CBluetoothCtrlDcmoAdapter::EDescriptionBufferTooSmall));
		    aStrValue = KDisableBluetoothDescription().Left(aStrValue.MaxLength());
		    break;
		default:			
		    status = EDcmoNotFound;
		    break;
	}
		
	return status;
}

void CBluetoothCtrlDcmoAdapter::GetLocalizedNameL(HBufC*& aLocName)
    {
    TFileName* fileName = new(ELeave) TFileName;
    CleanupStack::PushL(fileName);
    TParse* parseObj = new(ELeave) TParse();
    CleanupStack::PushL(parseObj);
    User::LeaveIfError(parseObj->Set(KRuntimeResourceFileName(), &KDC_RESOURCE_FILES_DIR, NULL));
    *fileName = parseObj->FullName();
    CleanupStack::PopAndDestroy(parseObj);
    
    CStringResourceReader* resReader = CStringResourceReader::NewL(*fileName);
    CleanupStack::PushL(resReader);
    
    TPtrC buf;
    buf.Set(resReader->ReadResourceString(R_DM_RUN_TIME_VAR_BLUETOOTH)); 
    aLocName = buf.AllocL() ; 
    
    CleanupStack::PopAndDestroy(2,fileName); //resReader,fileName
    }

void CBluetoothCtrlDcmoAdapter::Panic(TBluetoothDcmoPanic aPanic)
    {
    User::Panic(KBluetoothDcmoPanicName, aPanic);
    }
