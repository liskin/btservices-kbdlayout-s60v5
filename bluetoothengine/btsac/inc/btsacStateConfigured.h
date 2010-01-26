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
* Description:  state Configured declaration.
*
*/


#ifndef C_BTSACCONFIGURED_H
#define C_BTSACCONFIGURED_H

#include "btsacState.h"

/**
 * The state Configured
 *
 *
 *  @since S60 v3.1 
 */
class CBtsacConfigured : public CBtsacState, public MBtsacActiveObserver
    {
    
public:

    static CBtsacConfigured* NewL(CBTSAController& aParent, RSocket aNewSocket, TAudioOpenedBy aAudioOpenedBy, TStreamConfiguredBy aStreamConfiguredBy);
    
    virtual ~CBtsacConfigured();
    
private:
    
    // From base class CBtsacState
    
    /**
     * From CBtsacState
     * Entry of this state.
     */
    void EnterL();
    
    /** 
     * disconnect the connected accessory
     *
     * @since S60 v3.1
     * @param aStatus On completion, will contain an error code
     */
    void DisconnectL();
    
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
	
	/** 
     * From CBtsacState
     * Cancel the outstanding connecting operation
     *
     * @since S60 v3.1
     */
    void CancelConnectL();
     
    // From class MInternalGavdpUser
 	void GAVDP_StartStreamsConfirm();
  	TInt GAVDP_StartIndication(TSEID aSEID);
 	
 	/**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
	void RequestCompletedL(CBtsacActive& aActive);
    
    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
	virtual void CancelRequest(CBtsacActive& aActive);
    
    void StartTimer(TTimeIntervalMicroSeconds32 aTimeout);
    void StopTimer();
 	
 	void CancelActionL(TInt aError);
 	void HandleGavdpErrorL(TInt aError);

private:    

    CBtsacConfigured(CBTSAController& aParent, RSocket aNewSocket, TAudioOpenedBy aAudioOpenedBy, TStreamConfiguredBy aStreamConfiguredBy);
    void ConstructL();

private:
	enum TStartStreamStatus
	    {
	    EStartNone,
	    EStartSendBySrc,
	    EStartCollision
	    };
	/**
     * AO for configuration timer.
     */
    CBtsacActive* iTimerActive;
    
	/**
     * Configuration timer 
     */
    RTimer iTimer;
    
	TAudioOpenedBy iAudioOpenedBy;
	TStreamConfiguredBy iStreamConfiguredBy;
	TStartStreamStatus iStartStreamStatus;
    };

#endif      // C_BTSACCONFIGURED_H
            