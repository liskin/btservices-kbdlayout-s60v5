// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CONTROLCOMMAND_H
#define CONTROLCOMMAND_H

/**
@file
@internalComponent
@released
*/

#include <avctpservices.h>
#include <e32base.h>
#include <remconcoreapi.h>
#include <remcon/remconbearerplugin.h>
#include <remcon/clientid.h>
#include "internaltypes.h"

#include <avcframe.h>

#include "avrcpcommand.h"
#include "avrcptimer.h"
#include "avrcpfragmenter.h"

const TInt KRcpResponseTimeOut = 30000000;	// Allow remote 300ms to respond
const TInt KRcpHoldThreshold = 300000;		// After 300ms we may assume an unreleased button 
											// action is a press and hold
const TInt KRcpIncomingButtonReleaseTimeout = 1700000; // 1.7s (ie 2s - 300ms), if a button press 
											// has not been released after 2s we assume the release
											// has been lost (and generate one internally)	
const TInt KRcpOutgoingButtonReleaseTimeout = 1000000; // The remote expects a press refresh within
											// 2s, it's fine to send one before that.

const TUint KRemConStatusApiDefaultPageDataLength = 4;
const TUint KRemConMetadataHeaderLength = 4;

// Absolute lengths of Vendor Dependent PDU requests
const TUint8 KLengthGetCapabilitiesPdu = 1;          // PDU 0x10
const TUint8 KLengthListPASAttributesPdu = 0;        // PDU 0x11
const TUint8 KLengthListPASValuesPdu = 1;            // PDU 0x12
const TUint8 KLengthInformBatteryStatusOfCTPdu = 1;  // PDU 0x18
const TUint8 KLengthGetPlayStatusPdu = 0;            // PDU 0x30
const TUint8 KLengthRegisterNotification = 5;        // PDU 0x31
const TUint8 KLengthSetAddressedPlayerPdu = 2;		 // PDU 0x60

// Minimum lengths of Vendor Dependent PDU requests
const TUint8 KMinLengthGetCurrentPASValuePdu = 2;    // PDU 0x13
const TUint8 KMinLengthSetPASValuePdu = 1;           // PDU 0x14
const TUint8 KMinLengthGetPASAttributeTextPdu = 2;   // PDU 0x15
const TUint8 KMinLengthGetPASValueTextPdu = 3;       // PDU 0x16
const TUint8 KMinLengthGetElementAttributesPdu = 9;  // PDU 0x20
const TUint8 KMinLengthAddToNowPlayingPdu = 11;      // PDU 0x90

// Other lengths
const TUint8 KLengthPlaybackPosChangedParam = 4;     // Parameter length for this event (section 5.4.2)
const TUint8 KVendorDependentNumberAttributes = 0;   // Index to Number of attributes (the first byte)
const TUint8 KVendorDependentNumberAttribsPdu16 = 1; // Index to Number of attributes for PDU 0x16 (the second byte)
const TUint8 KVendorDependentEventId = 0;            // Index to Event Id (the first byte)

enum TMetadataTransferErrorCodes
	{
	EInvalidCommand		= 0x0,
	EInvalidParameter	= 0x1,
	EParameterNotFound	= 0x2,
	EInternalError		= 0x3,
	};

enum TMetaDataTransferPacketType
	{
	EUnfragmented		= 0x0,
	EFragmentStart		= 0x1,
	EFragmentContinue	= 0x2,
	EFragmentEnd		= 0x3,
	};

// Format of the first 3 bytes of a Vendor Dependent Request
// See example packets in AVRCP Specification Appendix I (page 84)
const TUint8 KVendorDependentRequestPDUId        = 0;
const TUint8 KVendorDependentRequestReservedByte = 1;
const TUint8 KVendorDependentRequestParamLenMSB  = 2;
const TUint8 KVendorDependentRequestParamLenLSB  = 3;

//See example packets in AVRCP Specification Appendix D (Volume Change Notification)
const TUint8 KVendorDependentNotifyEventIdOffset = 4;

// Format of the first 2 bytes of a Pass Through Request
// See example packets in AVRCP Specification Appendix I (page 89)
const TUint8 KPassThroughRequestOperationIdMSB  = 0;
const TUint8 KPassThroughRequestOperationIdLSB  = 1;

const TInt KSetAddressedPlayerPlayerIdOffset = 0;

const TInt KCapabilityIdOffset = 0;

const TInt KNumberEventsNotInPlayerInfoApi = 5;

typedef TUint8 TMetadataTransferPDUID;
typedef TUint8 TMetadataTransferNotifyEventID;

class CRcpRemoteDevice;
class CAVRCPFragmenter;
class CRcpIncomingCommandHandler;
class MRemConBearerObserver;
class CAvrcpPlayerInfoManager;
class CAvrcpPlayerInfoManager;
NONSHARABLE_CLASS(CControlCommand) : public CAvrcpCommand
	{
public:
	static CControlCommand* NewL(TUid aInterfaceUid,
		TUint aCommand, 
		TUint aRemConId, 
		SymbianAvctp::TTransactionLabel aTransactionLabel, 
		RBuf8& aCommandData, 
		TBool aIsClick,
		const TBTDevAddr& aAddr,
		TBool aKnownToBearer);
		
	static CControlCommand* NewL(CAVCFrame* aFrame, 
		TUint aRemConId,
		SymbianAvctp::TTransactionLabel aTransLabel, 
		const TBTDevAddr& aAddr,
		const TRemConClientId& aClientId,
		CAvrcpPlayerInfoManager* aPlayerInfoManager);
	
	SymbianAvctp::TMessageType MessageType() const;
	const TDesC8& Data() const; 
	
	// Called from bearer
	const TRemConClientId& ClientId() const;
		
	// Called from handlers
	TInt ParseIncomingCommandL(MRemConBearerObserver& aObserver, CAVRCPFragmenter& aFragmenter);
	void ProcessOutgoingCommandL(MRemConBearerObserver& aObserver);
	TInt ParseIncomingResponse(MRemConBearerObserver& aObserver, const CAVCFrame& aFrame);
	TInt ProcessOutgoingResponse(MRemConBearerObserver& aObserver, RBuf8& aCommandData, CAVRCPFragmenter& aFragmenter);
	void SetResponseType(TInt aErr);
	
	TDeltaTimerEntry* TimerEntry();
	TAvrcpTimerExpiryInfo* TimerExpiryInfo();
	void CancelTimer(CDeltaTimer& aTimer);
	
	const CAVCFrame& Frame() const;
	AVCPanel::TButtonAction ButtonAct() const;
	TBool Click() const;
	void SetClick(TBool aIsClick);
	void ReSetCoreButtonActionL(TRemConCoreApiButtonAction aButtonAct, TBool aCommand);

	void SetCoreButtonAction(TRemConCoreApiButtonAction aButtonAct, TBool aCommand);
	TInt InsertCoreResult(TInt aResult);
	TBool IsAvrcpPassthrough() const;
	TBool IsPassthrough() const;
	TInt SetSetAbsoluteVolumeResult(const CAVCFrame& aFrame);
	TInt SetNotifyVolumeChangeResult(const CAVCFrame& aFrame);
	TBool PlayerSpecificNotify() const;
	TBool NormalCommand();
	
	CControlCommand* InterimResponseL();
	
private:
	CControlCommand(TUid aInterfaceUid,
		TUint aCommand,
		TUint aRemConId, 
		SymbianAvctp::TTransactionLabel aTransactionLabel, 
		RBuf8& aCommandData, 
		TBool aIsClick,
		const TBTDevAddr& aAddr,
		TBool aKnownToBearer);

	CControlCommand(CAVCFrame* aFrame,
		TUint aRemConId,
		SymbianAvctp::TTransactionLabel aTransLabel, 
		const TBTDevAddr& aAddr,
		const TRemConClientId& aClientId,
		CAvrcpPlayerInfoManager* aPlayerInfoManager);

	void ConstructL();
	~CControlCommand();
	
	TInt ParseIncomingKnownOpcodeL(MRemConBearerObserver& aObserver);
	TInt ParseIncomingVendorCommandL(MRemConBearerObserver& aObserver, CAVRCPFragmenter& aFragmenter);

	void ParseIncomingUnknownResponse(MRemConBearerObserver& aObserver, const CAVCFrame& aFrame);

	TInt ParseInformBatteryStatusOfCT( TPtrC8& mtPayload);
													 
	TInt ParseContinuingResponse(TPtrC8& mtPayload,
									CAVRCPFragmenter& aFragmenter);
										
	TInt ParseAbortContinuingResponse(TPtrC8& mtPayload,
									CAVRCPFragmenter& aFragmenter);
										
	TInt ParseSetPlayerApplicationSettingValue(TPtrC8& mtPayload);
	TInt ParseGetCapabilities(TPtrC8& aMtPayload);
	TInt ParseGetPlayStatus(TPtrC8& aMtPayload);
	TInt ParseListPlayerApplicationSettingAttributes(TPtrC8& aMtPayload);
	TInt ParseListPlayerApplicationSettingValues(TPtrC8& aMtPayload);
	TInt ParseGetCurrentPlayerApplicationSettingValue(TPtrC8& aMtPayload);
	TInt ParseGetPlayerApplicationSettingAttributeText(TPtrC8& aMtPayload);
	TInt ParseGetPlayerApplicationSettingValueText(TPtrC8& aMtPayload);
	TInt ParseGetElementAttributes(TPtrC8& aMtPayload);
	TInt ParseSetAddressedPlayer(TPtrC8& aMtPayload);
	TInt ParsePlayItem(TPtrC8& aMtPayload);
	TInt ParseAddToNowPlaying(TPtrC8& aMtPayload);
	TInt ParseUidsChangedNotification(TPtrC8& aMtPayload);
	TInt ParseInternalUidsChangedNotification(TPtrC8& aMtPayload);
	TInt ParseVolumeChangedNotification(TPtrC8& aMtPayload);
	TInt ParseSetAbsoluteVolume(TPtrC8& aMtPayload);
	void DoParseSetAbsoluteVolumeL(const TPtrC8& aMtPayload, TDes8& aPayload);

	TInt ParseMetadataTransferVendorCommand(CAVRCPFragmenter& aFragmenter);
	TInt ParseVendorUniquePassthroughCommand(MRemConBearerObserver& aObserver);
	TInt ParseMetadataTransferPassthroughCommand();
	TInt GenerateMetadataResponsePayload(MRemConBearerObserver& aObserver, RBuf8& aFramePayload, const RBuf8& aResponseData);
	TInt GenerateMetadataGetCapabilitiesResponsePayloadL(MRemConBearerObserver& aObserver, RBuf8& aFramePayload, const RBuf8& aResponseData);
	void GenerateSetAbsoluteVolumeResponsePayloadL(RBuf8& aFramePayload, const TDesC8& responseData);
	void DoGenerateNotifyVolumeChangeResponsePayloadL(RBuf8& aFramePayload, const TDesC8& responseData);
	TInt GenerateNotificationResponsePayload(RBuf8& aFramePayload, const TDesC8& aResponseData);
	void GenerateMetadataRejectPayloadL(TInt aError);
	void SetVendorInfoL(TBool aIsPassthrough);

	void DoProcessOutgoingResponseL(MRemConBearerObserver& aObserver, RBuf8& aCommandData,
									CAVRCPFragmenter& aFragmenter);

	TUint8 GetPDUIdFromIPCOperationId(TInt aOperationId);
	TUint8 GetEventIdFromIPCOperationId(TInt aOperationId);

	TInt RespondToInforms(TMetadataTransferPDUID aMetadataPDUID);
	TInt AppendIncomingPayload(const TPtrC8& aPayload);
	
	static TInt AvrcpToRemConOperation(TUint aAvrcpOp, TUint& aRemConOp, TUid& aRemConIf);
	static TInt RemConToAvrcpOperation(TUint aRemConOp, AVCPanel::TOperationId& aAvrcpOp);
	
	TUint16 Get16(const TPtrC8& aPtr);
	
	static TInt DummyCallback(TAny*);

	void DoSetAbsoluteVolumeResultL(const CAVCFrame& aFrame);
	void DoSetNotifyVolumeChangeResultL(const CAVCFrame& aFrame);
	
private:
	TBool							iIsClick;
	
	TDeltaTimerEntry*				iTimerEntry;
	TAvrcpTimerExpiryInfo*			iTimerExpiryInfo;
	
	CAVCFrame*						iFrame;
	AVC::TAVCVendorId				iVendorId;
	TPtrC8							iVendorPayloadData; // ptr to the vendor data in iFrame;
	
	TRemConClientId					iClientId;
	};

class MetadataTransferParser
	{
public:
    static AVC::TAVCVendorId GetVID(const CAVCFrame& aFrame); // could be elsewhere really!
	static TMetadataTransferPDUID GetPDUID(const TPtrC8& aData);
	static TUint16 GetParamLength(const TPtrC8& aData);
	static TUint16 GetPassThroughOperationId(const TPtrC8& aData);
	static TMetadataTransferPDUID GetNotifyEventID(const TPtrC8& aData);
	};

	
#endif // CONTROLCOMMAND_H
