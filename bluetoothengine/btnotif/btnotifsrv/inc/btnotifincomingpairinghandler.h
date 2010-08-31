/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/


#ifndef BTNOTIFINCOMINGPAIRINGHANDLER_H
#define BTNOTIFINCOMINGPAIRINGHANDLER_H

#include <bt_sock.h>
#include "btnotifBasePairingHandler.h"

/**
 *  Class CBTNotifIncomingPairingHandler
 *
 *  Handles an incoming pairing.
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifIncomingPairingHandler ) : public CBTNotifBasePairingHandler
    {
public:

    /**
     * Two-phase constructor
     * @param aParent the owner of this observer
     * @param aAddr the remote device this observer is targeted to
     */
    static CBTNotifBasePairingHandler* NewL( CBTNotifSecurityManager& aParent, 
            const TBTDevAddr& aAddr);

    /**
     * Destructor
     */
    ~CBTNotifIncomingPairingHandler();
    
    
private: // From CBTNotifBasePairingHandler
    
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
    
private: // from base class MBtSimpleActiveObserver

    /**
     * Callback to notify that an outstanding request has completed.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus );

    /**
     * Callback from Docancel() for handling cancelation of an outstanding request.
     *
     * @since Symbian^4
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId );
    
    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBtSimpleActive* aActive, TInt aError );
    
private:
    
    /**
     * C++ default constructor
     */
    CBTNotifIncomingPairingHandler(CBTNotifSecurityManager& aParent, const TBTDevAddr& aAddr);
    
    /**
     * 2nd phase constructor.
     */
    void ConstructL();
    
    /**
     * Start monitoring physical link events if it hasn't yet.
     *
     * @since Symbian^4
     * 
     */
    void MonitorPhysicalLink();
    
    /**
     * Opens physical link adaptor with the device.
     * 
     * @since Symbian^4
     * @return KErrNone if the physical link exists.
     * 
     */
    TInt OpenPhysicalLinkAdaptor();
    
    /**
     * Cancel all outstanding requests.
     *
     * @since Symbian^4
     */
    void CancelPlaNotification();
    
    /**
     * Handle a physical link event.
     *
     * @since Symbian^4
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
    CBtSimpleActive* iActivePairingOk;
    TBool iUserAwarePairing;
    };

#endif /*BTNOTIFINCOMINGPAIRINGHANDLER_H*/



