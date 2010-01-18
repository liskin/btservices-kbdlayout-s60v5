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
* Description:  The listen state declaration
*
*/


#ifndef C_BTMSLISTEN_H
#define C_BTMSLISTEN_H

#include "btmstate.h"

class CBtmRfcommSock;

/**
 *  Connect state
 *
 *  This state is responsible to establish RFComm connection with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsListen : public CBtmState
    {

public:

    static CBtmsListen* NewL(CBtmMan& aParent);

    virtual ~CBtmsListen();

private:

    // From base class CBtmState
    
    /**
     * From CBtmState
     * Entry of this state.
     *
     * @since S60 v3.1
    */
    void EnterL();

    /**
     * From CBtmState
     * Returns the next state if EnterL leaves.
     *
     * @since S60 v3.1
     * @return the next state
     */
    CBtmState* ErrorOnEntryL(TInt aReason);

    /**
     * From CBtmState
     * Starts bt audio listener
     * @since S60 v3.2
     */
    void StartListenerL();
    
    /** 
     * From CBtmState
     * Connect to BT accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void ConnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * From CBtmState
     * Open audio link to BT accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
	void OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    // From base class MBtmSockObserver
    
    /**
     * From MBtmSockObserver
     * Notification of a RFCOMM socket accpet completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     * @param aService the connected profile
     */
    void RfcommAcceptCompletedL(TInt aErr, TUint aService);

private:

    CBtmsListen(CBtmMan& aParent);

    void ConstructL();

private:
    
    /**
     * Sock to listen HFP.
     * Ownership will be transfered the the next state if 
     * accept is completed with KErrNone. Otherwise owned.
     */
    CBtmRfcommSock* iHfpSock;
    
    /**
     * Sock to listen HSP.
     * Ownership will be transfered the the next state if 
     * accept is completed with KErrNone. Otherwise owned.
     */
    CBtmRfcommSock* iHspSock;
    
    };

#endif // C_BTMSLISTEN_H
            