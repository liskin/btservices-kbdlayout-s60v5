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


#ifndef C_BTSACCONNECTED_H
#define C_BTSACCONNECTED_H

#include "btsacState.h"

/**
 * The state Connected
 *
 *
 *  @since S60 v3.1 
 */
	
class CBtsacConnected : public CBtsacState, public MBtsacActiveObserver
    {
    
public:

    static CBtsacConnected* NewL(CBTSAController& aParent, TBTConnType aConnType);
    
    virtual ~CBtsacConnected();
    
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

    void CancelOpenAudioLinkL();

   	void ConfigureL();
   	void GetCapabilitiesOfAllSEPs();
     
    // From class MInternalGavdpUser
   	void GAVDP_ConfigurationConfirm();
   	void GAVDP_BearerReady(RSocket aSocket, const TAvdtpSockAddr& aAddress);
   	void GAVDP_SEPDiscovered(const TAvdtpSEPInfo& aSEP);
   	void GAVDP_SEPDiscoveryComplete();
   	void GAVDP_SEPCapability(TAvdtpServiceCapability* aCapability); 
	void GAVDP_SEPCapabilityComplete();	
	void GAVDP_AbortIndication(TSEID aSEID);
	void GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID);
	
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
    
    void StartTimer(TTimeIntervalMicroSeconds32 aTimeout, TInt aRequestId);
    void StopTimer();
    
    void CancelActionL(TInt aError);
    void HandleGavdpErrorL(TInt aError);
    
    void StartConfigurationL();

private:    

    CBtsacConnected(CBTSAController& aParent, TBTConnType aConnType);
    void ConstructL();

private:
	enum TBTSACSepFound
	    {
	    ESEPNotInitialized,
	    ESEPConfigure,
	    ESEPInUse,
	    ESEPCodecTypeNotAllowed
	    };
	
	/**
     * AO for configuration timer.
     */
    CBtsacActive* iTimerActive;
    
	/**
     * Configuration timer 
     */
    RTimer iTimer;
    
    TBool iSuitableSEPFoundAlready;
    TBTConnType iConnType; // Connection type: incoming/outgoing
    TBool iFirstConn; // used to learn if configuration should be done
    TBTSACSepFound iSEPFound;
    TInt iRemoteSEPIndex;
    TBool iBearersQuery;
    TBool iCancelConnectReq;
    TAudioOpenedBy iAudioOpenedBy;
    };

#endif      // C_BTSACCONNECTED_H
            