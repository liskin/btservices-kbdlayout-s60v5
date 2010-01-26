/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  state Listening declaration.
*
*/


#ifndef C_BTSACLISTENING_H
#define C_BTSACLISTENING_H

#include "btsacState.h"

enum TBTInitProcedure
	{
	EInitProcedureNone,
	EInitProcedureOngoing,
	EInitProcedureWaitingConfConfirmed,
	EInitProcedureDone
	};

/**
 * The state Idle
 *
 *
 *  @since S60 v3.1 
 */
class CBtsacListening : public CBtsacState, public MBtsacActiveObserver
    {
    
public:

    static CBtsacListening* NewL(CBTSAController& aParent, TBTSACGavdpResetReason aGavdpResetReason, TInt aDisconnectReason);
    
    virtual ~CBtsacListening();
    
private:
    
    // From base class CBtsacState
    
    /**
     * From CBtsacState
     * Entry of this state.
     */
    void EnterL();
    
    /** 
     * From CBtsacState
     * Connect to BT accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void ConnectL(const TBTDevAddr& aAddr);
    
    /** 
     * From CBtsacState
     * Open audio link to BT accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
	void OpenAudioLinkL(const TBTDevAddr& aAddr);
	
	/** 
     * From CBtsacState
     * Cancels Open audio link request
     *
     * @since S60 v3.1
     */
	void CancelOpenAudioLinkL();
     
    // From base class MInternalGavdpUser
    void GAVDP_ConnectConfirm(const TBTDevAddr& aDeviceAddr);
    void GAVDP_ConfigurationConfirm();
    void GAVDP_AbortStreamConfirm();
    void GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID);

	void HandleGavdpErrorL(TInt aError);
	
	void DoSelfComplete(TInt aError);
	
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
	
	TInt ResetGavdp();
	void GoListen();
	void DeleyedOpenAudioAndConnectL();

private:
	
    CBtsacListening(CBTSAController& aParent, TBTSACGavdpResetReason aGavdpResetReason, TInt aDisconnectReason);
    void ConstructL();
    
private:
	TBTSACGavdpResetReason iGavdpResetReason;
	TInt iDisconnectReason;
	TInt iPendingRequests;
	TBTInitProcedure iInitializationProcedure;
	TBool iUnfinishedGavdpError;
	 
	 /**
     * AO for self completing.
     */
    CBtsacActive* iSelfCompleteActive;
    };

#endif      // C_BTSACLISTENING_H
            