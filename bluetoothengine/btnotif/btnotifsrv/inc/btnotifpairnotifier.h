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

#ifndef BTNOTIFPAIRNOTIFIER_H
#define BTNOTIFPAIRNOTIFIER_H

#include <e32base.h>
#include <btmanclient.h>
#include <hb/hbcore/hbsymbianvariant.h>
#include "bluetoothdevicedialogs.h"
#include "btnotificationresult.h"
#include "bluetoothtrace.h"

class CBTNotifSecurityManager;
class CBTNotifConnectionTracker;
class CBluetoothNotification;

/**
 *  Helper class for performing user prompt for pairing.
 *  
 *  The design of this class is focussed on structure and maintainability first.
 *  Duplicate (state) information is kept to a minimum. And memory usage comes
 *  before processing. Pairing is an infrequent operation, and this class is
 *  only instantiated when there is pairing-related processing, so extreme
 *  focus on memory or processing efficiency would have relatively little effect.
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifPairNotifier ) : public CBase,
                                            public MBTNotificationResult
    {

public:

    /**
     * Two-phased constructor.
     * @param aConnection Pointer to the parent.
     * @param aDevice Pointer to information of the remote device.
     * aParam The owner of this object
     */
    static CBTNotifPairNotifier* NewL( CBTNotifSecurityManager& aParent );

    /**
    * Destructor.
    */
    virtual ~CBTNotifPairNotifier();

    /**
     * Handle a notifier request for pairing with the remote device
     * of this connection.
     *
     * @since Symbian^4
     * @param aMessage The client of this request.
     */    
    void StartPairingNotifierL( const RMessage2& aMessage );

    /**
     * Update an outstanding request for this connection.
     *  
     * @since Symbian^4
     * @param aUid The UID of the notifier for this update.
     * @param aParams The updated parameters for this request from the client.
     */
    void UpdatePairingNotifierL( TInt aUid, const TDesC8& aParams );

    /**
     * Cancel an outstanding request for this connection.
     *  
     * @since Symbian^4
     * @param aUid The UID of the notifier for this pairing request.
     */
    void CancelPairingNotifierL( TInt aUid );

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
     * C++ default constructor.
     */
    CBTNotifPairNotifier( CBTNotifSecurityManager& aParent );

    /**
     * Symbian 2nd-phase constructor.
     */
    void ConstructL();

    void StartPairingUserInputL();
    
    /**
     * Process the user input and complete the outstanding pairing request.
     *
     * @since Symbian^4
     * @param aError The result off the notification.
     * @param aResult The user response; ETrue if the user accepted the query,
     *                otherwise EFalse.
     * @param aData The data returned from the notification dialog.
     */
    void CompletePairingNotifierL( TInt aError, TBool aResult, const TDesC8& aData );

    /**
     * Ask the user to allow incoming pairing.
     *
     * @since Symbian^4
     */
    void StartAcceptPairingQueryL();

    /**
     * Process the user input and for accepting an incoming pairing and
     * continue with the outstanding pairing request.
     *
     * @since Symbian^4
     * @param aError The result of the notification.
     * @param aResult The user response; ETrue if the user accepted the query,
     *                otherwise EFalse.
     */
    void CompleteAcceptPairingQueryL( TInt aError);
    
    /**
     * Parse the parameters of a request for pairing.
     * This function also returns values to use for dialog config, and sets
     * the operation state member variable (iOperation).
     *
     * @since Symbian^4
     */
    void ParseNotifierReqParamsL();

    /**
     * Parse the parameters of a request for pairing using pin query.
     *
     * @since Symbian^4
     * @param aLocallyInitiated On return, will be set to ETrue if the pairing 
     *                          was initiated by us.
     * @param aMinPinLength On return, this will contain the minimum passcode length.
     */
    void ParseLegacyPinCodeReqParamsL( TBool& aLocallyInitiated, 
            TInt& aMinPinLength, TBTDevAddr& aAddr );    
    
    /**
     * Parse the parameters of a request for pairing using pin query.
     *
     * @since Symbian^4
     * @param aLocallyInitiated On return, will be set to ETrue if the pairing 
     *                          was initiated by us.
     * @param aMinPinLength On return, this will contain the minimum passcode length.
     */
    void ParsePinCodeReqParamsL( TBool& aLocallyInitiated, TInt& aMinPinLength,
            TBTDevAddr& aAddr);

    /**
     * Parse the parameters of a request for pairing using numeric comparison.
     *
     * @since Symbian^4
     * @param aLocallyInitiated On return, will be set to ETrue if the pairing 
     *                          was initiated by us.
     * @param aNumVal On return, this descriptor will contain the passkey to 
     *                show to the user.
     */
    void ParseNumericCompReqParamsL( TBool& aLocallyInitiated, TDes& aNumVal,
            TBTDevAddr& aAddr);

    /**
     * Parse the parameters of a request for pairing using passkey display.
     *
     * @since Symbian^4
     * @param aLocallyInitiated On return, will be set to ETrue if the pairing 
     *                          was initiated by us.
     * @param aNumVal On return, this descriptor will contain the passkey to 
     *                show to the user.
     */
    void ParsePasskeyDisplayReqParamsL( TBool& aLocallyInitiated, TDes& aNumVal,
            TBTDevAddr& aAddr );

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
    
private: // data

    enum TNotifierState
        {
        EIncomingPairingAcceptconfirm,
        EPairingInputConfirm,
        };
    
    CBTNotifSecurityManager& iParent;
    
    /**
     * The client request.
     */
    RMessage2 iNotifierMessage;
    
    /**
     * Buffer containing the parameters of the client message.
     * Own.
     */
    RBuf8 iParams;

    /**
     * Pointer to an outstanding user interaction.
     * Not own.
     */
    CBluetoothNotification* iNotification;
    
    // will be set to ETrue if the pairing 
    // was initiated by us.
    TBool iLocallyInitiated;
    
    // contain a number to use in the pairing dialog.
    TBuf<8> iDialogNumeric;
    
    // the dialog type.
    TBluetoothDialogParams::TBTDialogType iDialog;
    
    // the resource id to be loaded to the dialog
    TBTDialogResourceId iDialogResource;
    
    // the address of the device with which the pairing is performed.
    TBTDevAddr iRemote;
    
    // contains the minimum requirements for pin 
    // code length. -1 indicates this is not PIn code pairing.
    TInt iMinPinLength;
    
    // Contains the device name provided in params
    TBTDeviceName iCurrentDeviceName;
    
    TNotifierState iState;
    
    // Defines if the check box in the dialog is checked or not.
    TBool   iCheckBoxState;
    
    TBool   iAcceptPairingResult;
    
    BTUNITTESTHOOK

    };

#endif // BTNOTIFPAIRNOTIFIER_H
