// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @prototype
*/

#include <avcframe.h>
#include <e32base.h>
#include <remcon/remconbearerobserver.h>
#include <remcon/remconconverterplugin.h>
#include <remcon/messagetype.h>
#include <remcon/avrcpspec.h>
#include <remconbeareravrcp.h>

#include <remconbatterytargetobserver.h>
#include <absolutevolumeapi.h>
#include <absolutevolumeutils.h>

#include "avrcp.h"
#include "controlcommand.h"
#include "avrcpcommandframer.h"
#include "avrcpinternalinterface.h"
#include "avrcpipc.h"
#include "avrcplog.h"
#include "avrcputils.h"
#include "avrcpincomingcommandhandler.h"
#include "mediabrowse.h"
#include "mediainformation.h"
#include "nowplaying.h"
#include "playerinformation.h"
#include "remconbattery.h"
#include "remcongroupnavigation.h"

TInt CControlCommand::ParseMetadataTransferPassthroughCommand()
	{
	// company id and frame type should already be set before here
	__ASSERT_DEBUG(iFrame->Type() == AVC::EControl,  AvrcpUtils::Panic(EAvrcpCTypeNotSet));
	__ASSERT_DEBUG(iVendorId == KBluetoothSIGVendorId,  AvrcpUtils::Panic(EAvrcpCompanyIDNotSet));

	TInt ret = KErrNotSupported;
	TUint16 operation = MetadataTransferParser::GetPassThroughOperationId(iVendorPayloadData);
	if (operation == ENextGroup)
		{
		iOperationId = ENextGroup;
		iInterfaceUid = TUid::Uid(KRemConGroupNavigationApiUid);
		ret = KErrNone;
		}
	else if (operation == EPreviousGroup)
		{
		iOperationId = EPreviousGroup;
		iInterfaceUid = TUid::Uid(KRemConGroupNavigationApiUid);
		ret = KErrNone;
		}
	return ret;
	}

/* Before calling this method, the following MUST be set:
 *  - Vendor Payload data
 *  - Opcode       == Vendor Dependent
 *  - Subunit Type == Panel
 *  - Subunit Id   == 0
 */
 
TInt CControlCommand::ParseMetadataTransferVendorCommand(CAVRCPFragmenter& aFragmenter)
	{
	__ASSERT_DEBUG(iFrame->Opcode() == AVC::EVendorDependent, AvrcpUtils::Panic(EAvrcpNotFullyConstructed));
	__ASSERT_DEBUG(iFrame->SubunitType() == AVC::EPanel,      AvrcpUtils::Panic(EAvrcpNotFullyConstructed));
	__ASSERT_DEBUG(iFrame->SubunitID() == 0,                  AvrcpUtils::Panic(EAvrcpNotFullyConstructed));
	
	TInt ret = KErrNotSupported;
	iInterfaceUid = TUid::Uid(0);
	
	TMetadataTransferPDUID metadataPDUID = MetadataTransferParser::GetPDUID(iVendorPayloadData);
	
	TPtrC8 mtPayload(iVendorPayloadData.Mid(KRemConMetadataHeaderLength));
	
	TUint paramLength = MetadataTransferParser::GetParamLength(iVendorPayloadData);
	
	if (mtPayload.Length() != paramLength)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
		
	// If we're in a fragmented state, and we receive an un-expected
	// PDU (i.e. not CONTINUE or ABORT) then throw away the fragment
	// and just process the request. The specification (section 5.5.1)
	// isn't entirely clear as to what should happen, but Sian reckons
	// this is probably the most polite thing to do (be liberal, etc)
	if (aFragmenter.InFragmentedState())
		{
		if (   metadataPDUID != ERequestContinuingResponse
			&& metadataPDUID != EAbortContinuingResponse)
			{
			aFragmenter.Reset();
			}
		}
		
	switch (iFrame->Type())
		{
		case AVC::EControl:
			{
			switch (metadataPDUID)	
				{
				case ERequestContinuingResponse:
					ret = ParseContinuingResponse(mtPayload, aFragmenter);
					break;
				case EAbortContinuingResponse:
					ret = ParseAbortContinuingResponse(mtPayload, aFragmenter);
					break;
				case ESetPlayerApplicationSettingValue:
					ret = ParseSetPlayerApplicationSettingValue(mtPayload);
					break;
					
				case EInformBatteryStatusOfCT:
					ret = ParseInformBatteryStatusOfCT(mtPayload);
					break;
					
				case EInformDisplayableCharacterSet:
					ret = KErrNotSupported;
					break;
				case ESetAddressedPlayer:
					ret = ParseSetAddressedPlayer(mtPayload);
					break;
				case EPlayItem:
					ret = ParsePlayItem(mtPayload);
					break;
				case EAddToNowPlaying:
					ret = ParseAddToNowPlaying(mtPayload);
					break;
					
				case ESetAbsoluteVolume:
				    ret =ParseSetAbsoluteVolume(mtPayload);
				    break;
				default:
					// not allowed PDUID for Control
					ret = KErrAvrcpMetadataInvalidCommand;
				}
			break;
			}
		case AVC::EStatus:
			{
			switch (metadataPDUID)	
				{
				case EGetCapabilities:
					ret = ParseGetCapabilities( mtPayload );
					break;
					
				case EGetPlayStatus:
					ret = ParseGetPlayStatus( mtPayload );
					break;
					
				case EListPlayerApplicationSettingAttributes:
					ret = ParseListPlayerApplicationSettingAttributes( mtPayload );
					break;
					
				case EListPlayerApplicationSettingValues:
					ret = ParseListPlayerApplicationSettingValues( mtPayload );
					break;
					
				case EGetCurrentPlayerApplicationSettingValue:
					ret = ParseGetCurrentPlayerApplicationSettingValue( mtPayload );
					break;

				case EGetPlayerApplicationSettingAttributeText:
					ret = ParseGetPlayerApplicationSettingAttributeText( mtPayload );
					break;

				case EGetPlayerApplicationSettingValueText:
					ret = ParseGetPlayerApplicationSettingValueText( mtPayload );
					break;
					
				case EGetElementAttributes:
					ret = ParseGetElementAttributes(mtPayload );
					break;
					
				case ESetAddressedPlayer:
					ret = ParseSetAddressedPlayer(mtPayload );
					break;
				
				case EPlayItem:
					ret = ParsePlayItem(mtPayload );
					break;
					
				case EAddToNowPlaying:
					ret = ParseAddToNowPlaying(mtPayload );
					break;
				default:
					// not allowed PDUID for Status
					ret = KErrAvrcpMetadataInvalidCommand;
				}
				
			break;
			}
		case AVC::ENotify:
			{
			if (metadataPDUID==ERegisterNotification)
				{
				if (mtPayload.Length() != KLengthRegisterNotification)
					{
					ret = KErrAvrcpMetadataInvalidCommand;
					}
				else
					{
					ret = KErrNone;
					TRegisterNotificationEvent eventId = static_cast<TRegisterNotificationEvent>(mtPayload[KVendorDependentEventId]);
					switch (eventId)
						{
						case ERegisterNotificationPlaybackStatusChanged:
						case ERegisterNotificationTrackChanged:
						case ERegisterNotificationTrackReachedEnd:
						case ERegisterNotificationTrackReachedStart:
						case ERegisterNotificationBatteryStatusChanged:
						case ERegisterNotificationPlayerApplicationSettingChanged:
							{
							iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
							iOperationId = RAvrcpIPC::SetIPCOperationIdFromEventId(eventId);
							break;
							}
						// Note: ERegisterNotificationPlaybackPosChanged takes a 4 byte parameter
						case ERegisterNotificationPlaybackPosChanged:
							{
							iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
							iOperationId = RAvrcpIPC::SetIPCOperationIdFromEventId(eventId);
							iCommandData.Close();
							ret = iCommandData.Create(KLengthPlaybackPosChangedParam);
							if (ret == KErrNone)
								{
								iCommandData.Append(mtPayload.Right(KLengthPlaybackPosChangedParam));
								}
							else
								{
								ret = KErrAvrcpMetadataInternalError;
								}
							break;
							}
							
						// Note ERegisterNotificationSystemStatusChanged is not supported
						case ERegisterNotificationSystemStatusChanged_NotSupported:
							{
							ret = KErrNotSupported;
							break;
							}
							
						case ERegisterNotificationNowPlayingContentChanged:
							{
							iInterfaceUid = TUid::Uid(KRemConNowPlayingApiUid);
							iOperationId = RAvrcpIPC::SetIPCOperationIdFromEventId(eventId);
							break;
							}
						case ERegisterNotificationAvailablePlayersChanged:
						case ERegisterNotificationAddressedPlayerChanged:
						case ERegisterNotificationUidsChanged:
							{
							iInterfaceUid = TUid::Uid(KUidAvrcpInternalInterface);
							iOperationId = RAvrcpIPC::SetIPCOperationIdFromEventId(eventId);
							ret = KErrAvrcpInternalCommand;
							break;
							}
						case ERegisterNotificationVolumeChanged	:
							{
							ret = ParseVolumeChangedNotification(mtPayload);
							break;
							}
						default:
							{
							ret = KErrAvrcpMetadataInvalidParameter;
							}
						}
					}
				}
			else
				{
				ret = KErrAvrcpMetadataInvalidCommand;
				}
			break;
			}
		case AVC::EReserved1:
		case AVC::EReserved2:
		case AVC::EReserved3:
		case AVC::EReserved4:
			{
			ret = KErrAvrcpInvalidCType;
			break;
			}
		default:
			ret = KErrAvrcpMetadataInvalidCommand;
		}

	return ret;
	}

/** Allocate correct space and append the payload to iCommandData
 */
TInt CControlCommand::AppendIncomingPayload(const TPtrC8& aPayload)
	{
	iCommandData.Close();
	if (iCommandData.Create(aPayload.Length()) != KErrNone)
		{
		return KErrAvrcpMetadataInternalError;
		}
	iCommandData.Append(aPayload);
	return KErrNone;
	}

/** Decode a InformBatteryStatusOfCT PDU ID: 0x18 and call the BatteryStatus API 
 */
TInt CControlCommand::ParseInformBatteryStatusOfCT(TPtrC8& aMtPayload)
												 
	{
	if (aMtPayload.Length() != KLengthInformBatteryStatusOfCTPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	TUint8 batteryStatus = aMtPayload[0];
	if (batteryStatus > EFullCharge)
		{
		return KErrAvrcpMetadataInvalidParameter;
		}
	
	if (AppendIncomingPayload(aMtPayload) != KErrNone)
		{
		return KErrAvrcpMetadataInvalidParameter;
		}
	
	iOperationId = EInformBatteryStatusOfCT;
	iInterfaceUid = TUid::Uid(KRemConBatteryApiUid);
		
	TInt ret = RespondToInforms(EInformBatteryStatusOfCT);
	if (ret != KErrNone)
		{
		return ret;
		}
	
	return KErrAvrcpHandledInternallyInformRemCon;
	}


/** Decode PDU ID 0x40 - fragmentation support
 */
TInt CControlCommand::ParseContinuingResponse(TPtrC8& aMtPayload,
										  CAVRCPFragmenter& aFragmenter)
	{
	// Check if in fragmentation state, return error if not
	if (! aFragmenter.InFragmentedState())
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	// Check if the parameter matches the fragmented response
	TMetadataTransferPDUID pduId = MetadataTransferParser::GetPDUID(aMtPayload);
	if (pduId != aFragmenter.GetPDU())
		{
		return KErrAvrcpMetadataInvalidParameter;
		}
	
	RBuf8 respPayload;
	CAVCFrame* frame = NULL;
	TRAPD(err, frame = CAVCVendorDependentResponse::NewL(KBluetoothSIGVendorId));
	err = respPayload.Create(KAVCMaxVendorDependentPayload); //longest resp
	if (err == KErrNone)
		{
		respPayload.Append(aFragmenter.GetNextFragmentHeader());
		respPayload.Append(aFragmenter.GetNextFragment());
		frame->SetType(AVC::EStable);
		frame->Append(respPayload);
		delete iFrame;
		iFrame = frame;
		respPayload.Close();
		}
	else
		return KErrAvrcpMetadataInternalError;
	
	return KErrAvrcpHandledInternallyRespondNow;
	}


/** Decode PDU ID 0x41 - fragmentation support
 */
TInt CControlCommand::ParseAbortContinuingResponse(TPtrC8& aMtPayload,
												CAVRCPFragmenter& aFragmenter)
	{
	// Check if in fragmentation state, return error if not
	if (! aFragmenter.InFragmentedState())
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	// Check if the parameter matches the fragmented response
	TMetadataTransferPDUID pduId = MetadataTransferParser::GetPDUID(aMtPayload);
	if (pduId != aFragmenter.GetPDU())
		{
		return KErrAvrcpMetadataInvalidParameter;
		}
	
	aFragmenter.Reset();
	
	CAVCFrame* frame = NULL;
	TRAPD(err, frame = CAVCVendorDependentResponse::NewL(KBluetoothSIGVendorId));
	if (err == KErrNone)
		{
		frame->Append(EAbortContinuingResponse);
		frame->Append(EUnfragmented);
		// the package length is 0
		frame->Append( 0 );
		frame->Append( 0 );
		frame->SetType(AVC::EStable);
		delete iFrame;
		iFrame = frame;
		}
	else
		{
		return KErrAvrcpMetadataInternalError;
		}
	
	return KErrAvrcpHandledInternallyRespondNow;
	}


/** Decode a SetPlayerApplicationSettingValue PDU ID: 0x14 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseSetPlayerApplicationSettingValue(TPtrC8& aMtPayload)
	{
	if (aMtPayload.Length() < KMinLengthSetPASValuePdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
		
	// get the number of attributes contained 
	TUint8 numAttributes = aMtPayload[KVendorDependentNumberAttributes];
	
	// Each attribute is 16 bits long
	if (sizeof(numAttributes) + numAttributes*sizeof(TUint16) != aMtPayload.Length())
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	iOperationId = ESetPlayerApplicationSettingValue;
	iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
	return AppendIncomingPayload(aMtPayload);
	}

/** Decode a GetCapabilities PDU ID: 0x10 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseGetCapabilities(TPtrC8& aMtPayload)
	{
	// check enough data available
	if (aMtPayload.Length() != KLengthGetCapabilitiesPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	// pass one byte of data to Player Information API 
	iOperationId = EGetCapabilities;
	iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
	return AppendIncomingPayload(aMtPayload);
	}

/** Decode a GetPlayStatus PDU ID: 0x30 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseGetPlayStatus(TPtrC8& aMtPayload)
	{
	// no payload in this command
	if (aMtPayload.Length()!=KLengthGetPlayStatusPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	iOperationId = EGetPlayStatus;
	iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
	return KErrNone;
	}

/** Decode a ListPlayerApplicationSettingAttributes PDU ID: 0x11 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseListPlayerApplicationSettingAttributes(TPtrC8& aMtPayload)
	{
	// check length, there should be no parameters
	if (aMtPayload.Length()!=KLengthListPASAttributesPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}

	iOperationId = EListPlayerApplicationSettingAttributes;
	iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
	return KErrNone;
	}

/** Decode a ListPlayerApplicationSettingValues PDU ID: 0x12 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseListPlayerApplicationSettingValues(TPtrC8& aMtPayload)
	{
	// check length, there should be 1 byte of data
	if (aMtPayload.Length() != KLengthListPASValuesPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
		
	iOperationId = EListPlayerApplicationSettingValues;
	iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
	return AppendIncomingPayload(aMtPayload);
	}

/** Decode a GetCurrentPlayerApplicationSettingValue PDU ID: 0x13 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseGetCurrentPlayerApplicationSettingValue(TPtrC8& aMtPayload)
	{
	// check the length
	if (aMtPayload.Length() < KMinLengthGetCurrentPASValuePdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
		
	TUint8 numAttributes = aMtPayload[KVendorDependentNumberAttributes];
	if (sizeof(numAttributes) + numAttributes != aMtPayload.Length())
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	iOperationId = EGetCurrentPlayerApplicationSettingValue;
	iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
	return AppendIncomingPayload(aMtPayload);
	}

/** Decode a GetPlayerApplicationSettingAttributeText PDU ID: 0x15 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseGetPlayerApplicationSettingAttributeText(TPtrC8& aMtPayload)
	{
	if (aMtPayload.Length() < KMinLengthGetPASAttributeTextPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	// get the number of attributes contained 
	TUint8 numAttributes = aMtPayload[KVendorDependentNumberAttributes];
	if (sizeof(numAttributes) + numAttributes != aMtPayload.Length())
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	iOperationId = EGetPlayerApplicationSettingAttributeText;
	iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
	return AppendIncomingPayload(aMtPayload);
	}

/** Decode a GetPlayerApplicationSettingValueText PDU ID: 0x16 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseGetPlayerApplicationSettingValueText(TPtrC8& aMtPayload)
	{
	if (aMtPayload.Length() < KMinLengthGetPASValueTextPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	// get number of values; preceded by attributeId (1byte) and numValues (1byte)
	TUint8 numValues = aMtPayload[KVendorDependentNumberAttribsPdu16];
	if (sizeof(TUint8) + sizeof(numValues) + numValues != aMtPayload.Length())
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	iOperationId = EGetPlayerApplicationSettingValueText;
	iInterfaceUid = TUid::Uid(KRemConPlayerInformationUid);
	return AppendIncomingPayload(aMtPayload);
	}
	
/** Decode a GetElementAttributes PDU ID: 0x20 and call the PlayerInformation API 
 */
TInt CControlCommand::ParseGetElementAttributes(TPtrC8& aMtPayload)
	{
	LOG_FUNC
	
	if (aMtPayload.Length() < KMinLengthGetElementAttributesPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
		
	iOperationId = EGetElementAttributes;
	iInterfaceUid = TUid::Uid(KRemConMediaInformationApiUid);
	return AppendIncomingPayload(aMtPayload);
	}

TInt CControlCommand::ParseSetAddressedPlayer(TPtrC8& aMtPayload)
	{
	LOG_FUNC
	
	if (aMtPayload.Length() < KLengthSetAddressedPlayerPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}

	iOperationId = EAvrcpInternalSetAddressedPlayer;
	iInterfaceUid = TUid::Uid(KUidAvrcpInternalInterface);
	TInt err = AppendIncomingPayload(aMtPayload);

	return err == KErrNone ? KErrAvrcpInternalCommand : err;
	}

TInt CControlCommand::ParsePlayItem(TPtrC8& aMtPayload)
	{
	LOG_FUNC
	
	if (aMtPayload.Length() < KMinLengthAddToNowPlayingPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
		
	iOperationId = EPlayItem;
	iInterfaceUid = TUid::Uid(KRemConNowPlayingApiUid);
	return AppendIncomingPayload(aMtPayload);
	}

TInt CControlCommand::ParseAddToNowPlaying(TPtrC8& aMtPayload)
	{
	LOG_FUNC
	
	if (aMtPayload.Length() < KMinLengthAddToNowPlayingPdu)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
		
	iOperationId = EAddToNowPlaying;
	iInterfaceUid = TUid::Uid(KRemConNowPlayingApiUid);
	return AppendIncomingPayload(aMtPayload);
	}

TInt CControlCommand::ParseUidsChangedNotification(TPtrC8& /*aMtPayload*/)
	{
	LOG_FUNC
	iInterfaceUid = TUid::Uid(KRemConMediaBrowseApiUid);
	iOperationId = RAvrcpIPC::SetIPCOperationIdFromEventId(ERegisterNotificationUidsChanged);
	
	return KErrAvrcpHandledInternallyRespondNow;	
	}

TInt CControlCommand::ParseVolumeChangedNotification(TPtrC8& /*aMtPayload*/)
	{
	LOG_FUNC
	iInterfaceUid = TUid::Uid(KRemConAbsoluteVolumeTargetApiUid);
	iOperationId = KRemConAbsoluteVolumeNotification;
	
	return KErrNone;
	}

TInt CControlCommand::GenerateMetadataResponsePayload(MRemConBearerObserver& aObserver, RBuf8& aFramePayload, const RBuf8& aResponseData)
	{
	TInt err = KErrNone;
	
	// If it's a very large response; this requires re-allocating the buffer
	if (aResponseData.Length() > KAVCFrameMaxLength)
		{
		aFramePayload.Close();
		if (aFramePayload.Create(aResponseData.Length()) != KErrNone)
			return KErrAvrcpMetadataInternalError;
		}
	
	// Obtain the PDU from the combined PDU + (possible) notification eventId
	TMetadataTransferPDU pduId = RAvrcpIPC::GetPDUIdFromIPCOperationId(iOperationId);
	aFramePayload.Zero();
	aFramePayload.Append(pduId);
	aFramePayload.Append(EUnfragmented);

	//Check whether it is absolute volume response.
	TBool absoluteVolumeResponse = EFalse;
	if (pduId == ESetAbsoluteVolume)
		{
		absoluteVolumeResponse = ETrue;
		}
	else if (pduId == ERegisterNotification)
		{
		TRegisterNotificationEvent eventId = RAvrcpIPC::GetEventIdFromIPCOperationId(iOperationId);
		if (eventId == ERegisterNotificationVolumeChanged)
			{
			absoluteVolumeResponse = ETrue;
			}
		}
	
	TPtr8 responseData(NULL, 0);
	if (absoluteVolumeResponse)
		{
		responseData.Set(aResponseData.RightTPtr(aResponseData.Length()));
		}
	else
		{
		// Read 4 byte Big-Endian error code before the payload
		RAvrcpIPCError response;
		TRAP(err, response.ReadL(aResponseData));
		
		// If we couldn't parse the response via IPC, send an internal error
		if (err != KErrNone)
			{
			return KErrAvrcpMetadataInternalError;
			}
		
		// If an error occurred, return it (now that we've filled in the PDU id)
		if (response.iError != KErrNone)
			{
			return response.iError;
			}
		
		// Pass the rest of the response (minus error code) to be parsed
		responseData.Set(aResponseData.RightTPtr(aResponseData.Length() - KLengthErrorResponse));
		}
	
	switch (pduId)
		{
		case ESetPlayerApplicationSettingValue:
			{
			// the package length is 0
			aFramePayload.Append( 0 );
			aFramePayload.Append( 0 );
			break;
			}
		
		case EGetCapabilities:
			{
			TRAP(err,GenerateMetadataGetCapabilitiesResponsePayloadL(aObserver, aFramePayload, aResponseData));
			if(err != KErrNone)
				{
				return KErrAvrcpMetadataInternalError;
				}
			break;
			}
		case EListPlayerApplicationSettingAttributes:
		case EListPlayerApplicationSettingValues:
		case EGetCurrentPlayerApplicationSettingValue:
		case EGetPlayerApplicationSettingAttributeText:
		case EGetPlayerApplicationSettingValueText:
		case EGetElementAttributes:
		case EGetPlayStatus:
		case EPlayItem:
		case EAddToNowPlaying:
		case ESetAddressedPlayer:
			{
			// the package length is the response length
			TInt packageLength = responseData.Length();
			aFramePayload.Append(packageLength>>8);
			aFramePayload.Append(packageLength);
			aFramePayload.Append(responseData);
			break;
			}
		case ESetAbsoluteVolume:
			{			
			TRAPD(err, GenerateSetAbsoluteVolumeResponsePayloadL(aFramePayload,responseData));
			if (err != KErrNone)
				{
				return KErrAvrcpMetadataInternalError;
				}
		    break;
			}
		case ERegisterNotification:
			{		
			GenerateNotificationResponsePayload(aFramePayload, responseData);
			break;
			}		
		default:
			{
			return KErrNotSupported;
			}
		}

	// Success. Error conditions have been handled previously
	return KErrNone;
	}

void CControlCommand::GenerateSetAbsoluteVolumeResponsePayloadL(
		RBuf8& aFramePayload, 
		const TDesC8& responseData)
	{
	RRemConAbsoluteVolumeResponse response;
	CleanupClosePushL(response);
	
	response.ReadL(responseData);
	if (response.iError != KErrNone)
	    {
	    User::Leave(response.iError);
	    }
	TUint8 absVol = KAvrcpMaxAbsoluteVolume * response.iVolume / response.iMaxVolume;	
	TUint16 len = KLengthSetAbsoluteVolumeResponseParamter<<8 & 0xffff;
	TPckgBuf<TUint16> parameterLength(len);
	aFramePayload.Append(parameterLength);
	aFramePayload.Append( absVol );
	
	CleanupStack::PopAndDestroy(&response);
	}

void CControlCommand::DoGenerateNotifyVolumeChangeResponsePayloadL(
		RBuf8& aFramePayload, 
		const TDesC8& responseData)
	{
	RRemConAbsoluteVolumeResponse response;
	CleanupClosePushL(response);
	
	response.ReadL(responseData);
	if (response.iError != KErrNone)
	    {
	    User::Leave(response.iError);
	    }
	TUint8 absVol = KAvrcpMaxAbsoluteVolume * response.iVolume / response.iMaxVolume;
	TRegisterNotificationEvent eventId = RAvrcpIPC::GetEventIdFromIPCOperationId(iOperationId);
	TUint16 len = KLengthNotifyVolumeChangeResponseParameter<<8 & 0xffff;
	TPckgBuf<TUint16> parameterLength(len);
	aFramePayload.Append( parameterLength );
	aFramePayload.Append( eventId );
	aFramePayload.Append( absVol );
	
	CleanupStack::PopAndDestroy(&response);
	}

TInt CControlCommand::GenerateNotificationResponsePayload(RBuf8& aFramePayload, const TDesC8& responseData)
	{
	TInt err = KErrNone;
	
	TRegisterNotificationEvent eventId = RAvrcpIPC::GetEventIdFromIPCOperationId(iOperationId);
	switch(eventId)
		{
	case ERegisterNotificationVolumeChanged:
		{
		TRAPD(err, DoGenerateNotifyVolumeChangeResponsePayloadL(aFramePayload, responseData));
		if (err != KErrNone)
			{
			return KErrAvrcpMetadataInternalError;
			}
		break;
		}
	case ERegisterNotificationPlaybackStatusChanged:
	case ERegisterNotificationTrackChanged:
	case ERegisterNotificationTrackReachedEnd:
	case ERegisterNotificationTrackReachedStart:
	case ERegisterNotificationPlaybackPosChanged:
	case ERegisterNotificationBatteryStatusChanged:
	case ERegisterNotificationPlayerApplicationSettingChanged:
	case ERegisterNotificationNowPlayingContentChanged:
	case ERegisterNotificationAvailablePlayersChanged:
	case ERegisterNotificationAddressedPlayerChanged:
	case ERegisterNotificationUidsChanged:
		{
		TUint paramLength = responseData.Length() + 1;
		aFramePayload.Append( paramLength >>8 );
		aFramePayload.Append( paramLength );
		aFramePayload.Append( eventId );
		aFramePayload.Append( responseData );
		break;
		}
	default:
		{
		err = KErrNotSupported;
		break;
		}
		};

	return err;
	}

TInt CControlCommand::GenerateMetadataGetCapabilitiesResponsePayloadL(MRemConBearerObserver& /* aObserver */, RBuf8& aFramePayload, const RBuf8& aResponseData)
	{
	LOG_FUNC
	__ASSERT_DEBUG(iPlayerInfoManager, AVRCP_PANIC(EAvrcpNotFullyConstructed));
	TPtr8 responseData = aResponseData.RightTPtr(aResponseData.Length() - KLengthErrorResponse);

	if(responseData[KCapabilityIdOffset] == ECapabilityIdEventsSupported)
		{
		// Add supported events not handled in the player info API.
		RBuf8 eventsBuf;
		eventsBuf.CreateL(KNumberEventsNotInPlayerInfoApi);
		CleanupClosePushL(eventsBuf);
		TInt count = 0;
		
		// If a specific player (i.e. a specific client ID)  has been indicated then
		// we add support for the event if supported by the specific player.
		// If no specific player has been indicated (i.e. an invalid client ID), then
		// general support for the event is added if supported by any player.
		// The player info manager APIs handles these different "support" semantics.
		
		if(iPlayerInfoManager->AbsoluteVolumeSupportedL(ClientId()))
			{
			count++;
			eventsBuf.Append(ERegisterNotificationVolumeChanged );
			}
		
		if(iPlayerInfoManager->BrowsingSupportedL(ClientId()))
			{
			count += 2;
			eventsBuf.Append(ERegisterNotificationNowPlayingContentChanged );
			eventsBuf.Append(ERegisterNotificationUidsChanged );
			}
		
		// Always mark support for stuff that's handled internally rather than
		// by the player
		count+= 2;
		eventsBuf.Append(ERegisterNotificationAvailablePlayersChanged );
		eventsBuf.Append(ERegisterNotificationAddressedPlayerChanged );
		
		responseData[1] += count;

		TInt packageLength = responseData.Length()+ count;
		aFramePayload.Append(packageLength>>8);
		aFramePayload.Append(packageLength);
		aFramePayload.Append(responseData);
		aFramePayload.Append(eventsBuf);
		
		CleanupStack::PopAndDestroy(&eventsBuf);
		}
	else
		{
		TInt packageLength = responseData.Length();
		aFramePayload.Append(packageLength>>8);
		aFramePayload.Append(packageLength);
		aFramePayload.Append(responseData);
		}
	return KErrNone;
	}
TMetadataTransferPDUID MetadataTransferParser::GetNotifyEventID(const TPtrC8& aData)
	{
	return static_cast<TMetadataTransferNotifyEventID>(aData[KVendorDependentNotifyEventIdOffset]);
	}
	
TMetadataTransferPDUID MetadataTransferParser::GetPDUID(const TPtrC8& aData)
	{
	return static_cast<TMetadataTransferPDUID>(aData[KVendorDependentRequestPDUId]);
	}

TUint16 MetadataTransferParser::GetParamLength(const TPtrC8& aData)
	{
	TUint16 paramLength;
	paramLength  = aData[KVendorDependentRequestParamLenMSB] << 8;
	paramLength += aData[KVendorDependentRequestParamLenLSB];
	return paramLength;
	}

TUint16 MetadataTransferParser::GetPassThroughOperationId(const TPtrC8& aData)
	{
	TUint16 operation;
	operation  = aData[KPassThroughRequestOperationIdMSB] << 8;
	operation += aData[KPassThroughRequestOperationIdLSB];
	return operation;
	}

	
TInt CControlCommand::RespondToInforms(TMetadataTransferPDUID aMetadataPDUID)
	{
	CAVCFrame* frame = NULL;
	TRAPD(err, frame = CAVCVendorDependentResponse::NewL(KBluetoothSIGVendorId));
		
	RBuf8 respPayload;
	err = respPayload.Create(KAVRCPMinVendorDependentResponseLen);
	if (err == KErrNone)
		{
		respPayload.Append(aMetadataPDUID);
		respPayload.Append(EUnfragmented);   // No fragmentation needed
		respPayload.Append(0);	// no params
		respPayload.Append(0);	// no params
	
		frame->SetType(AVC::EAccepted);
		frame->Append(respPayload);
		
		delete iFrame;
		iFrame = frame;
		respPayload.Close();
		}
	else
		{
		err = KErrAvrcpMetadataInternalError;
		}
	return err;	
	}

void CControlCommand::GenerateMetadataRejectPayloadL(TInt aError)
	{
	CAVCFrame* frame = CAVCVendorDependentResponse::NewL(KBluetoothSIGVendorId);
	frame->SetType(AVC::ERejected);

	frame->Append(iFrame->Data()[6]); // PDU ID
	frame->Append(EUnfragmented);
	
	// Param length - 2 bytes
	frame->Append(0x0);
	frame->Append(0x1);

	TUint8 avcError = 0;
	switch (aError)
		{
		case KErrAvrcpMetadataInvalidCommand:
			{
			avcError = EInvalidCommand;
			break;
			}
		case KErrAvrcpMetadataInvalidParameter:
			{
			avcError = EInvalidParameter;
			break;
			}
		case KErrAvrcpMetadataParameterNotFound:
			{
			avcError = EParameterNotFound;
			break;
			}
		case KErrAvrcpMetadataInternalError:
			{
			avcError = EInternalError;
			break;
			}
		case KErrAvrcpAirInvalidCommand:
		case KErrAvrcpAirInvalidParameter:
		case KErrAvrcpAirParameterNotFound:
		case KErrAvrcpAirInternalError:
		case KErrAvrcpAirSuccess:
		case KErrAvrcpAirUidChanged:
		case KErrAvrcpAirReserved:
		case KErrAvrcpAirInvalidDirection:
		case KErrAvrcpAirNotADirectory:
		case KErrAvrcpAirDoesNotExist:
		case KErrAvrcpAirInvalidScope:
		case KErrAvrcpAirRangeOutOfBounds:
		case KErrAvrcpAirUidIsADirectory:
		case KErrAvrcpAirMediaInUse:
		case KErrAvrcpAirNowPlayingListFull:
		case KErrAvrcpAirSearchNotSupported:
		case KErrAvrcpAirSearchInProgress:
		case KErrAvrcpAirInvalidPlayerId:
		case KErrAvrcpAirPlayerNotBrowesable:
		case KErrAvrcpAirPlayerNotAddressed:
		case KErrAvrcpAirNoValidSearchResults:
		case KErrAvrcpAirNoAvailablePlayers:
		case KErrAvrcpAirAddressedPlayerChanged:
			{
			avcError = KErrAvrcpAirBase - aError;
			break;
			}
		}
	frame->Append(avcError);
	delete iFrame;
	iFrame = frame;
	}

/** Decode a SetAbsoluteVolume PDU ID: 0x50 and SetAbsoluteTarget call the  API 
 */
TInt CControlCommand::ParseSetAbsoluteVolume(TPtrC8& aMtPayload)
	{
	if (aMtPayload.Length() != KLengthSetAbsoluteVolumeRequestParameter)
		{
		return KErrAvrcpMetadataInvalidCommand;
		}
	
	iOperationId = KRemConSetAbsoluteVolume;
	iInterfaceUid = TUid::Uid(KRemConAbsoluteVolumeTargetApiUid);
	
	TBuf8<KAbsoluteVolumeRequestDataSize> payload;
	TRAPD(err, DoParseSetAbsoluteVolumeL(aMtPayload, payload));
	if (err != KErrNone)
		{
		return KErrAvrcpMetadataInternalError;
		}
	
	return AppendIncomingPayload(payload);
	}

void CControlCommand::DoParseSetAbsoluteVolumeL(const TPtrC8& aMtPayload, TDes8& aPayload)
	{
	RRemConAbsoluteVolumeRequest request;
	CleanupClosePushL(request);
	request.iVolume = KAbsoluteVolumeMask & aMtPayload[KLengthSetAbsoluteVolumeRequestParameter - 1];
	request.iMaxVolume = KAvrcpMaxAbsoluteVolume;
	request.WriteL(aPayload);
	CleanupStack::PopAndDestroy(&request);
	}
