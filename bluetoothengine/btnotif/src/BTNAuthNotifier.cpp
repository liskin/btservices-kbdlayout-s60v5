/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implements authorisation notifier class.
*
*/


// INCLUDE FILES
#include <StringLoader.h>    // Localisation stringloader
#include <BTNotif.rsg>       // Own resources
#include "btnauthnotifier.h" // Own class definition
#include "btNotifDebug.h"    // Debugging macros
#include <btextnotifiers.h>
#include <AknMediatorFacade.h> // Cover UI
#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>
#include <btengconstants.h>
#include <btengsettings.h>
#include <e32cmn.h>
#include <btotgpairpub.inl>
#include <btengprivatecrkeys.h>
#include <e32property.h>
#include <e32const.h>
#include <e32des8.h>
#include <e32cmn.h>
#include <utf.h> // Unicode character conversion utilities
#include <btengutil.h>
#ifdef __SERIES60_HELP
#include <hlplch.h>
#include <csxhelp/bt.hlp.hrh> // The bt hrh info is needed, for help launching
#endif
#include "btnotifnameutils.h"


//
// SDP UUID Constants - Short form
// Taken from Bluetooth Profile specification v1.1
// These are used when registering the service to
// local SDP database and when searching the service
// information from remote device.
const TUint KBTSdpDun                   = 0x1103;
const TUint KBTSdpGenericTelephony      = 0x1204;
const TUint KBTSdpFax                   = 0x1111;
const TUint KBTSdpObjectPush            = 0x1105;
const TUint KBTSdpFileTransfer          = 0x1106;
const TUint KBTSdpHeadSet               = 0x1108;
const TUint KBTSdpGenericNetworking     = 0x1201;
const TUint KBTSdpBasicImaging          = 0x111b;

/**  Identification for active object */
const TInt KBTNotifAuthNotifierLockReq = 10;

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTAuthNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTAuthNotifier* CBTAuthNotifier::NewL()
    {
    CBTAuthNotifier* self = new (ELeave) CBTAuthNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CBTAuthNotifier::CBTAuthNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTAuthNotifier::CBTAuthNotifier()
    {
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTAuthNotifier::~CBTAuthNotifier()
    {
    }

// ----------------------------------------------------------
// CBTAuthNotifier::RegisterL
// ----------------------------------------------------------
//
CBTAuthNotifier::TNotifierInfo CBTAuthNotifier::RegisterL()
    {
    iInfo.iUid=KBTManAuthNotifierUid;
    iInfo.iChannel=KBTAuthorisationChannel;
    iInfo.iPriority=ENotifierPriorityVHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTAuthNotifier::GetParamsL
// Initialize parameters and check if device is already
// in registry. Jump to RunL as soon as possible.
// ----------------------------------------------------------
//
void CBTAuthNotifier::GetParamsL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::GetParamsL()"));

    if( !iMessage.IsNull())
        {
        User::Leave(KErrInUse);
        }
    else if ( AutoLockOnL() )
        {
        // The phone is locked, access denied.
        // Write results back to caller and complete message.	
		CompleteMessage(EFalse, KErrNone);	
        return;
        }
    
	TBTAuthorisationParams param;
 	TPckgC<TBTAuthorisationParams> pckg(param);
 	pckg.Set(aBuffer);

 	iServiceUid = pckg().iUid.iUid;  // Pick up service uid from message
 	iBTAddr = pckg().iBDAddr;
 	if ( OtherOutgoPairing( iBTAddr ) )
        {
        // We won't allow connection request from another device during outgoing pairing:
        FLOG(_L("[BTNOTIF]\t CBTAuthNotifier: outgoing pair in progress, reject request from other device"));
        CompleteMessage(KErrCancel);
        return;
        }
 	
    iMessage = aMessage;
    iReplySlot = aReplySlot;
 	
    // create iDevice so that the name won't be lost if the device does
    // not exist in registry.
    iDevice = CBTDevice::NewL( iBTAddr );
    BtNotifNameUtils::SetDeviceNameL(pckg().iName, *iDevice);

    if ( !iNotifLockProp.Handle() )
        {
        User::LeaveIfError( iNotifLockProp.Attach( 
                KPSUidBluetoothEnginePrivateCategory, KBTNotifierLocks ) );
        }
    iLockActive = CBTNotifActive::NewL( this, KBTNotifAuthNotifierLockReq, CActive::EPriorityStandard );

    CheckAndSubscribeNotifLocks();
    
    if ( !iSuspended )
        {
        // Check if device is in the registry, function of notifier base  
        GetDeviceFromRegL( iBTAddr );
        }

#ifdef _DEBUG
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::GetParamsL() Executing authorisation..."));
    TBuf<12> deviceAddressString;
    pckg().iBDAddr.GetReadable(deviceAddressString);
    FTRACE(FPrint(_L("[BTNOTIF]\t  BT Address: %S"), &deviceAddressString));
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTAuthNotifier::GetParamsL Service Uid: %d = 0x%X"), iServiceUid, iServiceUid ));
#endif
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::GetParamsL() completed"));
    }

// ----------------------------------------------------------
// CBTAuthNotifier::UpdateL
// Notifier update. Stores the received bluetooth
// device name into registry and show it on screen.
// ----------------------------------------------------------
//
TPtrC8 CBTAuthNotifier::UpdateL(const TDesC8& aBuffer)
    {
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::UpdateL()"));

    TBTNotifierUpdateParams params; // Contains iName and iResult (name request)
    TPckgC<TBTNotifierUpdateParams> pckg(params);
    pckg.Set(aBuffer);

    FTRACE(FPrint(_L("[BTNOTIF]\t CBTAuthNotifier::UpdateL - Name: '%S' length: %d"), &pckg().iName, pckg().iName.Length() ));


    // If the device name request was successful and if new name is valid, show the new name.
    if (pckg().iResult == KErrNone)
        {
        BtNotifNameUtils::SetDeviceNameL(pckg().iName, *iDevice); // Override possible previous device name

        // Show new prompt for dialog if it is still on the screen
        if ( !iDevice->IsValidFriendlyName() && iDevice->IsValidDeviceName())
            {
            // Create new prompt string with new device name
            HBufC* stringholder = StringLoader::LoadL( iStrResourceId, BTDeviceNameConverter::ToUnicodeL(iDevice->DeviceName()));
            CleanupStack::PushL( stringholder );
            iNotifUiUtil->UpdateQueryDlgL( *stringholder );
            iNotifUiUtil->UpdateCoverUiL( iDevice->DeviceName() );
            CleanupStack::PopAndDestroy();  // stringholder
            }
        }
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::UpdateL() completed"));
    TPtrC8 ret(KNullDesC8);
    return (ret);
    }

// ----------------------------------------------------------
// CBTAuthNotifier::RequestCompletedL
// Gets called when P&S key notifies change
// ----------------------------------------------------------
void CBTAuthNotifier::RequestCompletedL( CBTNotifActive* aActive, TInt aId, TInt aStatus )
    {
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::RequestCompletedL()"));
    ASSERT( aId == KBTNotifAuthNotifierLockReq );
    (void) aActive;
    (void) aId;
    
    if ( aStatus == KErrNone )
        {
        TBool prevSuspend = iSuspended;
        // check the latest lock status and subscribe to further lock event:
        CheckAndSubscribeNotifLocks();
        if ( prevSuspend && !iSuspended )
            {
            // Check if device is in the registry, function of notifier base  
            GetDeviceFromRegL( iBTAddr ); 
            }
        else if ( iSuspended && !(iNotifUiUtil->IsQueryReleased()) )
            {
            iNotifUiUtil->DismissDialog();
            }
        }
    else
        {
        HandleError( aActive, KBTNotifAuthNotifierLockReq, iLockActive->RequestStatus().Int() );
        }
        
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::RequestCompletedL() completed"));
    
    }

// ----------------------------------------------------------
// CBTAuthNotifier::HandleError
// ----------------------------------------------------------
void CBTAuthNotifier::HandleError( CBTNotifActive* aActive, TInt aId, TInt aError )
    {
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTAuthNotifier::HandleError() error = %d"), aError ));
    (void) aActive;
    (void) aId;
    (void) aError;
    }

// ----------------------------------------------------------
// CBTAuthNotifier::DoCancelRequest
// Root caller is CBTNotifActive::CancelRequest(), which calls 
// CActive::Cancel that calls DoCancel() if request is active.
// ----------------------------------------------------------
void CBTAuthNotifier::DoCancelRequest( CBTNotifActive* aActive, TInt aId )
    {
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::DoCancel() >>"));
    ASSERT( aId == KBTNotifAuthNotifierLockReq );
    (void) aActive;
    (void) aId;
    
    iNotifLockProp.Cancel();
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::DoCancel() <<"));
    }
	
// ----------------------------------------------------------
// CBTAuthNotifier::Cancel
// Not Active Object's cancel, but notifier deactivation
// Release all own resources (member variables)
// ----------------------------------------------------------
//
void CBTAuthNotifier::Cancel()
    {
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::Cancel()"));
    
    iLockActive->CancelRequest();
    delete iLockActive;
    iLockActive = NULL;    
    iNotifLockProp.Close();
        
	CBTNotifierBase::Cancel(); 
    
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::Cancel() completed"));
    }
   
void CBTAuthNotifier::HandleGetDeviceCompletedL(const CBTDevice* aDev)
	{
	FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::HandleGetDeviceCompleted()"));	
    (void) aDev;
    if ( iSuspended)
        {
        FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::HandleGetDeviceCompleted, still suspended, wait"));
        return;
        }
        
    TBool reqAllowed = IsJustWorksPaired( iDevice->AsNamelessDevice() ) ? 
        IsUserAwarePaired( iDevice->AsNamelessDevice() ) : ETrue ;
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTAuthNotifier, reqAllowed %d" ), reqAllowed ) );
    if ( !reqAllowed )
        {
        // Do not grant access to device that is not paired in Just Works mode without
        // user awareness, e.g. pairing due to an incoming connection request from a 
        // headset.
        CompleteMessage(KErrAccessDenied);
        return;
        }
    
    iStrResourceId = R_BT_AUTHORISATION_NAME;

    // Select authorisation state for further use (dialogue selection)
    switch( iServiceUid )
        {
        case KBTSdpObjectPush:
        case KBTSdpBasicImaging: 
            iAuthState = EBTObexAuthorisation;
            iStrResourceId = R_BT_RECEIVE_MESSAGE;
            iCoverUiDlgId = ECmdShowReceiveMessageFromDeviceDlg;
            
            if( IsUserAwarePaired( iDevice->AsNamelessDevice() ) )
                {
                iAuthState = EBTObexAuthForPairedDevice;
                iStrResourceId = R_BT_RECEIVE_MESSAGE_PAIRED;
                iCoverUiDlgId = ECmdShowReceiveMessageFromPairedDeviceDlg; 
                }
            break;
        case KBTSdpFax:
        case KBTSdpDun:
        case KBTSdpFileTransfer:
        case KBTSdpHeadSet:
        case KBTSdpGenericTelephony:
        case KBTSdpGenericNetworking:
            if ( iDevice->GlobalSecurity().NoAuthorise() )
                {
                iAuthState = EBTAutoAuthorisation;
                }
            else
                {
                iAuthState = EBTNormalAuthorisation;
                }
            iCoverUiDlgId = ECmdShowAcceptConnRequestDlg;
            break;
        default:
            { // check if device is authorized
            if ( iDevice->GlobalSecurity().NoAuthorise() )
                {
                iAuthState = EBTAutoAuthorisation;
                }
            else
                {
                iAuthState = EBTAutoAuthorisationNotAuthorisedOnPhone;                        
                }
            break;
            }
        }
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTAuthNotifier::HandleGetDeviceComplete() Complete. iAuthState = %d"), iAuthState ));    
    ShowAuthoQueryL();
	}

// ----------------------------------------------------------
// CBTAuthNotifier::GetByPassAudioNotifier
// is bypassing this audio connection query needed and if it is, will the
// query be automatically accepted or rejected.
//
// This is used for bypassing the notifiers of the 2nd audio connection, so
// the the user needs to reply to the incoming audio link only once.
// ----------------------------------------------------------
//    
TBool CBTAuthNotifier::GetByPassAudioNotifier(const TBTDevAddr& aDeviceAddress,TBool& aAccept)
	{
	FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::GetByPassAudioNotifier()"));	
		TBuf8<48> buf;
		
		RProperty::Define(KPSUidBluetoothEnginePrivateCategory,KBTAuthorInfoPerDevice, RProperty::EByteArray );
		RProperty::Get(KPSUidBluetoothEnginePrivateCategory,KBTAuthorInfoPerDevice,buf);

		if(buf.Length()>2)
			{
			// accept status of last attempt			
			aAccept= (buf[0]=='1');

			// last address						
			TBuf<12> oldAddrString;
			oldAddrString.Copy(buf.Mid(2,12));
			TBTDevAddr lastAddress;
			lastAddress.SetReadable(oldAddrString);
			
			// last time
			TInt64 num(0);
			TLex8 lex;			
			lex=buf.Mid(15);	
			TBuf<32> tmp_debug;
			tmp_debug.Copy(buf.Mid(15));
				
			lex.Val( num );
			TTime lastConnectionTime( num );
			TTime now;			
			now.UniversalTime();			

			TBuf16<48> buf2;
			buf2.Copy(buf);
			
			FTRACE(FPrint(_L("[BTNOTIF]\t CBTAuthNotifier::GetByPassAudioNotifier() Complete same_address=%d Time_ok=%d last_reply=%d"),lastAddress==aDeviceAddress,now>=lastConnectionTime && now <lastConnectionTime+ TTimeIntervalSeconds(5),aAccept));	
			return lastAddress==aDeviceAddress && (now>=lastConnectionTime && now <lastConnectionTime+ TTimeIntervalSeconds(5) );		
			}
		FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::GetByPassAudioNotifier() Complete - empty key"));				
		return EFalse;
	}
// ----------------------------------------------------------
// CBTAuthNotifier::MemorizeCurrentAudioAttempt
// Store the information about this audio connection attempt 
// and what user replied to that. This is used by
// GetBypassAudio Notifier.
// ----------------------------------------------------------
//    
    			
void CBTAuthNotifier::MemorizeCurrentAudioAttempt(const TBool aAccept,const TBTDevAddr& aDeviceAddress)    			
	{
	FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::MemorizeCurrentAudioAttempt()"));	
	TBuf8<48> buf;
	buf.Zero();
	buf.AppendNum(aAccept!=EFalse);//now 0/1 only.
	buf.Append(';');
	
	TBuf<12> a;
	aDeviceAddress.GetReadable(a);
	for(TInt t=0;t<12;t++)
		buf.Append(a[t]);
	buf.Append(';');		

	TTime writeTime;
	writeTime.UniversalTime();		
	buf.AppendNum(  writeTime.Int64()  );
	
	RProperty::Set(KPSUidBluetoothEnginePrivateCategory,KBTAuthorInfoPerDevice,buf);
	FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::MemorizeCurrentAudioAttempt() complete"));	
	}


// ----------------------------------------------------------
// CBTAuthNotifier::ShowAuthoQueryL
// Ask user's response on authorization query
// ----------------------------------------------------------
// 
void CBTAuthNotifier::ShowAuthoQueryL()
    {
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::ShowAuthoQueryL"));
    
    if ( iAuthState == EBTAutoAuthorisation )
        {
        // device alreay set trusted now. allow connection automatically       
        CompleteMessage(ETrue, KErrNone);        
        FLOG(_L("[BTNOTIF]\t CBTAuthNotifier, device authorized already, allow connection automatically" ));
        return;
        }

    TBool reply;
    if ( GetByPassAudioNotifier(iDevice->BDAddr(),reply) )
        {
        FTRACE(FPrint(_L("[BTNOTIF]\t CBTAuthNotifier::ShowAuthoQueryL bypassing notifier with reply: %d"),reply));          
        CompleteMessage(reply, KErrNone);
        return;
        }

    // Not asking user's response if auth request is for: profile HFP/HSP/A2DP/AVRCP, and
    // there is already existing connection to one of those profiles from the same device.
    // 
    TBool connectStatus = IsExistingConnectionToAudioL( iDevice->BDAddr() );  
    // if iAuthState==EBTAutoAuthorisationNotAuthorised and device is connected it means it tries several connections
    // so we just approve this. Also if connected & EBTAutoAuthorisation.
    //                              
    if(connectStatus && ( iAuthState==EBTAutoAuthorisationNotAuthorisedOnPhone))                                         
        {
        FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::Automatic authorisation ()"));
        //Antomatic authorisation without asking user           
        CompleteMessage(ETrue, KErrNone);   
        FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::ShowAuthoQueryL() completed-"    ));     
        return;                 
        }

    // check if we're showing bt off query
    TInt queryValue=EBTQueryOff;
    RProperty::Get(KPSUidBluetoothEnginePrivateCategory,KBTTurnBTOffQueryOn,queryValue);                
    if( queryValue == EBTQueryOn )
        {
        CompleteMessage(KErrCancel);
        return;
        }

    TBTDeviceName tempDeviceName; 
    BtNotifNameUtils::GetDeviceDisplayName(tempDeviceName, iDevice);
    HBufC* stringholder = StringLoader::LoadLC( iStrResourceId, tempDeviceName);
    TInt keypress = iNotifUiUtil->ShowQueryL( *stringholder, R_BT_AUTHORISATION_QUERY, 
            iCoverUiDlgId, tempDeviceName, CAknQueryDialog::EConfirmationTone );
    CleanupStack::PopAndDestroy();  // stringholder
    // If this notifier is cancelled by the caller, no need to perform the rest operation:
    if ( iSuspended || iMessage.IsNull() )
        {
        return;
        }
    
    if( keypress )  // User has accepted the dialog
        {
        if( iDevice && !iDevice->IsValidLinkKey() )
            {               
            TTime now;
            now.UniversalTime();
            TBuf8<32> buf;
            buf.AppendNum(  now.Int64()  );             
            RProperty::Set(KPSUidBluetoothEnginePrivateCategory,KBTConnectionTimeStamp,buf);                
            }
        
        MemorizeCurrentAudioAttempt(ETrue,iDevice->BDAddr() );
        CompleteMessage( ETrue, KErrNone );
        }
    else // User has rejected the dialog.
        {
        iLockActive->CancelRequest();
        DoRejectAuthorizationL();
        }
    }

// ----------------------------------------------------------
// CBTAuthNotifier::DoRejectAuthorizationL
// Handle query threshold and block after user rejects authorization
// ----------------------------------------------------------
//
void CBTAuthNotifier::DoRejectAuthorizationL()
    {
    FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::DoRejectAuthorizationL()"));
    
    CheckAndHandleQueryIntervalL();
    if( iMessage.IsNull() )
        {
        return; // No need to continue.
        }

    MemorizeCurrentAudioAttempt(EFalse,iDevice->BDAddr());
    
    //Logic: query block only at the second time for the same paired device
    //       query block everytime after rejection for non-paired device.
    if( IsPaired( iDevice->AsNamelessDevice() ) )
        {
        FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::DoRejectAuthorizationL() rejected a paired device."));            
        
        TBuf<2*KBTDevAddrSize> tmp; //2 hex digits per byte
        RProperty::Get(KPSUidBluetoothEnginePrivateCategory, KBTBlockDevAddr, tmp);
        TBTDevAddr priorDeviceaddress;
        priorDeviceaddress.Reset();
        priorDeviceaddress.SetReadable(tmp);
        
        if(priorDeviceaddress != iDevice->BDAddr() )                            
            {
            FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::DoRejectAuthorizationL() NOT the prior rejected device, no block query."));           
            
            // The last device was not current device,
            // so the last device is updated to be current device.                  
            iDevice->BDAddr().GetReadable(tmp);
            RProperty::Set(KPSUidBluetoothEnginePrivateCategory, KBTBlockDevAddr, tmp );

            // Complete message and free resources
            CompleteMessage(EFalse, KErrNone);
            FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::DoRejectAuthorizationL() completed"));
            return;
            }
        else 
            {
            FLOG(_L("[BTNOTIF]\t CBTAuthNotifier::DoRejectAuthorizationL() IS SAME as the prior rejected device."));           
            
            // this was 2nd time in row. Clear the key, so the question will not be made next time.
            TBuf<2*KBTDevAddrSize> tmp;                 
            TBTDevAddr emptyAddr;
            emptyAddr.Reset();
            emptyAddr.GetReadable(tmp);                 
            RProperty::Set(KPSUidBluetoothEnginePrivateCategory, KBTBlockDevAddr, tmp );
            }
        }
        
    // Ask user to Block this device or not.
    if( !iMessage.IsNull() )
        {
        QueryBlockDeviceL();
        }
    }

// ----------------------------------------------------------
// CBTAuthNotifier::CheckAndSubscribeNotifLocks
// Subscribe to PS key and check key value.
// ----------------------------------------------------------
//
void CBTAuthNotifier::CheckAndSubscribeNotifLocks()
    {
    if ( !iLockActive->IsActive() )
        {
        FLOG(_L("[BTNOTIF]\t CBTAuthNotifier, Subscribe Device Operation Info"));
        iNotifLockProp.Subscribe( iLockActive->RequestStatus() );
        iLockActive->GoActive();
        }
    TInt ops;
    TBTNotifLockPublish::GetNotifLocks( iNotifLockProp, 
            ops, iBTAddr );
    iSuspended = ops & EBTNotiferLockPairedDeviceSetting;
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTAuthNotifier suspended ? %d"), iSuspended ));    
    }


// End of File
