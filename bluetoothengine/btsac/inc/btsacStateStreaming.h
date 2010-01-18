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
* Description:  state Streaming declaration.
*
*/


#ifndef C_BTSACSTREAMING_H
#define C_BTSACSTREAMING_H

#include "btsacState.h"

/**
 * The state Configured
 *
 *
 *  @since S60 v3.1 
 */
class CBtsacStreaming : public CBtsacState
    {
    
public:

    static CBtsacStreaming* NewL(CBTSAController& aParent,TAudioOpenedBy aAudioOpenedBy, TBool aStartCollision);
    
    virtual ~CBtsacStreaming();
    
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
     * Close the audio link to the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void CloseAudioLinkL(const TBTDevAddr& aAddr);
	
	/** 
	 * From CBtsacState
     * Cancels close the audio link request
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
	void CancelCloseAudioLink(const TBTDevAddr& aAddr);
     
    // From class MInternalGavdpUser
    void GAVDP_AbortIndication(TSEID aSEID);
    TInt GAVDP_SuspendIndication(TSEID aSEID);
    void GAVDP_ReleaseIndication(TSEID aSEID);
	void GAVDP_SuspendStreamsConfirm();
 	
 	void StartRecording();
 	void CancelActionL(TInt aError, TBTSACGavdpResetReason aGavdpReset);
 	void HandleGavdpErrorL(TInt aError);

private:    

    CBtsacStreaming(CBTSAController& aParent, TAudioOpenedBy aAudioOpenedBy, TBool aStartCollision);

private:

	TBool iSuspending;
	TAudioOpenedBy iAudioOpenedBy;
	TBool iStartCollision;
    };

#endif      // C_BTSACSTREAMING_H
            