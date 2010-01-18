// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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



/**
 @file
 @internalComponent
*/

#ifndef AVRCPSDPUTILS_H
#define AVRCPSDPUTILS_H

#include <e32base.h>
#include <btsdp.h>

namespace AvrcpSdp
	{
	enum TRecordType
		{
		ERemoteControl,
		ERemoteControlTarget,
		};
	
	_LIT8(KAvrcpProviderName, "Symbian OS");
	_LIT8(KAvrcpTargetServiceName, "AVRCP Target");	
	_LIT8(KAvrcpControllerServiceName, "AVRCP Controller");
	_LIT8(KAvrcpServiceDescription, "Audio Video Remote Control");
	
	const TUint16 KAvrcpProfileVersion14 = 0x0104;
	const TUint16 KAvrcpProfileVersion13 = 0x0103;
	const TUint16 KAvctpProtocolVersion13 = 0x103;
	const TUint16 KAvctpProtocolVersion12 = 0x102;
	
	enum TAvrcpFeature
		{
		EPlayerRecorder		= 1 << 0,
		EMonitorAmplifier	= 1 << 1,
		ETuner				= 1 << 2,
		EMenu				= 1 << 3,
		EPlayer				= 1 << 4,
		EGroupNavigation	= 1 << 5,
		EBrowsing			= 1 << 6,
		EMultiplePlayers	= 1 << 7,
		};
	
	const TUint16 KAvrcpBaseCtFeatures = 	EPlayerRecorder | 
											EMonitorAmplifier |
											ETuner |
											EMenu;
	
	const TUint16 KAvrcpBaseTgFeatures = 	EPlayerRecorder | 
											EMonitorAmplifier |
											ETuner |
											EMenu |
											EMultiplePlayers;
	}

NONSHARABLE_CLASS(AvrcpSdpUtils)
	{
public:
	static void CreateServiceRecordL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle, TBool aTarget, TUint16 aProfileVersion);
	static void UpdateProtocolDescriptorListL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle, TUint16 aProtocolVersion);
	static void UpdateAdditionalProtocolDescriptorListL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle);
	static void UpdateBrowseListL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle);
	static void UpdateProfileDescriptorListL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle, TUint16 aProfileVersion);
	static void UpdateSupportedFeaturesL(RSdpDatabase& aSdpDatabase, TSdpServRecordHandle& aRecHandle, AvrcpSdp::TRecordType aType, TUint16 aFeatures);
	};

#endif // AVRCPSDPUTILS_H
