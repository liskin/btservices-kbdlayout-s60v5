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
* Description:  The reconnect state declaration
*
*/


#ifndef C_BTMRESCONNECT_H
#define C_BTMRESCONNECT_H

#include <bt_sock.h>
#include <btengdiscovery.h>

#include "btmsinuse.h"
#include "btmactiveobserver.h"
#include "btmdefs.h"

class CBtmActive;
class CBtmRfcommSock;

/**
 *  Reconnect state
 *
 *  This state is responsible to reconnect HSP RFComm connection with a BT device
 *
 *  @since S60 v3.1
 */
class CBtmsReconnect : public CBtmsInuse, MBTEngSdpResultReceiver
    {
public:

    static CBtmsReconnect* NewL(CBtmMan& aParent, TRequestStatus* aRequest, const TBTDevAddr& aAddr);

    virtual ~CBtmsReconnect();

private:

    // From base class CBtmState
    
    /**
     * From CBtmState
     * Entry of this state.
     *
     * @since S60 v3.1
    */
    void EnterL();

	void DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

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

    // From CBtmsInuse
    /**
     * The interface for subclasses to handle RFComm error situations.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommErrorL(TInt aErr);

    // From base class MBtmSockObserver
    
    /**
     * From MBtmSockObserver
     * Notification of a RFCOMM socket connect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void RfcommConnectCompletedL(TInt aErr);


public:
	// from MBTEngSdpResultReceiver

    void ServiceSearchComplete( const RSdpRecHandleArray& aResult, 
                                         TUint aTotalRecordsCount, TInt aErr );

    void AttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                           const RSdpResultArray& aAttr, TInt aErr );

    void ServiceAttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                                  const RSdpResultArray& aAttr, 
                                                  TInt aErr );

		void DeviceSearchComplete( CBTDevice* aDevice, TInt aErr );
private:

    CBtmsReconnect(CBtmMan& aParent, TRequestStatus* aRequest, const TBTDevAddr& aAddr);

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
     * The addr for connecting
     */
    TBTSockAddr iSockAddr;
    };

#endif // C_BTMSCONNECT_H
            