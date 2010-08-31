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
* Description: Pairing handler for local device initiated pairing
*
*/

#ifndef BTNOTIFOUTGOINGPAIRINGHANDLER_H_
#define BTNOTIFOUTGOINGPAIRINGHANDLER_H_

#include <bttypes.h>
#include <bluetooth/pairing.h>
#include <e32property.h>
#include "btnotifclientserver.h"
#include "btnotifBasePairingHandler.h"
#include <hb/hbcore/hbsymbianvariant.h>
#include "btnotificationresult.h"
#include "bluetoothdevicedialogs.h"

class CBluetoothNotification;

enum TBTOutgoingPairMode
    {
    /**
     * not outgoing pairing operation
     */
    EBTOutgoingPairNone = 0,
    
    /**
     * outgoing automatic 0000 pin pairing with headset in progress 
     */
    EBTOutgoingHeadsetAutoPairing,
    
    /**
     * outgoing manual pairing with headset in progress 
     */
    EBTOutgoingHeadsetManualPairing,
    
    /**
     * outgoing pairing with non-headset device in progress 
     */
    EBTOutgoingNoneHeadsetPairing,
    };

/**
 *  Perform a outgoing pair with a BT device.
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifOutgoingPairingHandler ) : public CBTNotifBasePairingHandler,
                                                      public MBTNotificationResult
    {

public:

    /**
     * Two-phase constructor
     * @param aParent the owner of this object
     * @param aAddr the remote device this observer is targeted to
     */
    static CBTNotifBasePairingHandler* NewL( CBTNotifSecurityManager& aParent, 
            const TBTDevAddr& aAddr );

    /**
     * Destructor
     */
    ~CBTNotifOutgoingPairingHandler();

    
    // from base class MBTNotificationResult

    /**
     * From MBTNotificationResult.
     * Handle an intermediate result from a user query.
     * This function is called if the user query passes information
     * back before it has finished i.e. is dismissed. The final acceptance/
     * denial of a query is passed back in MBRNotificationClosed.
     *
     * @since Symbian^4
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRDataReceived( CHbSymbianVariantMap& aData );

    /**
     * From MBTNotificationResult.
     * The notification is finished. The resulting data (e.g. user input or
     * acceptance/denial of the query) is passed back here.
     *
     * @since Symbian^4
     * @param aErr KErrNone or one of the system-wide error codes.
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRNotificationClosed( TInt aError, const TDesC8& aData );
    
private:
    
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
     * Gets the pin code to be used for pairing a device.
     * @param aPin contains the pin code if it is not empty
     * @param aAddr the device to which pairing is performed.
     * @param the required minimum length of a pin code.
     */
    void GetPinCode( TBTPinCode& aPin, const TBTDevAddr& aAddr, TInt aMinPinLength );
    
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
    
private: // from base class MBtSimpleActiveObserver

    /**
     * Callback to notify that an outstanding request has completed.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus );

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
    void HandleError( CBtSimpleActive* aActive, TInt aError );

private:
    /**
     * Get a notification and configure it according to the current operation.
     *
     * @since Symbian^4
     * @param aType The notification type.
     * @param aResourceId Identifier for the resource to display.
     */
    void PrepareNotificationL( TBluetoothDialogParams::TBTDialogType aType,
                TBTDialogResourceId aResourceId );

    /**
     * Handle the result from a notification that is finished.
     *
     * @since Symbian^4
     * @param aErr KErrNone or one of the system-wide error codes.
     * @param aData The returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    void NotificationClosedL( TInt aError, const TDesC8& aData );

private:

    /**
     * C++ default constructor
     */
    CBTNotifOutgoingPairingHandler( CBTNotifSecurityManager& aParent, const TBTDevAddr& aAddr );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();
    
    /**
     * Starts an actual pair operation.
     */
    void DoPairingL();
    
    /**
     * Ask the user to retry pairing.
     */    
    void ShowPairingRetryDialog();
    
    /**
     * Tell the user the pairing failed.
     */    
    void ShowPairingFailureDialog();
    
private: // data
	
    /**
     * Socket address of the remote device to pair with.
     */	
	TBTSockAddr iSockAddr;
	
	/**
	 * The CoD of the device to be paired
	 */
	TBTDeviceClass iCod;

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
     * the current pairing mode this class is in
     */
    TBTOutgoingPairMode iPairMode;

    /**
     * Pointer to an outstanding user interaction.
     * Not own.
     */
    CBluetoothNotification* iNotification;

    /**
     * Number of pairing attempt
     */
    TInt iPairingAttempt;
    
    };


#endif /* BTNOTIFOUTGOINGPAIRINGHANDLER_H_ */


