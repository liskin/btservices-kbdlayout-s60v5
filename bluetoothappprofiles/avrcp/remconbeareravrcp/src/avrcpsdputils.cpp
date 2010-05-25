// Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include <bt_sock.h>

#include "avrcpsdputils.h"

void AvrcpSdpUtils::CreateServiceRecordL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle, TBool aTarget, TUint16 aProfileVersion)
	{
	if(!aTarget)
		{
		// New service record with service class attribute containing
		// 0x110e (for A/V Remote Control) 
		CSdpAttrValueDES* serviceClassUuids = CSdpAttrValueDES::NewDESL(NULL);
		CleanupStack::PushL(serviceClassUuids);
		
		if(aProfileVersion == AvrcpSdp::KAvrcpProfileVersion13)
			{
			serviceClassUuids->StartListL()
					->BuildUUIDL(TUUID(TUint16(KAVRemoteControlUUID)))
			->EndListL();
			}
		else
			{
			serviceClassUuids->StartListL()
					->BuildUUIDL(TUUID(TUint16(KAVRemoteControlUUID)))
					->BuildUUIDL(TUUID(TUint16(KAVRemoteControlControllerUUID)))
			->EndListL();
			}
		
		aSdpDatabase.CreateServiceRecordL(*serviceClassUuids, aRecHandle);
		CleanupStack::PopAndDestroy(serviceClassUuids);

		aSdpDatabase.UpdateAttributeL(aRecHandle,
			KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceName,
			AvrcpSdp::KAvrcpControllerServiceName
			);
		}
	else
		{
		// New service record with service class attribute containing
		// 0x110c (for A/V Remote Control Target) 
		aSdpDatabase.CreateServiceRecordL(KAVRemoteControlTargetUUID, aRecHandle);
		
		aSdpDatabase.UpdateAttributeL(aRecHandle,
			KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceName,
			AvrcpSdp::KAvrcpTargetServiceName
			);
		}

	aSdpDatabase.UpdateAttributeL(aRecHandle,
		KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetProviderName,
		AvrcpSdp::KAvrcpProviderName
		);
	aSdpDatabase.UpdateAttributeL(aRecHandle,
		KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceDescription,
		AvrcpSdp::KAvrcpServiceDescription
		);	
	}

void AvrcpSdpUtils::UpdateProtocolDescriptorListL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle, TUint16 aProtocolVersion)
	{
	CSdpAttrValueDES *attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);
	attrValDES->StartListL()
		->BuildDESL()->StartListL()
			->BuildUUIDL(TUUID(TUint16(KL2CAP)))					// L2CAP
			->BuildUintL(TSdpIntBuf<TUint16>(KAVCTP))				// PSM = AVCTP
		->EndListL()
		->BuildDESL()->StartListL()
			->BuildUUIDL(TUUID(TUint16(KAVCTP)))					// AVCTP
			->BuildUintL(TSdpIntBuf<TUint16>(aProtocolVersion))
		->EndListL()
	->EndListL();
	aSdpDatabase.UpdateAttributeL(aRecHandle, KSdpAttrIdProtocolDescriptorList, *attrValDES);
	CleanupStack::PopAndDestroy(attrValDES);
	}

void AvrcpSdpUtils::UpdateAdditionalProtocolDescriptorListL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle)
	{
	CSdpAttrValueDES *attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);
	attrValDES->StartListL()
		->BuildDESL()->StartListL()
			->BuildUUIDL(TUUID(TUint16(KL2CAP)))				// L2CAP
			->BuildUintL(TSdpIntBuf<TUint16>(0x1b))				// PSM = AVCTP_browse
		->EndListL()
		->BuildDESL()->StartListL()
			->BuildUUIDL(TUUID(TUint16(KAVCTP)))					// AVCTP
			->BuildUintL(TSdpIntBuf<TUint16>(AvrcpSdp::KAvctpProtocolVersion13))// 0x0103
		->EndListL()
	->EndListL();
	aSdpDatabase.UpdateAttributeL(aRecHandle, KSdpAttrIdAdditionalProtocolDescriptorList, *attrValDES);
	CleanupStack::PopAndDestroy(attrValDES);
	}

void AvrcpSdpUtils::UpdateBrowseListL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle)
	{
	CSdpAttrValueDES *attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);
	attrValDES->StartListL()
			  	->BuildUUIDL(TUUID(TUint16(KPublicBrowseGroupUUID)))	 // Public browse group										
		      	->EndListL();
	aSdpDatabase.UpdateAttributeL(aRecHandle, KSdpAttrIdBrowseGroupList, *attrValDES);
	CleanupStack::PopAndDestroy(attrValDES);
	}

void AvrcpSdpUtils::UpdateProfileDescriptorListL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle, TUint16 aProfileVersion)
	{
	CSdpAttrValueDES *attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);
	attrValDES->StartListL()
		->BuildDESL()->StartListL()
			//In the profile descriptor list the Control UUID is used 
			//for BOTH controller and target
			->BuildUUIDL(KAVRemoteControlUUID) 
			->BuildUintL(TSdpIntBuf<TUint16>(aProfileVersion))
		->EndListL()
	->EndListL();
	aSdpDatabase.UpdateAttributeL(aRecHandle, KSdpAttrIdBluetoothProfileDescriptorList, *attrValDES);
	CleanupStack::PopAndDestroy(attrValDES);
	}

void AvrcpSdpUtils::UpdateSupportedFeaturesL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle, AvrcpSdp::TRecordType aType, TUint16 aFeatures)
	{
	// Supported Features
	// For both target and controller roles if we support that role then
	// indicate support for all categories that are available within that
	// role.												
	CSdpAttrValue* attrVal = NULL;
	TSdpIntBuf<TUint16> featureBuf = (aType==AvrcpSdp::ERemoteControl) ? AvrcpSdp::KAvrcpBaseCtFeatures | aFeatures : AvrcpSdp::KAvrcpBaseTgFeatures | aFeatures;
	attrVal = CSdpAttrValueUint::NewUintL(featureBuf); 
	CleanupStack::PushL(attrVal);
	aSdpDatabase.UpdateAttributeL(aRecHandle, KSdpAttrIdSupportedFeatures, *attrVal);
	CleanupStack::PopAndDestroy(attrVal);
	}
