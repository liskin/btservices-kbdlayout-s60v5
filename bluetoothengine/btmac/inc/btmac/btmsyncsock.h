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
* Description:  BT sync socket.
*  Version     : %version: 7 %
*
*/


#ifndef BTMACSCOLINKER_H
#define BTMACSCOLINKER_H

#include <e32base.h> 
#include <bt_sock.h>
#include <bttypes.h>

class MBtmSockObserver;

const TInt KEscoPacketTypeSpec = TBTSyncPackets::ESyncPacketsEV5 |
    TBTSyncPackets::ESyncPacketsEV3 |
    TBTSyncPackets::ESyncPacketsEV4;
    
const TInt KScoPacketTypeSpec = TBTSyncPackets::ESyncAnySCOPacket;

const TInt KMaxLatencySpec = 16;


/**
 * Sync socket.
 *
 *  @since S60 v3.1
 */
class CBtmSyncSock : public CBase, public MBluetoothSynchronousLinkNotifier
    {
    
public:

    static CBtmSyncSock* NewL(MBtmSockObserver& aObserver, RSocketServ& aSockServ);
    
    virtual ~CBtmSyncSock();

    void SetupL(const TBTDevAddr& aAddr, const TBTSyncPackets& aPackets);


    void CancelSetup();

    /**
     * Disconnects a sync socket.
     *
     * @since S60 v3.1
     */
    void DisconnectL();

    /**
     * Listens to incoming sync sock connection.
     *
     * @since S60 v3.1
     */
    void AcceptL(TBool aAllowEsco);
    
    /**
     * Cancel the accept operation.
     *
     * @since S60 v3.1
     */
    void CancelAccept();

	void SetMaxLatency(TUint16 aLatency);
	
	void SetRetransmissionEffort(TBTeSCORetransmissionTypes aRetransmissionEffort);
    
    /**
     * Zero the owned pointers in this object.
     *
     * @since S60 v3.1
     */
    void Reset();

    /**
     * Sets the socket observer to the specified.
     *
     * @since S60 v3.1
     * @param aObserver the new observer.
     */
    void SetObserver(MBtmSockObserver& aObserver);
    
    /**
     * Return audio link latency
     *
     * @since S60 5.0
     * @return latency.
     */ 
    TInt Latency();
    
private:  

    // From base class MBluetoothSynchronousLinkNotifier

    /**
     * Notification of a synchronous socket setup completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void HandleSetupConnectionCompleteL(TInt aErr);

    /**
     * Notification of a synchronous socket disconnect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void HandleDisconnectionCompleteL(TInt aErr);

    /**
     * Notification of a synchronous socket accept completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void HandleAcceptConnectionCompleteL(TInt aErr);

    /**
     * Notification of a synchronous socket send completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void HandleSendCompleteL(TInt aErr);

    /**
     * Notification of a synchronous socket send completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void HandleReceiveCompleteL(TInt aErr);

private:

    CBtmSyncSock(MBtmSockObserver& aObserver, RSocketServ& aSockServ);

    void ConstructL();

private:

    class TScoAcceptor;

    void AcceptRedirectL(TInt aErr, TScoAcceptor& aAcceptor);

    class TScoAcceptor : public MBluetoothSynchronousLinkNotifier
        {
    public:
        TScoAcceptor();
        TScoAcceptor(CBtmSyncSock* aParent);
    private:
        CBtmSyncSock* iParent;
    private:
        void HandleSetupConnectionCompleteL(TInt aErr);
        void HandleDisconnectionCompleteL(TInt aErr);
        void HandleAcceptConnectionCompleteL(TInt aErr);
        void HandleSendCompleteL(TInt aErr);
        void HandleReceiveCompleteL(TInt aErr);
        
        friend void CBtmSyncSock::AcceptRedirectL(TInt aErr, TScoAcceptor& aAcceptor);
        };

private:    // Data

    /**
     * socket event observer.
     * Not own.
     */
    MBtmSockObserver* iObserver;
    
    /**
     * The socket server session
     * Not own.
     */
    RSocketServ& iServer;

    /**
     * The BT SCO link for setup connection and SCO accept
     */
    CBluetoothSynchronousLink* iSco;

    /**
     * The BT Sync link for ESCO accept
     */
    CBluetoothSynchronousLink* iEScoLink;

    TScoAcceptor iScoAcceptor;
    
    TScoAcceptor iESco_Acceptor;
    
    CBluetoothSynchronousLink* iConnectedLink; // pointer to either iSco or iEScoLink or none.
};

#endif      // BTMACSCOLINKER_H
            
