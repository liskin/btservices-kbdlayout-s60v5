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
* Description:  The in use state declaration
*
*/


#ifndef C_BTMSINUSE_H
#define C_BTMSINUSE_H

#include <e32base.h>
#include <es_sock.h>
#include <bttypes.h>
#include "btmstate.h"
#include <btengconstants.h>

class CBtmRfcommSock;
class CBtmSyncSock;

/**
 *  the in use state. This class is abstract.
 *
 *  @since S60 v3.1
 */
class CBtmsInuse : public CBtmState
    {
    
public:

    virtual ~CBtmsInuse();

    /**
     * The interface for subclasses to handle RFComm error situations.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void RfcommErrorL(TInt aErr) = 0;

protected:

    // From CBtmstate

    virtual void ConnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    virtual void SendProtocolDataL(const TDesC8& aData);
    
    TBTDevAddr Remote();
    
protected:
    
    // From base class MBtmSockObserver
    
    /**
     * Notification of a RFCOMM socket connect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void RfcommConnectCompletedL(TInt aErr);

    /**
     * Notification of a RFCOMM socket accpet completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     * @param aService the connected profile
     */
    virtual void RfcommAcceptCompletedL(TInt aErr, TUint aService);

    /**
     * Notification of a RFCOMM socket shutdown completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void RfcommShutdownCompletedL(TInt aErr);
    
    /**
     * Notification of a RFCOMM socket send completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommSendCompletedL(TInt aErr);

    /**
     * Notification of a RFCOMM socket receive completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommReceiveCompletedL(TInt aErr, const TDesC8& aData);

protected:

    CBtmsInuse(CBtmMan& aParent, TRequestStatus* aRequest, CBtmRfcommSock* aRfcomm);

protected:

    CBtmRfcommSock* SwapStateRfcommSock();
    
    CBtmSyncSock* SwapSyncSock(CBtmSyncSock*& aSyncSock);
    
protected:
    
    /**
     * RFComm Socket.
     * Ownership is possible to be transfered the the next state.
     */
    CBtmRfcommSock* iRfcomm;
    
    
    };

#endif // C_BTMSINUSE_H
            