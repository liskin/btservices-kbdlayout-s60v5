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
* Description:  The audio opened state declaration
*  Version     : %version: 2.2.3 %
*
*/


#ifndef C_BTMSAUDIO_H
#define C_BTMSAUDIO_H

#include "btmstate.h"

#include "btmsinuse.h"

class CBtmSyncSock;

/**
 *  audio opened state
 *
 *  This state indicates a sync socket connection with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsAudio : public CBtmsInuse
    {
public:

    static CBtmsAudio* NewL(CBtmMan& aParent, CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco);

    virtual ~CBtmsAudio();

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
     * disconnect the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    void OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * Close the audio link (synchronous) to the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void CloseAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    // From base class CBtmsInuse

    /**
     * The interface for subclasses to handle RFComm error situations.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommErrorL(TInt aErr);

    // From base class CBtmstate
    
    /*
     * Get auido link latency 
     *
     * @since S60 v5.0
     */  
    TInt AudioLinkLatency();
    
    // From base class MBtmSockObserver

    /**
     * Notification of a synchronous socket disconnect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void SyncLinkDisconnectCompleteL(TInt aErr);

private:

    CBtmsAudio(CBtmMan& aParent, CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco);

private:
    
    /**
     * sync socket that has a sco connection with a BT device.
     * Ownership is possible to be transfered the the next state.
     */
    CBtmSyncSock* iSco;
       
    };

#endif // C_BTMSAUDIO_H
            
