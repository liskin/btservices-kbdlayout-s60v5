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

#ifndef CONTROLBEARER_H
#define CONTROLBEARER_H

/**
@file
@internalComponent
@released
*/

#include <avctpservices.h>
#include <btsdp.h>
#include <e32std.h>
#include <remcon/clientid.h>
#include <remcon/remconbearerinterface.h>
#include <remcon/remconbearerplugin.h>
#include <remcon/avrcpspec.h>
#include <remcon/messagetype.h>
#include <remcon/playertype.h>
#include "avrcpbearerinterface.h"
#include "remconcommandinterface.h"

class CControlCommand;
class CRcpRemoteDevice;
class CControlRouter;
class TBTDevAddr;
class CBrowseCommand;
class CAvrcpPlayerInfoManager;
class CAvrcpBulkBearer;

NONSHARABLE_CLASS(CRemConBearerAvrcp) : public CRemConBearerPlugin, public MRemConBearerInterfaceV3,
	public MAvrcpBearer, public MRemConControlCommandInterface
	{
public:
	static CRemConBearerAvrcp* NewL(TBearerParams& aParams);	
	virtual ~CRemConBearerAvrcp();	
		
public:	// Interface to command handlers, from MRemConCommandInterface
	void MrcciNewCommand(CAvrcpCommand& aCommand);
	void MrcciNewCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId);
	void MrccciNewNotifyCommand(CAvrcpCommand& aCommand);
	void MrccciNewNotifyCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId);

	void MrccciNewResponse(CAvrcpCommand& aCommand);
	void MrccciNewNotifyResponse(CControlCommand& aCommand);
	
	TUint MrcciNewTransactionId();
    void MrcciCommandExpired(TUint aTransactionId);
    
	void MrccciSetAddressedClient(const TRemConClientId& aClient);
	void MrccciRegisterForLocalAddressedClientUpdates();
	void MrccciUnregisterForLocalAddressedClientUpdates();

private: // MRemConBearerInterfaceV3 functions called from RemCon
	virtual TInt GetResponse(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr);
			
	virtual TInt SendCommand(TUid aInterfaceUid, 
		TUint aOperationId, 
		TUint aId,  
		RBuf8& aData, 
		const TRemConAddress& aAddr);
		
	virtual TInt GetCommand(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr);	
	
	virtual TInt GetNotifyCommand(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr);	
			
	virtual CAvrcpCommand* GetFirstCommand(TDblQue<CAvrcpCommand>& aQue,
		TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr);
		
	virtual TInt SendResponse(TUid aInterfaceUid, 
		TUint aOperationId, 
		TUint aId, 
		RBuf8& aData, 
		const TRemConAddress& aAddr);
	
	virtual void SendReject(TUid aInterfaceUid, 
			TUint aOperationId, 
			TUint aTransactionId, 
			const TRemConAddress& aAddr); 
	virtual void ConnectRequest(const TRemConAddress& aAddr);
	virtual void DisconnectRequest(const TRemConAddress& aAddr);	
	
	virtual TSecurityPolicy SecurityPolicy() const;
	void ClientStatus(TBool aControllerPresent, TBool aTargetPresent);	
	virtual void ClientAvailable(TRemConClientId& aId, TPlayerType aClientType, TPlayerSubType aClientSubType, const TDesC8& aName);
	virtual void ClientNotAvailable(TRemConClientId& aId);
	virtual void ControllerFeaturesUpdated(RArray<TUid>& aSupportedInterfaces);
	
	virtual TInt SetLocalAddressedClient(TRemConClientId& aId);
	
	virtual void TargetFeaturesUpdated(const TRemConClientId& aId, TPlayerType aPlayerType, TPlayerSubType aPlayerSubType, const TDesC8& aName);
	
private: // from CRemConBearerPlugin
	TAny* GetInterface(TUid aUid);
	
private:	
	// from MAvrcpBearer called from router
	void ConnectIndicate(const TBTDevAddr& aBTDevice);
	void ConnectConfirm(const TBTDevAddr& aBTDevice, TInt aError);
	void DisconnectIndicate(const TBTDevAddr& aBTDevice);
	void DisconnectConfirm(const TBTDevAddr& aBTDevice, TInt aError);
	
	MIncomingCommandHandler* IncomingHandler(const TBTDevAddr& aAddr);
	MOutgoingCommandHandler* OutgoingHandler(const TBTDevAddr& aAddr);
	
private:
	// MRemConBearerInterfaceV3 plugin functions called from RemCon
	virtual TInt SendNotifyCommand(TUid aInterfaceUid, 
			TUint aOperationId, 
			TUint aId,  
			RBuf8& aData, 
			const TRemConAddress& aAddr);
	
	virtual TInt GetNotifyResponse(TUid& aInterfaceUid, 
			TUint& aId, 
			TUint& aOperationId, 
			RBuf8& aCommandData, 
			TRemConAddress& aAddr,
			TRemConMessageSubType& aSubMessageType);

private:
	enum TAvrcpRecordType
		{
		EAvrcpRemoteControlRecord,
		EAvrcpRemoteControlTargetRecord,
		};

private:
	CRemConBearerAvrcp(TBearerParams& aParams);
	void ConstructL();
	
	// utility functions
	CRcpRemoteDevice* RemoteDevice(const TBTDevAddr& aAddr);
	
	void UpdateServiceRecordL(RSdpDatabase& aSdpDatabase,
		TBool aController,
		TBool aTarget);
	void RegisterServiceRecordL(RSdpDatabase& aSdpDatabase,
		TAvrcpRecordType aType);
	
	void HandleUndeliveredCommand(CAvrcpCommand& aCommand, const TRemConAddress& aAddr);
	
private:
	TDblQue<CRcpRemoteDevice>	iRemotes;
	TDblQue<CAvrcpCommand>		iReadyCommands;
	TDblQue<CAvrcpCommand>		iReadyResponses;
	TDblQue<CControlCommand>	iReadyNotifyResponses;
	TDblQue<CAvrcpCommand>		iReadyNotifyCommands;
	CControlRouter*				iRouter;
	CDeltaTimer*				iTimer;
	RSdp						iSdp;
	TSdpServRecordHandle		iControllerSdpRecordHandle;
	TSdpServRecordHandle		iTargetSdpRecordHandle;
	TBool						iConstructionComplete;
	CAvrcpPlayerInfoManager*	iPlayerInfoManager;
	MIncomingCommandHandler*	iInternalHandler;
	
	CAvrcpBulkBearer*			iBulkBearer;
	
	RAvctp						iAvctp;
	
	TInt						iRemotesInterestedInLocalAddressedClient;
	};

#endif //CONTROLBEARER_H
