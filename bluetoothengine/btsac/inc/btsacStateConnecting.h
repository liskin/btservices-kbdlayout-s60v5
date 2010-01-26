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


#ifndef C_BTSACCONNECTING_H
#define C_BTSACCONNECTING_H

#include "btsacState.h"

/**
 * The state Idle
 *
 *
 *  @since S60 v3.1 
 */
class CBtsacConnecting : public CBtsacState
    {
    
public:

    static CBtsacConnecting* NewL(CBTSAController& aParent, TBTConnType aConnType);
    
    virtual ~CBtsacConnecting();
    
private:
    
    // From base class CBtsacState
    
    /**
     * From CBtsacState
     * Entry of this state.
     */
    void EnterL();
    
    /** 
     * From CBtsacState
     * Cancels connect request to BT accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */    
    void CancelConnectL();
	
	/** 
     * From CBtsacState
     * Cancels open audio request
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */ 
	void CancelOpenAudioL(const TBTDevAddr& aAddr);
     
    // From base class MInternalGavdpUser
    
    void GAVDP_ConnectConfirm(const TBTDevAddr& aDeviceAddr);   
        
    void CancelActionL(TInt aError);
    void HandleGavdpErrorL(TInt aError);
       

private:
    CBtsacConnecting(CBTSAController& aParent, TBTConnType aConnType);

private:
	TBTConnType iConnType; // Connection type: incoming/outgoing
    };

#endif      // C_BTSACCONNECTING_H
            