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


#ifndef C_BTSACCONFIGURING_H
#define C_BTSACCONFIGURING_H

#include "btsacState.h"

/**
 * The state Connected
 *
 *
 *  @since S60 v3.1 
 */
class CBtsacConfiguring : public CBtsacState
    {
    
public:

    static CBtsacConfiguring* NewL(CBTSAController& aParent, TSEID aLocalSEID, TSEID aRemoteSEID);
    
    virtual ~CBtsacConfiguring();
    
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
     * From CBtsacState
     * Cancel the outstanding connecting operation
     *
     * @since S60 v3.1
     */
    void CancelConnectL();

     
    // From class MInternalGavdpUser
    void GAVDP_BearerReady(RSocket aSocket, const TAvdtpSockAddr& aAddress);
	TInt GAVDP_ConfigurationIndication(TAvdtpServiceCapability* aCapability);
	TInt GAVDP_ConfigurationEndIndication();
	TInt GAVDP_StartIndication(TSEID aLocalSEID);
	
	void CancelActionL(TInt aError);
	void HandleGavdpErrorL(TInt aError);

private:    

    CBtsacConfiguring(CBTSAController& aParent, TSEID aLocalSEID, TSEID aRemoteSEID);
    
private:

    TSEID iLocalSEID;
    TSEID iRemoteSEID;
    TBool iSEPFound;
    TInt iRemoteSEPIndex;
    TAudioOpenedBy iAudioOpenedBy;
    };

#endif      // C_BTSACCONFIGURING_H
            