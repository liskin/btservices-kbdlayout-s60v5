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
 @publishedAll
 @released
*/

#ifndef AVRCPSPEC_H
#define AVRCPSPEC_H

#include <e32base.h>

// The IANA assigned MIB Enum for UTF8, defined at:
//    http://www.iana.org/assignments/character-sets
// and specified in the AVRCP 1.3 specification in
// section 13, "References" as reference number 11.

const TInt KUtf8MibEnum = 106;

// The maximum absolute volume defined by the AVRCP 1.4 spcification
// in section 6.13.1
const TUint8 KAvrcpMaxAbsoluteVolume = 0x7F;

// These PDU ids are defined by the AVRCP 1.3 specification
// in sections 5.1, 5.2, 5.3 and 5.4.

enum TMetadataTransferPDU
	{
	EGetCapabilities							= 0x10,
	EListPlayerApplicationSettingAttributes		= 0x11,
	EListPlayerApplicationSettingValues			= 0x12,
	EGetCurrentPlayerApplicationSettingValue	= 0x13,
	ESetPlayerApplicationSettingValue			= 0x14,
	EGetPlayerApplicationSettingAttributeText	= 0x15,
	EGetPlayerApplicationSettingValueText		= 0x16,
	EInformDisplayableCharacterSet				= 0x17, // not implemented; always use UTF8
	EInformBatteryStatusOfCT					= 0x18,
	EGetElementAttributes						= 0x20,
	EGetPlayStatus								= 0x30,
	ERegisterNotification						= 0x31,
	ERequestContinuingResponse					= 0x40,
	EAbortContinuingResponse					= 0x41,
	ESetAbsoluteVolume							= 0x50,
	ESetAddressedPlayer							= 0x60,
	EPlayItem									= 0x74,
	EAddToNowPlaying							= 0x90,
	
	// Internal values
	EGetPlayStatusUpdate						= 0xff,
	};

enum TMediaBrowsePDU
	{
	EMbSetBrowsedPlayer                           = 0x70,
	EMbGetFolderItems                             = 0x71,
	EMbChangePath                                 = 0x72,
	EMbGetItemAttributes                          = 0x73,
	EMbSearch                                     = 0x80,
	};

// These RegisterNotification (PDU 0x31) event ids are defined
// in the AVRCP 1.3 specification in section 5.4.

enum TRegisterNotificationEvent
	{
	ERegisterNotificationPlaybackStatusChanged	 		 = 0x1,
	ERegisterNotificationTrackChanged					 = 0x2,
	ERegisterNotificationTrackReachedEnd				 = 0x3,
	ERegisterNotificationTrackReachedStart				 = 0x4,
	ERegisterNotificationPlaybackPosChanged				 = 0x5,
	ERegisterNotificationBatteryStatusChanged			 = 0x6,
	ERegisterNotificationSystemStatusChanged_NotSupported= 0x7, // not supported
	ERegisterNotificationPlayerApplicationSettingChanged = 0x8,
	ERegisterNotificationNowPlayingContentChanged		 = 0x9,
	ERegisterNotificationAvailablePlayersChanged		 = 0xa,
	ERegisterNotificationAddressedPlayerChanged			 = 0xb,
	ERegisterNotificationUidsChanged					 = 0xc,
	ERegisterNotificationVolumeChanged					 = 0xd,
	ERegisterNotificationReservedLast				 	 = 0xf,
	};

// These are the allowed values for GetCapabilities (PDU 0x10)
// and are specified in the AVRCP 1.3 specification, section 5.1.1

enum TGetCapabilityValues
	{
	ECapabilityIdCompanyID			= 2,
	ECapabilityIdEventsSupported	= 3,
	};

// Pass Through command values for Group Navigation, as
// defined in the AVRCP 1.3 specification, in section 4.7.9

enum TGroupNavigationPassthroughOperationIds
	{
	ENextGroup				= 0x0,
	EPreviousGroup			= 0x1,
	};

// Error numbers allocated to the AVRCP component, for use in
// IPC communication between the client APIs and the AVRCP bearer

const TInt KErrAvrcpBaseError                     = -6751;  // Base error number
const TInt KErrAvrcpInvalidCType                  = KErrAvrcpBaseError - 1; // -6752
const TInt KErrAvrcpInvalidOperationId            = KErrAvrcpBaseError - 2; // -6753
const TInt KErrAvrcpMetadataInvalidCommand        = KErrAvrcpBaseError - 3; // -6754
const TInt KErrAvrcpMetadataInvalidParameter      = KErrAvrcpBaseError - 4; // -6755
const TInt KErrAvrcpMetadataParameterNotFound     = KErrAvrcpBaseError - 5; // -6756
const TInt KErrAvrcpMetadataInternalError         = KErrAvrcpBaseError - 6; // -6757
const TInt KErrAvrcpHandledInternallyRespondNow   = KErrAvrcpBaseError - 7; // -6758
const TInt KErrAvrcpHandledInternallyInformRemCon = KErrAvrcpBaseError - 8; // -6759
const TInt KErrAvrcpInternalCommand 				= KErrAvrcpBaseError - 9; // -6760
const TInt KErrAvrcpFurtherProcessingRequired		= KErrAvrcpBaseError - 10; // -6791

const TInt KErrAvrcpAirBase							= -6780;
const TInt KErrAvrcpAirInvalidCommand 				= KErrAvrcpAirBase - 0;
const TInt KErrAvrcpAirInvalidParameter 			= KErrAvrcpAirBase - 1;
const TInt KErrAvrcpAirParameterNotFound 			= KErrAvrcpAirBase - 2;
const TInt KErrAvrcpAirInternalError 				= KErrAvrcpAirBase - 3;
const TInt KErrAvrcpAirSuccess 						= KErrAvrcpAirBase - 4;
const TInt KErrAvrcpAirUidChanged 					= KErrAvrcpAirBase - 5;
const TInt KErrAvrcpAirReserved 					= KErrAvrcpAirBase - 6;
const TInt KErrAvrcpAirInvalidDirection 			= KErrAvrcpAirBase - 7;
const TInt KErrAvrcpAirNotADirectory 				= KErrAvrcpAirBase - 8;
const TInt KErrAvrcpAirDoesNotExist 				= KErrAvrcpAirBase - 9;
const TInt KErrAvrcpAirInvalidScope 				= KErrAvrcpAirBase - 0xa;
const TInt KErrAvrcpAirRangeOutOfBounds 			= KErrAvrcpAirBase - 0xb;
const TInt KErrAvrcpAirUidIsADirectory 				= KErrAvrcpAirBase - 0xc;
const TInt KErrAvrcpAirMediaInUse 					= KErrAvrcpAirBase - 0xd;
const TInt KErrAvrcpAirNowPlayingListFull 			= KErrAvrcpAirBase - 0xe;
const TInt KErrAvrcpAirSearchNotSupported 			= KErrAvrcpAirBase - 0xf;
const TInt KErrAvrcpAirSearchInProgress 			= KErrAvrcpAirBase - 0x10;
const TInt KErrAvrcpAirInvalidPlayerId 				= KErrAvrcpAirBase - 0x11;
const TInt KErrAvrcpAirPlayerNotBrowesable 			= KErrAvrcpAirBase - 0x12;
const TInt KErrAvrcpAirPlayerNotAddressed 			= KErrAvrcpAirBase - 0x13;
const TInt KErrAvrcpAirNoValidSearchResults 		= KErrAvrcpAirBase - 0x14;
const TInt KErrAvrcpAirNoAvailablePlayers 			= KErrAvrcpAirBase - 0x15;
const TInt KErrAvrcpAirAddressedPlayerChanged 		= KErrAvrcpAirBase - 0x16;
const TInt KErrAvrcpInvalidScope                    = KErrAvrcpAirBase - 0x17;

#endif // AVRCPSPEC_H
