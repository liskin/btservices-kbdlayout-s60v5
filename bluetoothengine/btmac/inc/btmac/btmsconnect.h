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
* Description:  The connect state declaration
*
*/


#ifndef C_BTMSCONNECT_H
#define C_BTMSCONNECT_H

#include <bt_sock.h>
#include <btengdiscovery.h>
#include "btmstate.h"
#include "btmactiveobserver.h"
#include "btmdefs.h"

class CBtmActive;
class CBtmRfcommSock;


/**
 *  Connect state
 *
 *  This state is responsible to establish RFComm connection with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsConnect : public CBtmState, public MBTEngSdpResultReceiver
    {
public:

    /**
     * @param aParent the state machine
     * @param aStatus the request from client of btmac if not null
     * @param aAddr the remote device address
     * @param aRequestCat the category of the client request
     */    
    static CBtmsConnect* NewL(CBtmMan& aParent, TRequestStatus* aRequest, 
        const TBTDevAddr& aAddr, TRequestCategory aRequestCat);

    virtual ~CBtmsConnect();


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
     * Cancel the outstanding connecting operation
     *
     * @since S60 v3.1
     */
    void CancelConnectL();

    void CancelOpenAudioLinkL(const TBTDevAddr& aAddr);

    // From base class MBtmSockObserver
    
    /**
     * From MBtmSockObserver
     * Notification of a RFCOMM socket connect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommConnectCompletedL(TInt aErr);

    // From base class CBtmActive


    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void CancelRequest(CBtmActive& aActive);

public:
	// from MBTEngSdpResultReceiver

    void ServiceSearchComplete( const RSdpRecHandleArray& aResult, 
                                         TUint aTotalRecordsCount, TInt aErr );

    void AttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                           const RSdpResultArray& aAttr, 
                                           TInt aErr );

    void ServiceAttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                                  const RSdpResultArray& aAttr, 
                                                  TInt aErr );
                                                  
		void DeviceSearchComplete( CBTDevice* aDevice, TInt aErr );   
private:

    CBtmsConnect(CBtmMan& aParent, TRequestStatus* aRequest, 
        const TBTDevAddr& aAddr, TRequestCategory aRequestCat);

    void ConstructL();

    void DoSockConnectL(TUint aService);
    
private:
    
    /**
     * for SDP query.
     */
    CBTEngDiscovery* iBteng;
    
    
    /**
     * the RFComm channel in the remote device.
     */
    TInt iRemoteChannel;
    
    /**
     * RFComm Socket.
     * Ownership is possible to be transfered the the next state.
     */
    CBtmRfcommSock* iRfcomm;
    
    /**
     * The addr for connecting
     */
    TBTSockAddr iSockAddr;
    
    CBTDevice* iDevice;
    
    TRequestCategory iRequestCat;
    
    TInt iSearchState;
    };

#endif // C_BTMSCONNECT_H
            