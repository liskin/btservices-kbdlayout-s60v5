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
* Description:  state Suspended declaration.
*
*/


#ifndef C_BTSACSUSPENDED_H
#define C_BTSACSUSPENDED_H

#include "btsacState.h"

/**
 * The state Configured
 *
 *
 *  @since S60 v3.1 
 */
class CBtsacSuspended : public CBtsacState
    {
    
public:

    static CBtsacSuspended* NewL(CBTSAController& aParent);
    
    virtual ~CBtsacSuspended();
    
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
     * @param aAddr the BD address of the accessory
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
     * Cancels the open audio link request
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void CancelOpenAudioL(const TBTDevAddr& aAddr);
	
     
    // From class MInternalGavdpUser
    void GAVDP_StartStreamsConfirm();
   	TInt GAVDP_StartIndication(TSEID aSEID);
    void GAVDP_ReleaseIndication(TSEID aSEID);
 	
 	void CancelActionL(TInt aError);
 	void HandleGavdpErrorL(TInt aError);

private:    

    CBtsacSuspended(CBTSAController& aParent);
    
    TBool iOpenAudioReq;
    };

#endif      // C_BTSACSUSPENDED_H
            