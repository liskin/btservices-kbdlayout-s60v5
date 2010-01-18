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
* Description:  The audio closing state declaration
*
*/


#ifndef C_BTMSCLOSEAUDIO_H
#define C_BTMSCLOSEAUDIO_H

#include "btmsinuse.h"

class CBtmSyncSock;

/**
 *  audio closing state
 *
 *  This state is responsible to release sync socket connection with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsCloseAudio : public CBtmsInuse
    {
public:

    static CBtmsCloseAudio* NewL(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco);

    virtual ~CBtmsCloseAudio();

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

    /** 
     * Cancel the outstanding operation
     *
     * @since S60 v3.1
     */
    void CancelCloseAudioLinkL(const TBTDevAddr& aAddr);

    // From base class CBtmsInuse

    /**
     * The interface for subclasses to handle RFComm error situations.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommErrorL(TInt aErr);

    // FRom base class MBtmSockObserver

    /**
     * Notification of a synchronous socket disconnect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void SyncLinkDisconnectCompleteL(TInt aErr);

private:

    CBtmsCloseAudio(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco);

    void ConstructL();

private:
    
    /**
     * The connected sync Socket.
     */
    CBtmSyncSock* iSco;
       
    };

#endif // C_BTMSCLOSEAUDIO_H
            