/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Remote connection observer interface declaration.
*
*/


#ifndef BTASBAUDIOSERVICEPLUGIN_H
#define BTASBAUDIOSERVICEPLUGIN_H

//  INCLUDES
#include <e32base.h>
#include <remcon/remconbearerplugin.h>
#include <remcon/remconbearerinterface.h>
#include <remcon/messagetype.h>
#include <remconaddress.h>
#include <bttypes.h>
#include "atcodec.h"
#include "playercontrolistener.h"

// FORWARD DECLARATIONS

class CBmbCmdListener;

/**
*  RemCon bearer plugin implementation for BT Audio Service.
*/
class CBmbPlugin 
:   public CRemConBearerPlugin,
    public MRemConBearerInterface,
    public MBmbPlayerControl
	{
    public:
        /**
        * Two-phased constructor.
        * @param aParams   
        */
    	static CBmbPlugin* NewL(TBearerParams& aParams);
    	~CBmbPlugin();

        void DataFromRemote(const TDesC8& aATCmd);


    private: // from CRemConBearerPlugin
    
    	/**
    	* Called by RemCon server to get a pointer to an object which implements the 
    	* bearer API with UID aUid. This is a mechanism for allowing future change 
    	* to the bearer API without breaking BC in existing (non-updated) bearer 
    	* plugins.
    	* @return the pointer of this implementation
    	*/    
	    TAny* GetInterface(TUid aUid);
        
    private: // from MRemConBearerInterface
    
    	/** 
    	* Called by RemCon to retrieve a response on a connection. Must only be 
    	* called as a result of a NewResponse upcall.
    	* @param aInterfaceUid The UID of the outer-layer client API specifying the 
    	*         response.
    	* @param aId The command identifier used as a cookie for command/response 
    	*        matching.
    	* @param aOperationId The ID of the response operation in the outer-layer 
    	*        client API.
    	* @param aData API-specific message data. On success, ownership is 
    	*        returned.
    	* @param aAddr The connection.
    	*/    
    	TInt GetResponse(
    	    TUid& aInterfaceUid, 
    		TUint& aTransactionId, 
    		TUint& aOperationId, 
    		RBuf8& aData, 
    		TRemConAddress& aAddr);

    	/** 
    	* Called by RemCon to send a command on a connection. Must only be 
    	* called as a result of a NewCommand upcall.
    	* @param aInterfaceUid The UID of the outer-layer client API specifying the 
    	*        command.
    	* @param aId The command identifier used as a cookie for command/response 
    	*        matching.
    	* @param aOperationId The ID of the command operation in the outer-layer 
    	*        client API.
    	* @param aData API-specific message data. On success, ownership is 
    	*        returned.
    	* @param aAddr The connection.
    	*/    		
    	TInt SendCommand(
    	    TUid aInterfaceUid, 
    		TUint aCommand, 
    		TUint aTransactionId,  
    		RBuf8& aData, 
    		const TRemConAddress& aAddr);

    	/** 
    	* Called by RemCon to retrieve a command on a connection. The connection is not 
    	* assumed to exist- the bearer is responsible for bringing up the requested 
    	* connection if necessary.
    	* @param aInterfaceUid The UID of the outer-layer client API specifying the 
    	*        command.
    	* @param aOperationId The ID of the command operation in the outer-layer 
    	*        client API.
    	* @param aId The command identifier used as a cookie for command/response 
    	*        matching.
    	* @param aData API-specific message data. On success, ownership is passed. 
    	* @param aAddr The connection.
    	* @return Error. This request is synchronous. It should be completed by the 
    	* bearer when it has taken responsibility for sending the message. This will 
    	* involve checking that the message is well-formed, and possibly actually 
    	* trying to send it, or adding it to a queue.
    	*/    		
    	TInt GetCommand(
    	    TUid& aInterfaceUid, 
    		TUint& aTransactionId, 
    		TUint& aCommand, 
    		RBuf8& aData, 
    		TRemConAddress& aAddr);

    	/** 
    	* Called by RemCon to send a response on a connection. The connection is not 
    	* assumed to exist- the bearer is responsible for bringing up the requested 
    	* connection if necessary.
    	* @param aInterfaceUid The UID of the outer-layer client API specifying the 
    	*        response.
    	* @param aOperationId The ID of the response operation in the outer-layer 
    	*       client API.
    	* @param aId The command identifier used as a cookie for command/response 
    	*        matching
    	* @param aData API-specific message data. On success, ownership is passed.
    	* @param aAddr The connection.
    	* @return Error. This request is synchronous. It should be completed by the 
    	* bearer when it has taken responsibility for sending the message. This will 
    	* involve checking that the message is well-formed, and possibly actually 
    	* trying to send it, or adding it to a queue.
    	*/    		
    	TInt SendResponse(
    	    TUid aInterfaceUid, 
    		TUint aOperationId, 
    		TUint aTransactionId, 
    		RBuf8& aData, 
    		const TRemConAddress& aAddr);

    	/** 
    	* Called by RemCon to establish a bearer-level connection to another party. 
    	* Completion is signalled back in ConnectConfirm.
    	* @param aAddr The RemCon address to connect to.
    	*/    		
    	void ConnectRequest(const TRemConAddress& aAddr);
    	
    	/** 
    	* Called by RemCon to destroy a bearer-level connection to another party. 
    	* Completion is signalled back in DisconnectConfirm.
    	* @param aAddr The RemCon address to disconnect from.
    	*/    	
    	void DisconnectRequest(const TRemConAddress& aAddr);
    	
    	/**
    	* Called by RemCon when either (a) the number of controller clients changes 
    	* from 0 to 1 or from 1 to 0, or (b) the number of target clients changes 
    	* from 0 to 1 or from 1 to 0.
    	*/    	
    	void ClientStatus(TBool aControllerPresent, TBool aTargetPresent);

    	/**
    	* Called by RemCon to get the capabilities required to make/destroy 
    	* connections over the bearer, and to send and receive messages over the 
    	* bearer.
    	@return The bearer's security policy.
    	*/    	
    	TSecurityPolicy SecurityPolicy() const;        
               
    private:
        // From MBmbPlayerControl
        void Pause();
        void Play();
            
    private:
        
        /**
        * C++ default constructor.
        */
    	CBmbPlugin(TBearerParams& aParams);

        /**
        * Symbian 2nd phase constructor.
        */    	
    	void ConstructL();

    private: // owned

    	// Last received and corrected-decoded message.
		TBuf8<KDefaultCmdBufLength> iInData;       
    	TUid iInterfaceUid;
    	TUint iTransactionId;
    	TUint iOperationId;
    	TRemConMessageType iMsgType;
        TRemConAddress iRemConAddress;
        CBTAudioPlayerControlListener *iPlayerControlListener;       
        CBmbCmdListener *iListener; 

        TBool iConnIndicated;
	};


#endif // BTASBAUDIOSERVICEPLUGIN_H
