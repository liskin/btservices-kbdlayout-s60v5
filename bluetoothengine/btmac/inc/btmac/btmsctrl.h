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
* Description:  The Control state declaration
*
*/


#ifndef C_BTMSCTRL_H
#define C_BTMSCTRL_H

#include "btmsinuse.h"
#include "btmactiveobserver.h"
#include "btmdefs.h"

class CBtmActive;
class CBtmSyncSock;

const TUint KBTSdpHandsfreeAudioGateway = 0x111F;
const TUint KBTSdpHeadsetAudioGateway = 0x1112;


/**
 *  Connect state
 *
 *  This state is responsible to establish RFComm connection with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsCtrl : public CBtmsInuse, public MBtmActiveObserver
    {
public:

    /**
     * @param aParent the state machine
     * @param aStatus the request from client of btmac if not null
     * @param aRfcomm the rfcomm connection
     * @param aSco the sync socket having an outstanding accept operation if not null
     * @param aRequestCat the category of the client request
     */
    static CBtmsCtrl* NewL(
        CBtmMan& aParent,
        CBtmRfcommSock* aRfcomm,
        CBtmSyncSock* aSco);

    virtual ~CBtmsCtrl();

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
     * disconnect the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * From CBtmState
     * Open audio link (synchronous) to the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);
    
    // From base class CBtmsInuse

    /**
     * The interface for subclasses to handle RFComm error situations.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommErrorL(TInt aErr);

    // From base class CBtmsInuse

    void AccInUse();
    
    TBool CanDisableNrec();

    // FRom base class MBtmSockObserver

    /**
     * From MBtmSockObserver
     * Notification of a synchronous socket accept completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void SyncLinkAcceptCompleteL(TInt aErr);

    void RfcommLinkInSniffModeL();

    // From base class CBtmActive

    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void RequestCompletedL(CBtmActive& aActive);

    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void CancelRequest(CBtmActive& aActive);
    

private:

    CBtmsCtrl(CBtmMan& aParent, 
        CBtmRfcommSock* aRfcomm, 
        CBtmSyncSock* aSco);

    void ConstructL();

private:
    
    /**
     * sync socket that listens a sco connection from a BT device.
     * Ownership is possible to be transfered the the next state.
     */
    CBtmSyncSock* iSco;
    };

#endif // C_BTMSCTRL_H
            