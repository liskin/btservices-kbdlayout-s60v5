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
* Description:  The RFCOMM connected state declaration
*
*/


#ifndef C_BTMSRFCOMM_H
#define C_BTMSRFCOMM_H

#include "btmsinuse.h"
#include "btmactiveobserver.h"
#include "btmdefs.h"

class CBtmActive;

/**
 *  Connect state
 *
 *  This state is responsible to establish RFComm connection with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsRfcomm : public CBtmsInuse
    {
public:

    /**
     * @param aParent the state machine
     * @param aStatus the request from client of btmac if not null
     * @param aRfcomm the rfcomm connection
     * @param aRequestCat the category of the client request
     */
    static CBtmsRfcomm* NewL(
        CBtmMan& aParent,
        TRequestStatus* aStatus,
        CBtmRfcommSock* aRfcomm,
        TRequestCategory aRequestCat = ERequestNone);

    virtual ~CBtmsRfcomm();

private:

    // From base class CBtmState
    
    /**
     * From CBtmState
     * Entry of this state.
     *
     * @since S60 v3.1
    */
    void EnterL();

    void CancelConnectL();

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

    void CancelOpenAudioLinkL(const TBTDevAddr& aAddr);
    
    // From base class CBtmsInuse

    /**
     * The interface for subclasses to handle RFComm error situations.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommErrorL(TInt aErr);

    // From base class CBtmsInuse

    /**
     * 
     *
     * @since S60 v3.1
     * @param
     */
    void SlcIndicateL(TBool aSlc);
    
    TBool CanDisableNrec();   

private:

    CBtmsRfcomm(CBtmMan& aParent, 
        TRequestStatus* aStatus, 
        CBtmRfcommSock* aRfcomm, 
        TRequestCategory aRequestCat);

    void ConstructL();

private:
   
    TRequestCategory iRequestCat;
    };

#endif // C_BTMSRFCOMM_H
            