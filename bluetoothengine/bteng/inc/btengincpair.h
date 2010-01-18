/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Incoming Pairing handler definition
*
*/


#ifndef BTENGINCPAIR_H_
#define BTENGINCPAIR_H_

#include <bt_sock.h>
#include "btengpairbase.h"

/**
 *  Class CBTEngIncPair
 *
 *  Handles an incoming pairing.
 *
 *  @since S60 S60 v5.1
 */
NONSHARABLE_CLASS( CBTEngIncPair ) : public CBTEngPairBase
    {
public:

    /**
     * Two-phase constructor
     * @param aParent the owner of this observer
     * @param aAddr the remote device this observer is targeted to
     */
    static CBTEngIncPair* NewL( CBTEngPairMan& aParent, 
            const TBTDevAddr& aAddr);

    /**
     * Destructor
     */
    ~CBTEngIncPair();
    
    
private: // From CBTEngPairBase
    
    /**
     * Start observing the result of pairing which was originated from
     * the remote device.
     * @param the address of the remote device to be paired
     * @return KErrNone if this request is accepted; otherwise an error code
     */
    TInt ObserveIncomingPair( const TBTDevAddr& aAddr );

    /**
     * Start an outgoing pairing with the remote device.
     * @param the address of the remote device to be paired
     * @return KErrNone if this request is accepted; otherwise an error code
     */
    void HandleOutgoingPairL( const TBTDevAddr& aAddr, TUint aCod );

    /**
     * Cancels pairing handling with the specified device
     * @param aAddr the address of the device the pairing is with
     */
    void StopPairHandling( const TBTDevAddr& aAddr );
    
    /**
     * Handle a pairing result with the remote device which this is for.
     * Must be specialized by subclass.
     *
     * @param aResult The status code of the pairing or authentication result.
     */
    void DoHandlePairServerResult( TInt aResult );
    
    /**
     * Handles event of registry new paired event with the remote 
     * device this is for.
     * @aType the type of authentication with the device.
     */
    void DoHandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev );
    
private: // from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an outstanding request has completed.
     *
     * @since S60 v5.1
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, TInt aId, TInt aStatus );

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an error has occurred in RunL.
     *
     * @since S60 v5.1
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aId, TInt aError );
    
private:
    
    /**
     * C++ default constructor
     */
    CBTEngIncPair(CBTEngPairMan& aParent, const TBTDevAddr& aAddr);
    
    /**
     * 2nd phase constructor.
     */
    void ConstructL();
    
    /**
     * Start monitoring physical link events if it hasn't yet.
     *
     * @since S60 v5.1
     * 
     */
    void MonitorPhysicalLink();
    
    /**
     * Opens physical link adaptor with the device.
     * 
     * @since S60 v5.1
     * @return KErrNone if the physical link exists.
     * 
     */
    TInt OpenPhysicalLinkAdaptor();
    
    /**
     * Cancel all outstanding requests.
     *
     * @since S60 v5.1
     */
    void CancelPlaNotification();
    
    /**
     * Handle a physical link event.
     *
     * @since S60 v5.1
     * @param aResult The status code of the event.
     */
    void HandlePhysicalLinkResultL( TInt aResult );

private:
    
    /**
     * Package buffer for retrieving physical link events.
     */
    TBTBasebandEvent iBbEvent;

    /**
     * Subsession with the socket server for 
     * getting physical link event notifications.
     */
    RBTPhysicalLinkAdapter iPla;
    
    /**
     * iPairingOkTimer is activated if the link goes down.
     * If it expires it means the pairing process has failed, 
     * then the error popup will be shown.
     * If the pairing process ends correctly, DoHandleRegistryNewPairedEvent 
     * will be called and it will cancel the timer. 
     * iActivePairingOk is the active object associated to the timer.  
     */
    RTimer iPairingOkTimer;
    CBTEngActive* iActivePairingOk;
    };

#endif /*BTENGINCPAIR_H_*/
