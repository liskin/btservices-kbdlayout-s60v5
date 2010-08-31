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

#include "btnotifpairnotifier.h"
#include "btnotifclientserver.h"
#include <btextnotifiers.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifierspartner.h>
#endif
#include "btnotifconnectiontracker.h"
#include "btnotifsecuritymanager.h"
#include "btnotificationmanager.h"
#include "bluetoothnotification.h"
#include "btnotifserver.h"
#include "bluetoothtrace.h"


/**  Format syntax for numeric comparison value. */
_LIT( KNumCompFormat, "%06u" );
/**  Format syntax for passkey display value. */
_LIT( KPassKeyFormat, "%06u" );


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor.
// ---------------------------------------------------------------------------
//
CBTNotifPairNotifier::CBTNotifPairNotifier(
        CBTNotifSecurityManager& aParent )
:   iParent( aParent )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTNotifPairNotifier* CBTNotifPairNotifier::NewL(
        CBTNotifSecurityManager& aParent )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    CBTNotifPairNotifier* self = new( ELeave ) CBTNotifPairNotifier( aParent );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CBTNotifPairNotifier::~CBTNotifPairNotifier()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    iParams.Close();
    if ( !iNotifierMessage.IsNull() )
        {
        iNotifierMessage.Complete( KErrServerTerminated );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Handle a notifier request for pairing with a remote device.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::StartPairingNotifierL(const RMessage2& aMessage )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    
    TInt uid = aMessage.Int0();
    TInt opCode = aMessage.Function();
    BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST,"[BTNotif]:Opcode: %d",opCode);
    
    if ( (!iNotifierMessage.IsNull()) && 
         (opCode != EBTNotifCancelNotifier )&&(opCode != EBTNotifUpdateNotifier))
        {
        // todo: do we allow concurrent pairing?
        BOstrace0(TRACE_DEBUG,DUMMY_DEVLIST,"[BTNotif]:We are busy");
        User::Leave(KErrServerBusy );
        }
    
    if(opCode == EBTNotifCancelNotifier){
        CancelPairingNotifierL(uid);
        aMessage.Complete(KErrNone);
        return;
    }
    
    // Store the parameters locally, we need them later again.
    iParams.CreateL( aMessage.GetDesLengthL( EBTNotifSrvParamSlot ) );
    aMessage.ReadL( EBTNotifSrvParamSlot, iParams );
    iNotifierMessage = aMessage;
    
    // Read the notifier parameters
    ParseNotifierReqParamsL();

    if(opCode ==EBTNotifUpdateNotifier ){
        UpdatePairingNotifierL(uid,iParams);
        aMessage.Complete(KErrNone);
        return;
    }
    
    const CBtDevExtension* dev = iParent.BTDevRepository().Device(iRemote);
    if(dev)
        {
        if (!iLocallyInitiated && dev->Device().GlobalSecurity().Banned() )
            {
            // If the device is banned and pairing is not locally initiated
            // then we reject.
            BOstrace0(TRACE_DEBUG,DUMMY_DEVLIST,"[BTNotif]:Device is banned");
            iNotifierMessage.Complete( KErrCancel );
            return;
            }
        if (iLocallyInitiated && dev->Device().GlobalSecurity().Banned())
            {
            // Remove the banned device from the blocking history
            iParent.ConnectionTracker().UpdateBlockingHistoryL(&dev->Device(),ETrue);
            iParent.BlockDevice(dev->Addr(),EFalse);
            }
        if(0 != dev->Device().FriendlyName().Length()&& dev->Device().IsValidFriendlyName())
            {
            // If we have a friendly name use it for the UI
            iCurrentDeviceName = dev->Device().FriendlyName();
            }
        else
            {
            // We don't have any friendly name then chek if we have the device name
            // otherwise use the Alias
            if(0 >= iCurrentDeviceName.Length())
                {
                iCurrentDeviceName = dev->Alias();
                }
            }
        }
    else
        {
        //If we didn't get a name then we make one from the Bluetooth device address
        if(0 >= iCurrentDeviceName.Length())
            {
            iRemote.GetReadable(iCurrentDeviceName);
            }
        }
    // If this is an incoming pairing, we first ask the user to accept it.
    if( !iLocallyInitiated  )
        {
        User::LeaveIfError(iParent.SetPairObserver(iRemote,ETrue));
        StartAcceptPairingQueryL();
        }
    else
        {
        TInt uid = iNotifierMessage.Int0();
        if(uid == KBTNumericComparisonNotifierUid.iUid)
            {
            StartPairingUserInputL();
            }
        else
            {
            TBTPinCode pinCode;
            if ( iMinPinLength > -1 )
                {
                // Legacy Pin pairing. Check if a pin code is already available
                // in pairing manager for this device:
                iParent.GetPinCode( pinCode, iRemote, iMinPinLength );
                }
            if ( pinCode().iLength > 0 )        
                {
                // a pin is ready for this pairing.
                // write it back to client (stack)
                TInt err = iNotifierMessage.Write( EBTNotifSrvReplySlot, pinCode );
                iNotifierMessage.Complete( err );
                }
            else
                {
                // no pin code is available or a pin code does not meet
                // the security requirement.
                // User need to interact:
                StartPairingUserInputL();
                }
            }
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// Update a notifier, update the outstanding dialog if the notifier request 
// is currently being served.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::UpdatePairingNotifierL( TInt aUid, const TDesC8& aParams )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    (void) aUid;
    TBTNotifierUpdateParams2 params;    // Enough for reading the base class type parameter
    TPckgC<TBTNotifierUpdateParams2> paramsPckg( params );
    paramsPckg.Set( aParams );
    if( paramsPckg().Type() == TBTNotifierUpdateParams2::EPasskeyDisplay )
        {
        // Paskey display update - keypress on remote device.
        }
    else
        {
        // name update
        TBTDeviceNameUpdateParams nameUpdate;
        TPckgC<TBTDeviceNameUpdateParams> nameUpdatePckg( nameUpdate );
        nameUpdatePckg.Set( aParams );
        // The result means result of conversion to unicode
        if( !nameUpdatePckg().Result() )
            {
            // Check first if we already have a friendly name
            const CBtDevExtension* dev = iParent.BTDevRepository().Device(iRemote);
            if(dev)
                {
                if(0 != dev->Device().FriendlyName().Length()&& dev->Device().IsValidFriendlyName())
                    {
                    return;
                    }
                // We don't have a friendly name then use this name
                iCurrentDeviceName = nameUpdatePckg().DeviceName();
                if(0 == iCurrentDeviceName.Length())
                    {
                    // The new name is empty then use the Alias
                    iCurrentDeviceName = dev->Alias();
                    }
                }
            else
                {
                // We don't have a friendly name then use this name
                iCurrentDeviceName = nameUpdatePckg().DeviceName();
                if(0 == iCurrentDeviceName.Length())
                    {
                    // The new name is empty then use the Alias
                    iRemote.GetReadable(iCurrentDeviceName);
                    }
                }
            if( iNotification )
                {
                // Update the dialog with the new name. It is up to the dialog to 
                // determine the validity (in case another dialog is shown).
                iNotification->Update(iCurrentDeviceName);
                }
            }
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Cancel a request, dismiss the outstanding dialog if the notifier request 
// is currently being served.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::CancelPairingNotifierL( TInt aUid )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if(  iNotifierMessage.Int0() == aUid )
        {
        if( iNotification )
            {
            // Cancel the user query
            // This will also unregister us from the notification.
            TInt err = iNotification->Close();
            NOTIF_NOTHANDLED( !err )
            iNotification = NULL;
            }
        // todo: Any bonding requester needs to be informed.
        // Currently we don't show any "Unable to pair" note
        // so no need to inform any bonding requester.
        if ( !iNotifierMessage.IsNull() )
            {
            iNotifierMessage.Complete( KErrCancel );
            }
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// Handle a result from a user query.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::MBRDataReceived( CHbSymbianVariantMap& aData )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if(aData.Keys().MdcaPoint(0).Compare(_L("actionResult")) == 0)
        {
        TInt val = *(static_cast<TInt*>(aData.Get(_L("actionResult"))->Data()));
        if(val)
            {
                iAcceptPairingResult = ETrue;
            }
        else
            {
                iAcceptPairingResult = EFalse;
            }
        }
    else if(aData.Keys().MdcaPoint(0).Compare(_L("checkBoxState")) == 0)
        {
        iCheckBoxState = *(static_cast<TInt*>(aData.Get(_L("checkBoxState"))->Data()));
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// The notification is finished.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::MBRNotificationClosed( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    // First unregister from the notification, so we can already get the next one.
    iNotification->RemoveObserver();
    iNotification = NULL;
    TRAP_IGNORE( NotificationClosedL( aError, aData ) );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// Request a user input for the outstanding pairing request.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::StartPairingUserInputL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    PrepareNotificationL( iDialog, iDialogResource );
    iState = EPairingInputConfirm;
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Process the user input and complete the outstanding pairing request.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::CompletePairingNotifierL( TInt aError, TBool aResult,
    const TDesC8& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    if ( iNotifierMessage.IsNull() )
        {
        // Request not anymore active -> ignore
        return;
        }
    TInt err = aError;
    TPtrC8 resultData(KNullDesC8);
    TBTPinCode pinCode;
    TPckgBuf<TBool> userAcceptance;
    TInt uid = iNotifierMessage.Int0();
 
    if( !err )
        {
        // The returned data is the entered passkey.
        const CBtDevExtension* dev = iParent.BTDevRepository().Device(iRemote);
        if(dev)
            {
            iParent.ConnectionTracker().UpdateBlockingHistoryL(&dev->Device(),aResult);
            }
         if( uid == KBTNumericComparisonNotifierUid.iUid )
            {
            // Numeric comparison needs the boolean result passed back.
            userAcceptance() = aResult;
            resultData.Set( userAcceptance );
            }
        if( aResult )
            {
            if( uid == KBTManPinNotifierUid.iUid 
                || uid == KBTPinCodeEntryNotifierUid.iUid )
                {
                // Check the passkey entered by the user.
                // The length of the returned data equals the number of characters
                // entered by the user.
                // Check that the passkey length do not exceed the maximum allowed size
                TInt pinCodeLength = aData.Length();
                if(pinCodeLength > KHCIPINCodeSize)
                    {
                        pinCodeLength = KHCIPINCodeSize;
                    }
                pinCode().iLength = pinCodeLength;
                // Check that the length of the passkey meets the minimum 
                // required pin code length
                if( pinCodeLength >= iMinPinLength )
                    {
                    for( TInt i = 0; i < pinCodeLength; i++ )
                        {
                        pinCode().iPIN[i] = aData[i];
                        }
                    resultData.Set( pinCode );
                    }
                else
                    {
                    // shouldn't happen since the length is checked in the dialog
                    err = KErrCompletion;
                    }
                }
            }
        else
            {
            err = KErrCancel;
            }
        }
    // Complete the message with the result, and result data if any.
    if ( !err && resultData.Length() )
        {
        err = iNotifierMessage.Write( EBTNotifSrvReplySlot, resultData );
        }
    if(err && (uid == KBTNumericComparisonNotifierUid.iUid))
        {
        // We need to reject the numeric comparaison otherwise
        // the link will remain active
        userAcceptance() = aResult;
        resultData.Set( userAcceptance );
        err = iNotifierMessage.Write( EBTNotifSrvReplySlot, resultData );
        }
    iNotifierMessage.Complete( err );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// Ask the user to allow incoming pairing.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::StartAcceptPairingQueryL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    PrepareNotificationL( TBluetoothDialogParams::EUserAuthorization, EAuthorization );
    iState = EIncomingPairingAcceptconfirm;
    // if rejected, the client message is completed in CompleteAcceptPairingQueryL
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// The user was asked to accept an incoming pairing. Process and proceed. 
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::CompleteAcceptPairingQueryL( TInt aError)
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TInt err = aError;
    TBool proceed = EFalse;
    
    if( !err )
        {
        const CBtDevExtension* dev = iParent.BTDevRepository().Device(iRemote);
        if(dev)
            {
            proceed = iParent.ConnectionTracker().UpdateBlockingHistoryL(&dev->Device(),iAcceptPairingResult);
            }
        if( iAcceptPairingResult )
            {
            // User accepted, continue to show pairing query.
            // Trust the device
            if(iCheckBoxState){
            iParent.TrustDevice(iRemote);
            }
            StartPairingUserInputL();
            }
        else
            {
            if( proceed && iCheckBoxState )
                {
                //ask to block the device.
                iParent.BlockDevice(iRemote,proceed);
                }
            err = iParent.SetPairObserver(iRemote,EFalse);
            err = KErrCancel; // We need to complete the pairing request here
            }
        }
    if( err )
        {
        // The user denied the connection, or something else prevented completion.
        CompletePairingNotifierL( err, EFalse, KNullDesC8 );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::ParseNotifierReqParamsL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // Reset to make sure all vars contain initial values.
    iLocallyInitiated = EFalse;
    iDialogNumeric.Zero();
    iDialog = TBluetoothDialogParams::EInvalidDialog;
    iDialogResource = ENoResource;
    iRemote = TBTDevAddr();
    iMinPinLength = -1;
    
    TInt uid = iNotifierMessage.Int0();
    // Reset for other pairing modes than PIN code.
    // Determine the notifier type by the length of the parameter buffer
    if( uid == KBTManPinNotifierUid.iUid || uid == KBTPinCodeEntryNotifierUid.iUid )
        {
        if ( uid == KBTManPinNotifierUid.iUid ) 
            {
            ParseLegacyPinCodeReqParamsL( iLocallyInitiated, iMinPinLength, iRemote );
            }
        else
            {
            ParsePinCodeReqParamsL( iLocallyInitiated, iMinPinLength, iRemote );
            }
        iDialog = TBluetoothDialogParams::EInput;
        iDialogResource = EPinInput;
        }
    else if( uid == KBTNumericComparisonNotifierUid.iUid )
        {
        ParseNumericCompReqParamsL( iLocallyInitiated, iDialogNumeric, iRemote );
        iDialog = TBluetoothDialogParams::EQuery;
        iDialogResource = ENumericComparison;
        }
    else if( uid == KBTPasskeyDisplayNotifierUid.iUid )
        {
        ParsePasskeyDisplayReqParamsL( iLocallyInitiated, iDialogNumeric, iRemote );
        iDialog = TBluetoothDialogParams::EQuery;
        iDialogResource = EPasskeyDisplay;
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing using pin query.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::ParseLegacyPinCodeReqParamsL( TBool& aLocallyInitiated,
    TInt& aMinPinLength, TBTDevAddr& aAddr )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TBTPasskeyNotifierParams params;
    TPckgC<TBTPasskeyNotifierParams> paramsPckg( params );
    paramsPckg.Set( iParams );
    aLocallyInitiated = paramsPckg().iLocallyInitiated;
    aMinPinLength = paramsPckg().iPasskeyMinLength;
    aAddr = paramsPckg().iBDAddr;
    iCurrentDeviceName = paramsPckg().iName;
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing using pin query.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::ParsePinCodeReqParamsL( TBool& aLocallyInitiated,
    TInt& aMinPinLength, TBTDevAddr& aAddr )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TBTPinCodeEntryNotifierParams params;
    TPckgC<TBTPinCodeEntryNotifierParams> paramsPckg( params );
    paramsPckg.Set( iParams );
    aLocallyInitiated = paramsPckg().LocallyInitiated();
    aMinPinLength = paramsPckg().PinCodeMinLength();
    aAddr = paramsPckg().DeviceAddress();
    iCurrentDeviceName = paramsPckg().DeviceName();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing using numeric comparison.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::ParseNumericCompReqParamsL( TBool& aLocallyInitiated,
    TDes& aNumVal, TBTDevAddr& aAddr )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TBTNumericComparisonParams params;
    TPckgC<TBTNumericComparisonParams> paramsPckg( params );
    paramsPckg.Set( iParams );
    aLocallyInitiated = paramsPckg().LocallyInitiated();
    TBTNumericComparisonParams::TComparisonScenario scenario =
                paramsPckg().ComparisonScenario();
    aNumVal.Format( KNumCompFormat, paramsPckg().NumericalValue() );
    aNumVal.Insert(3,_L(" "));
    aAddr = paramsPckg().DeviceAddress();
    iCurrentDeviceName = paramsPckg().DeviceName();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Parse the parameters of a request for pairing using passkey display.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::ParsePasskeyDisplayReqParamsL( TBool& aLocallyInitiated,
    TDes& aNumVal, TBTDevAddr& aAddr )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TBTPasskeyDisplayParams params;
    TPckgC<TBTPasskeyDisplayParams> paramsPckg( params );
    paramsPckg.Set( iParams );
    aLocallyInitiated = paramsPckg().LocallyInitiated();
    aNumVal.Format( KPassKeyFormat, paramsPckg().NumericalValue() );
    aAddr = paramsPckg().DeviceAddress();
    iCurrentDeviceName = paramsPckg().DeviceName();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Get and configure a notification.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::PrepareNotificationL( TBluetoothDialogParams::TBTDialogType aType,
    TBTDialogResourceId aResourceId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iNotification = 
            iParent.ConnectionTracker().NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    iNotification->SetNotificationType( aType, aResourceId );
    TInt err = iNotification->SetData( TBluetoothDeviceDialog::EDeviceName, iCurrentDeviceName );
    NOTIF_NOTHANDLED( !err )
    TBTDeviceName name;
    iRemote.GetReadable(name);
    err = iNotification->SetData( TBluetoothDialogParams::EAddress, name );
    NOTIF_NOTHANDLED( !err )
    const CBtDevExtension* dev = iParent.BTDevRepository().Device(iRemote);
    TInt classofdevice = 0;
    if(dev)
        {
            if(dev->Device().IsValidDeviceClass())
                {
                classofdevice = dev->Device().DeviceClass().DeviceClass();
                }
        }
    err = iNotification->SetData( (TInt) TBluetoothDeviceDialog::EDeviceClass, 
                classofdevice );
    if( iDialogNumeric.Length() )
        {
        err = iNotification->SetData( 
        TBluetoothDeviceDialog::EAdditionalDesc, iDialogNumeric );
        }
    err = iNotification->SetData( TBluetoothDialogParams::EDialogTitle, TBluetoothDialogParams::EPairingRequest);
    iCheckBoxState = ETrue;
    iAcceptPairingResult = EFalse;
    iParent.ConnectionTracker().NotificationManager()->QueueNotificationL( iNotification);
    NOTIF_NOTHANDLED( !err )
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// The notification is finished, handle the result.
// ---------------------------------------------------------------------------
//
void CBTNotifPairNotifier::NotificationClosedL( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    // Read the result.
    TPckgC<TBool> result( EFalse );
    result.Set( aData.Ptr(), result.Length() ); // Read the part containing the result
    // Set a pointer descriptor to capture the remaining data, if any.
    TPtrC8 dataPtr( aData.Mid( result.Length() ) );

    if ( iState == EIncomingPairingAcceptconfirm )
        {
        CompleteAcceptPairingQueryL(aError);
        }
    else
        {
        CompletePairingNotifierL( aError, result(), dataPtr );
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }




