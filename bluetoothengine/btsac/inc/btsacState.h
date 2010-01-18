/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The base state declaration
*
*/


#ifndef C_BTSACSTATE_H
#define C_BTSACSTATE_H

#include <e32base.h>
#include <es_sock.h>
#include <bttypes.h>
#include "btsacGavdpObserver.h"  // MInternalGavdpUser
#include "btsacActiveObserver.h"
#include "btsaController.h"

enum TStateIndex
	{
	EStateIdle,
	EStateListening,
	EStateConnecting,
	EStateConfiguring,
	EStateConnected,
	EStateConfigured,
	EStateStreaming,
	EStateSuspended,
	EStateAborting
	};

/**
 *  the base class of the state machine
 *
 *  @since S60 v3.1
 */
class CBtsacState : public CBase, public MInternalGavdpUser
    {
public:

    virtual ~CBtsacState();
    
    /**
     * Entry of this state.
     *
     * @since S60 v3.1
     */
    virtual void EnterL() = 0;    
    
    /**
     * Returns the next state if EnterL leaves.
     *
     * @since S60 v3.1
     * @return the next state
     */
    virtual CBtsacState* ErrorOnEntryL(TInt aReason);

    /** 
     * Connect to BT accessory 
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    virtual void ConnectL(const TBTDevAddr& aAddr);

    /** 
     * Cancel the outstanding connecting operation
     *
     * @since S60 v3.1
     */
    virtual void CancelConnectL();

    /** 
     * disconnect the connected accessory
     *
     * @since S60 v3.1
     * @param aStatus On completion, will contain an error code
     */
    virtual void DisconnectL();

    /** 
     * Open audio link (synchronous) to the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    virtual void OpenAudioLinkL(const TBTDevAddr& aAddr);

    /** 
     * Cancel the outstanding operation
     *
     * @since S60 v3.1
     */
    virtual void CancelOpenAudioLinkL();

    /** 
     * Close the audio link (synchronous) to the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    virtual void CloseAudioLinkL(const TBTDevAddr& aAddr);
    
    /** 
     * Cancel the outstanding operation
     *
     * @since S60 v3.1
     */
    virtual void CancelCloseAudioLink(const TBTDevAddr& aAddr);
    
    /**
     * 
     * @return an error. Default implementation returns KErrNotFound
     */
 	virtual void Configure();

    /**
    * Called when the connection is disconnected from remote side or there
    * is error on reading/writing data packet.
    * @param aErr the error code of event
    */
    virtual void DisconnectedFromRemoteOrError(TInt aErr);
    
    /**
    * BTAccServer informs BTSAC to start streaming after getting permission
    * from Acc Fw
    */
    virtual void StartRecording();
    
    /**
    * Default for handling gavdp errors
    */ 
    virtual void HandleGavdpErrorL(TInt aError);
    
    /**
    * Gets the index of the state
    */ 
    TStateIndex GetStateIndex() const;

   /*
	* Defaults from MInternalGavdpUser @see btsacGavdpObserver.h for definitions
	*/
	
public:
		
	virtual void GAVDP_ConnectConfirm(const TBTDevAddr& aDeviceAddr);	
	virtual void GAVDP_SEPDiscovered(const TAvdtpSEPInfo& aSEP);
	virtual void GAVDP_SEPDiscoveryComplete();	
	virtual void GAVDP_SEPCapability(TAvdtpServiceCapability* aCapability); // transfers ownership
	virtual void GAVDP_SEPCapabilityComplete();		
	virtual void GAVDP_AbortStreamConfirm();
	virtual void GAVDP_StartStreamsConfirm();	
	virtual void GAVDP_SuspendStreamsConfirm();	
	virtual void GAVDP_SecurityControlConfirm(const TDesC8& aResponseData);
	virtual void GAVDP_ConfigurationConfirm();	// configuration complete and SEP selected *AND* reconfigure confirm
	virtual void GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID);	
	virtual TInt GAVDP_ConfigurationIndication(TAvdtpServiceCapability* aCapability);
	virtual TInt GAVDP_ConfigurationEndIndication();
	virtual TInt GAVDP_StartIndication(TSEID aSEID);
	virtual TInt GAVDP_SuspendIndication(TSEID aSEID);
	virtual TInt GAVDP_SecurityControlIndication(TSEID aSEID, TDes8& aSecurityDataInOut);
	virtual void GAVDP_AbortIndication(TSEID aSEID);	
	virtual void GAVDP_ReleaseIndication(TSEID aSEID);		
	virtual void GAVDP_BearerReady(RSocket aSocket, const TAvdtpSockAddr& aAddress);
	virtual void GAVDP_Error(TInt aError, const TDesC8& aErrorData);

   	/*
   	 * Defaults from MBtsacActiveObserver
	 */	
	/**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
	virtual void RequestCompletedL(CBtsacActive& aActive);

    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
	virtual void CancelRequest(CBtsacActive& aActive);
	
protected:

    CBtsacState(CBTSAController& aParent, TStateIndex aStateIndex);

    /**
     * Gets the owner of the state machine
     *
     * @since S60 v3.1
     * @return the state machine owner
     */
    CBTSAController& Parent();
    
    CBtsacActive* GavdpErrorActive();
        
private:

    /**
     * the owner of the state machine
     * Not own.
     */
    CBTSAController& iParent;
    
    /**
     * State name.
     */
    TStateIndex iStateIndex;
    };

#endif // C_BTSACSTATE_H
            