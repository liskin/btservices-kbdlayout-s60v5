/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Helper class for performing pairing (i.e. bonding) 
*                with another device.
*
*/

#ifndef BTENGOTGPAIR_H_
#define BTENGOTGPAIR_H_

#include <bttypes.h>
#include <bluetooth/pairing.h>
#include <e32property.h>
#include "btengpairbase.h"
#include "btengprivatepskeys.h"

/**
 *  Perform a outgoing pair with a BT device.
 *
 *  @lib ?library
 *  @since S60 v5.1
 */
NONSHARABLE_CLASS( CBTEngOtgPair ) : public CBTEngPairBase
    {

public:

    /**
     * Two-phase constructor
     * @param aParent the owner of this object
     * @param aAddr the remote device this observer is targeted to
     */
    static CBTEngPairBase* NewL( CBTEngPairMan& aParent, 
            const TBTDevAddr& aAddr );

    /**
     * Destructor
     */
    ~CBTEngOtgPair();
    
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
     * Cancel any outstanding pairing operation.
     */
    void CancelOutgoingPair();

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
     * Handles registry new paired event for the remote 
     * device this is pairing with.
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
    void RequestCompletedL( CBTEngActive* aActive, TInt aId, TInt aStatus );

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an error has occurred in RunL.
     *
     * @since S60 v5.1
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    void HandleError( CBTEngActive* aActive, TInt aId, TInt aError );

private:

    /**
     * C++ default constructor
     */
    CBTEngOtgPair( CBTEngPairMan& aParent, const TBTDevAddr& aAddr );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();
    
    /**
     * Starts an actual pair operation.
     */
    void DoPairingL();
    
    /**
     * Cancel any outstanding pairing operation.
     */
    void DoCancelOutgoingPair();
    
private: // data
	
    /**
     * Socket address of the remote device to pair with.
     */	
	TBTSockAddr iSockAddr;
	
	/**
	 * The CoD of the device to be paired
	 */
	TUint32 iCod;

    /**
     * Dedicated bonding session to the pairing server.
     */
    RBluetoothDedicatedBondingInitiator iBondingSession;
    
    /**
     * socket for creating L2CAP link with the remote device.
     */
    RSocket iSocket;
	
    /**
     * Timer for recovery from Repeated Attempts
     */
    RTimer iTimer;
    
    /**
     * used for getting and setting outgoing pair state
     */
    RProperty iOutgoProperty;
    
    /**
     * the current pairing mode this class is in
     */
    TBTOutgoingPairMode iPairMode;
    
    };


#endif // BTENGOTGPAIR_H_
