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
* Description:  The audio opening state declaration
*
*/


#ifndef C_BTMSOPENAUDIO_H
#define C_BTMSOPENAUDIO_H

#include "btmsinuse.h"

class CBtmSyncSock;

// enum for eSCO parameter negotiation, same as Symbian eSCO packet types

enum EESCOParam
	{
	EEV3		= 0x20008,
	EEV4		= 0x20010,
	EEV5		= 0x20020,
	ENoeSCO = 0
	};


/**
 *  audio opening state
 *
 *  This state is responsible to establish sync socket connection with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsOpenAudio : public CBtmsInuse
    {
public:

    static CBtmsOpenAudio* NewL(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco, TBool aUseEsco = ETrue, EESCOParam aESCOParameter = EEV5);

    virtual ~CBtmsOpenAudio();

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
    void Disconnect(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * Open audio link (synchronous) to the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);


    /** 
     * Cancel openning audio
     *
     * @since S60 v3.1
     */
    void CancelOpenAudioLinkL(const TBTDevAddr& aAddr);

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
     * From MBtmSockObserver
     * Notification of a synchronous socket setup completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void SyncLinkSetupCompleteL(TInt aErr);

    /**
     * From MBtmSockObserver
     * Notification of a synchronous socket accept completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void SyncLinkAcceptCompleteL(TInt aErr);

private:

    CBtmsOpenAudio(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco, TBool aType, EESCOParam aESCOParameter);

    void ConstructL();

private:
    
    /**
     * sync Socket used for accepting SCO.
     * Ownership is possible to be transfered the the next state.
     */
    CBtmSyncSock* iAcceptSco;
    
    TBool iUseEsco;
    /**
     * sync Socket used for establishing SCO.
     * Ownership is possible to be transfered the the next state.
     */
    CBtmSyncSock* iSetupSco;
    
    EESCOParam iESCOParameter;
       
    };

#endif // C_BTMSOPENAUDIO_H
            