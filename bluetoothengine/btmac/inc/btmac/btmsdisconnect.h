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
* Description:  The disconnect state declaration
*
*/


#ifndef C_BTMSDISCONNECT_H
#define C_BTMSDISCONNECT_H

#include "btmsinuse.h"

class CBtmSyncSock;

/**
 *  audio closing state
 *
 *  This state is responsible to release RFComm and sync socket connections with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsDisconnect : public CBtmsInuse
    {
public:

    static CBtmsDisconnect* NewL(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm);

    virtual ~CBtmsDisconnect();

    void SetConnectFailReason(TInt aReason);

    // From base class CBtmsInuse

    /**
     * The interface for subclasses to handle RFComm error situations.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommErrorL(TInt aErr);

private:
    // From base class MBtmSockObserver

    /**
     * Notification of a RFCOMM socket shutdown completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommShutdownCompletedL(TInt aErr);

private:

    // From base class CBtmState
    
    /**
     * From CBtmState
     * Entry of this state.
     *
     * @since S60 v3.1
    */
    void EnterL();


private:

    CBtmsDisconnect(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm);

    void ConstructL();

private:

    TInt iConnectFailReason;

    };

#endif // C_BTMSDISCONNECT_H
            