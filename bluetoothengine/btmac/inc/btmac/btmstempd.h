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
* Description:  The temporarily disconncted state declaration
*
*/


#ifndef C_BTMSHSTEMPD_H
#define C_BTMSHSTEMPD_H

#include "btmsinuse.h"

class CBtmSyncSock;

/**
 *  audio closing state
 *
 *  This state is responsible to release RFComm and sync socket connections with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsHsTempd : public CBtmsInuse
    {
public:

    static CBtmsHsTempd* NewL(CBtmMan& aParent, const TBTDevAddr& aConnectedAddr);

    virtual ~CBtmsHsTempd();

private:

    // From base class CBtmState

    void EnterL();
    
	void DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    void OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    // From base CBtmsInuse
    
    void RfcommErrorL(TInt aErr);

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
    CBtmsHsTempd(CBtmMan& aParent, const TBTDevAddr& aConnectedAddr);

private:
    TBTDevAddr iConnectedAddr;    
    };

#endif // C_BTMSDISCONNECT_H
            