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
* Description:  Declares Bluetooth notifiers base class.
*
*/


// INCLUDE FILES
#include <centralrepository.h> 
#include <CoreApplicationUIsSDKCRKeys.h>

#include <settingsinternalcrkeys.h>
#include <e32property.h>

#include <btengsettings.h>
#include <btengprivatecrkeys.h>
#include <btengconnman.h>
#include <btengutil.h>
#include <StringLoader.h>    // Localisation stringloader
#include <BTNotif.rsg>       // Own resources
#include <featmgr.h>            // Feature Manager API
#include <utf.h>             // CnvUtfConverter
#include <bluetoothuiutil.h>
#include "btnotifier.h"      // Own class definition
#include "btNotifDebug.h"    // Debugging macros
#include "btnotiflock.h"
#include "btnotif.hrh"       // define MaxNameLength
#include "btnotifpanic.h"
#include "btnotifnameutils.h"

#ifdef __SERIES60_HELP
#include <hlplch.h>
#include <csxhelp/bt.hlp.hrh> // The bt hrh info is needed, for help launching
#endif

// CONSTANTS
const TInt KBTNAutolockEnabled = 1;

// The following definitions are used to implement checking of connection/pairing
// request frequency

// if a new connection is made quicker than CONNECT_ADD_TIME after the old one, 
// then the saved time is added with CONNECT_ADD_TIME.
#define CONNECT_ADD_TIME TTimeIntervalSeconds(10)

// if the saved time is later than now+DENY_THRESHOLD, start rejecting incoming connections
// from unpaired devices.
#define DENY_THRESHOLD	 TTimeIntervalSeconds(30)

// if the user denies incoming connection the saved time goes this much in the future.
#define REJECT_ADD_TIME TTimeIntervalSeconds(31)


// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTNotifierBase::CBTNotifierBase
// C++ default constructor can NOT contain any code, that
// might leave. Sets the AOs priority and puts
// itself to the active scheduler stack.
// ----------------------------------------------------------
//
CBTNotifierBase::CBTNotifierBase() 
    {
    }

// ----------------------------------------------------------
// CBTNotifierBase::ConstructL
// Symbian 2nd phase constructor can leave.
// Create registry object and open resource file.
// ----------------------------------------------------------
//
void CBTNotifierBase::ConstructL()
    {
	// Sets up TLS, must be done before FeatureManager is used.
	FeatureManager::InitializeLibL();
	iIsCoverUI = ( FeatureManager::FeatureSupported( KFeatureIdCoverDisplay ) ) 
                    ? ETrue : EFalse; 	
	// Frees the TLS. Must be done after FeatureManager is used.
    FeatureManager::UnInitializeLib(); 

    iBTEngSettings = CBTEngSettings::NewL();
    iDevMan = CBTEngDevMan::NewL( this );
    iNotifUiUtil = CBTNotifUIUtil::NewL( iIsCoverUI );
    }

// ----------------------------------------------------------
// Destructor.
// ----------------------------------------------------------
//
CBTNotifierBase::~CBTNotifierBase()
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::~CBTNotifierBase()"));    
    Cancel();
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::~CBTNotifierBase() -- Done"));    
    }

// ----------------------------------------------------------
// CBTNotifierBase::Release
// Release itself. Call to destructor.
// ----------------------------------------------------------
//
void CBTNotifierBase::Release()
    {
    delete this;
    }


// ----------------------------------------------------------
// CBTNotifierBase::Info
// Return registered information.
// ----------------------------------------------------------
//
CBTNotifierBase::TNotifierInfo CBTNotifierBase::Info() const
    {
    return iInfo;
    }

// ----------------------------------------------------------
// CBTNotifierBase::StartL
// Synchronic notifier launch. Does nothing here.
// ----------------------------------------------------------
//
TPtrC8 CBTNotifierBase::StartL(const TDesC8& /*aBuffer*/)
    {
    TPtrC8 ret(KNullDesC8);
    return (ret);
    }

// ----------------------------------------------------------
// CBTNotifierBase::StartL
// Asynchronic notifier launch.
// ----------------------------------------------------------
//
void CBTNotifierBase::StartL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    if( !iNotifUiUtil )
        {
        iNotifUiUtil = CBTNotifUIUtil::NewL( iIsCoverUI );    
        }
    
    TRAPD(err, GetParamsL(aBuffer, aReplySlot, aMessage));
    if (err)
        {
        CompleteMessage(err);
        }
    }

// ----------------------------------------------------------
// CBTNotifierBase::Cancel
// Cancelling method.
// ----------------------------------------------------------
//
void CBTNotifierBase::Cancel()
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::Cancel()"));    

    delete iNotifUiUtil;
    iNotifUiUtil = NULL;
    
	delete iBTEngSettings;
    iBTEngSettings = NULL;

    delete iDevMan;
    iDevMan = NULL;
    
    delete iDevice;
    iDevice = NULL;
    
    if (iDeviceArray)
        {
        iDeviceArray->ResetAndDestroy();
        delete iDeviceArray;
        iDeviceArray = NULL;
        }
        
	CompleteMessage(KErrCancel);        
    }

// ----------------------------------------------------------
// CBTNotifierBase::UpdateL
// Notifier update. Not supported.
// ----------------------------------------------------------
//
TPtrC8 CBTNotifierBase::UpdateL(const TDesC8& /*aBuffer*/)
    {
    TPtrC8 ret(KNullDesC8);
    return (ret);
    }


// ----------------------------------------------------------
// CBTNotifierBase::AutoLockOnL
// ----------------------------------------------------------
//
TBool CBTNotifierBase::AutoLockOnL()
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::AutoLockOnL()"));

    TBool result = EFalse;
    TInt status = KBTNAutolockEnabled;
    
    // Connecting and initialization:
	CRepository* repository = CRepository::NewL(KCRUidSecuritySettings);
		
	repository->Get(KSettingsAutolockStatus, status);
		
	// Closing connection:
	delete repository;
		
    if ( status == KBTNAutolockEnabled )
        {
        result = ETrue;
        }

    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifierBase::AutoLockOnL() completed with %d"), result ));

    return result;
    }

// ----------------------------------------------------------
// CBTNotifierBase::CheckAndSetPowerOnL
// The note or query to be shown depends on two SharedData flags
// ----------------------------------------------------------
//
TBool CBTNotifierBase::CheckAndSetPowerOnL()
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::CheckAndSetPowerOnL()"));

	TBTPowerStateValue powerState( EBTPowerOff );
    // Get current power status
    //
    User::LeaveIfError( iBTEngSettings->GetPowerState( powerState ) );

    // If power is off, turn it on
    //
    if ( powerState == EBTPowerOff )
        {
        TInt offlineModeOff( 1 );    // possible values are 0 and 1
        TInt activationEnabled( 0 ); // possible values are 0 and 1
        
        // Offline mode on?
        CRepository* repository = CRepository::NewL(KCRUidCoreApplicationUIs);
        repository->Get(KCoreAppUIsNetworkConnectionAllowed, offlineModeOff);
        delete repository;
        
        // BT Activation enabled?
        repository = CRepository::NewL(KCRUidBluetoothEngine);
        repository->Get(KBTEnabledInOffline, activationEnabled);
        delete repository;
        
        // Choose user interaction
        //
        if ( !offlineModeOff && !activationEnabled )
            {
            // Show user that feature is disabled:
            iNotifUiUtil->ShowInfoNoteL( R_BT_OFFLINE_DISABLED, ECmdShowBtOfflineDisableNote );
            return EFalse; // Bluetooth can't be set on.
            }
        else
            {
            // Choose query
            //
            TInt keypress( 0 );	
            if ( offlineModeOff )
                {
                keypress = iNotifUiUtil->ShowQueryL( KErrNone, R_BT_POWER_IS_OFF_QUERY, ECmdShowBtIsOffDlg );
                }
            else
                {
                keypress = iNotifUiUtil->ShowQueryL( KErrNone, R_BT_ACTIVATE_IN_OFFLINE_QUERY, 
                        ECmdShowBtActivateInOfflineDlg );
				}

            if( keypress )  // User answered YES
                { 
                //Check if the local name has been set. If not ask user to set name
                
				TBool ok = IsLocalNameModifiedL();
    			if( !ok )
    				{
      				ok = AskLocalBTNameQueryL();
    				}
               if ( ok )
			       {  
                	ok = ( iBTEngSettings->SetPowerState( EBTPowerOn ) ) ? EFalse : ETrue;
                	}
                return ok;
                }
            else // User has cancelled the dialog
                {
                FLOG(_L("[BTNOTIF]\t CBTNotifierBase::CheckAndSetPowerOnL(), dialog canceled"));
                return EFalse;
                }                    
            }
        }
    return ETrue;
    }
    
// ----------------------------------------------------------
// CBTNotifierBase::AskLocalBTNameQueryL()
// ----------------------------------------------------------
//
TBool CBTNotifierBase::AskLocalBTNameQueryL()
    {
	FLOG(_L("[BTNOTIF]\t CBTNotifierBase::AskLocalBTNameQueryL() >>"));
	
	TBTDeviceName tempNameBuffer;
    // Cut the excess part of the name away. The query can handle only 
    // KBTUIMaxNameLength characters.
    //
    if( tempNameBuffer.Length() > KBTUIMaxNameLength )
     {
     tempNameBuffer.SetLength( KBTUIMaxNameLength );
     }

    //Get default name if given by e.g. phone product
    (void) RProperty::Get( KPropertyUidBluetoothCategory, 
                           KPropertyKeyBluetoothGetDeviceName, tempNameBuffer );
    
    if( !tempNameBuffer.Length() )
        {
        (void) RProperty::Get( KPropertyUidBluetoothCategory, 
                KPropertyKeyBluetoothGetDeviceName, tempNameBuffer );
        }
    
    if( !tempNameBuffer.Length() )
        {
        RBTRegServ btRegServ;
        RBTLocalDevice btReg;
        TBTLocalDevice localDev;

        TInt err = btRegServ.Connect();
        if( !err )
            {
            err = btReg.Open( btRegServ );
            } 
        if( !err )
            {
            // Read the BT local name from BT Registry.
            err = btReg.Get( localDev );
            }
        if( !err )
            {
            // The error can be > 0 if there are unconverted characters.
            err = CnvUtfConverter::ConvertToUnicodeFromUtf8( tempNameBuffer, localDev.DeviceName() );
            }
        btReg.Close();
        btRegServ.Close();
        }

    TInt keypress = iNotifUiUtil->ShowTextInputQueryL( tempNameBuffer, 
            R_BT_ENTER_LOCAL_NAME_QUERY, ECmdBTnotifUnavailable );
    
    if( keypress )  // User has accepted the dialog
        {
        AknTextUtils::StripCharacters(tempNameBuffer,  KAknStripListControlChars);
        tempNameBuffer.TrimAll();   // Remove extra spaces
        // If name was full of invalid chars, it becomes empty after above cleanup. 
        if( tempNameBuffer.Length() ) 
            {
            TInt err = iBTEngSettings->SetLocalName( tempNameBuffer );
            FLOG(_L("[BTNOTIF]\t CBTNotifierBase::AskLocalBTNameQueryL() <<"));
            return (err) ? EFalse : ETrue;    
            }
        else
            {
            FLOG(_L("[BTNOTIF]\t CBTNotifierBase::AskLocalBTNameQueryL() << failed"));
            return EFalse;
            }
        }
    else
        {
        FLOG(_L("[BTNOTIF]\t CBTNotifierBase::AskLocalBTNameQueryL() << cancelled"));
        return EFalse;
        }
    }

// ---------------------------------------------------------------------------------
// CBTNotifierBase::IsExistingConnectionToAudioL
// Check if there is any existing connection to audio profiles from the same device
// ---------------------------------------------------------------------------------
TBool CBTNotifierBase::IsExistingConnectionToAudioL( const TBTDevAddr& aDevAddr )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::IsExistingConnectionToAudioL()"));
    CBTEngConnMan* connMan = CBTEngConnMan::NewL();
    TBTEngConnectionStatus conntatus( EBTEngNotConnected );
    (void) connMan->IsConnected(aDevAddr,conntatus);      
    delete connMan;
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::IsExistingConnectionToAudioL() complete"));
    return ( conntatus==EBTEngConnected || conntatus==EBTEngConnecting );
    }

// ---------------------------------------------------------------------------
// CBTNotifierBase::IsLocalNameModified
// Checks from central repository whether the Bluetooth friendly name 
// has been modified .
// ---------------------------------------------------------------------------
//
TBool CBTNotifierBase::IsLocalNameModifiedL()
	{
	FLOG(_L("[BTNOTIF]\t CBTNotifierBase::IsLocalNameModified()"));   
    CRepository* cenRep = CRepository::NewL( KCRUidBTEngPrivateSettings );
    TInt val( EBTLocalNameDefault );
    (void) cenRep->Get( KBTLocalNameChanged, val );
    delete cenRep;
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::IsLocalNameModified() complete"));
    return val == EBTLocalNameSet;
    }


void CBTNotifierBase::GetDeviceFromRegL(const TBTDevAddr& aAddr)
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::GetDeviceFromRegL"));
    __ASSERT_DEBUG(iDevice, BTNOTIF_PANIC(EiDeviceNullWhenCallingGetDeviceFromRegL));

    TBTRegistrySearch mySearch;
    mySearch.FindAddress( aAddr );
    iDeviceArray = new (ELeave) CBTDeviceArray(1);
    if( !iDevMan )
        {
        iDevMan = CBTEngDevMan::NewL( this );    
        }
    TInt err = iDevMan->GetDevices( mySearch, iDeviceArray );
    if(err)
        {
        FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifierBase::GetDeviceFromRegL iDevMan->GetDevices error = %d"), err));
        DoHandleGetDevicesCompleteL(err, NULL);
        }
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::GetDeviceFromRegL done"));
    }

// ----------------------------------------------------------
// CBTNotifierBase::QueryBlockDeviceL
// Opens a query that returns wether or not user wants the device blocked
// ----------------------------------------------------------
void CBTNotifierBase::QueryBlockDeviceL()
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::QueryBlockDeviceL()"));
    __ASSERT_DEBUG( iNotifUiUtil->IsQueryReleased(), User::Panic( _L("CBTNotifierBase - iYesNoDlg not released!"), KErrAlreadyExists ) );
    
    TBTDeviceName bName;
    BtNotifNameUtils::GetDeviceDisplayName(bName, iDevice);

    HBufC* header= StringLoader::LoadLC( R_BT_BLOCK_DEVICE_HEADER );

    TInt resId = IsUserAwarePaired( iDevice->AsNamelessDevice() ) ? 
        R_BT_BLOCK_PAIRED_DEVICE_NOHELP : R_BT_BLOCK_DEVICE_NOHELP;
	
    RBuf stringholder;
    stringholder.CleanupClosePushL();
    BluetoothUiUtil::LoadResourceAndSubstringL( stringholder, resId, bName, 0 );

	TInt keypress = iNotifUiUtil->ShowMessageQueryL( stringholder, *header, 
	        R_BT_GENERIC_MESSAGE_QUERY, CAknQueryDialog::EConfirmationTone );
    
    CleanupStack::PopAndDestroy(&stringholder);                   
    CleanupStack::PopAndDestroy(header);  

    if( keypress )// user replied "Yes"
        {
        DoBlockDevice();
        }
    else  // user replied "No" 
        {
        CompleteMessage(KErrCancel);        
        } 
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::QueryBlockDeviceL() complete"));   
    }

void CBTNotifierBase::DoBlockDevice()
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::DoBlockDevice()"));
    TBTDeviceSecurity deviceSecurity = iDevice->GlobalSecurity();
    deviceSecurity.SetBanned(ETrue);
    deviceSecurity.SetNoAuthenticate(EFalse);
    deviceSecurity.SetNoAuthorise(EFalse);
    iDevice->SetGlobalSecurity(deviceSecurity);
    iDevice->DeleteLinkKey();
 
    iBTRegistryQueryState = ESetDeviceBlocked;
    TInt err = KErrNone;
    if( !iDevMan )
        {
        TRAP(err, iDevMan = CBTEngDevMan::NewL( this ));    
        }
    if( !err )
        {
        err = iDevMan->ModifyDevice( *iDevice );
        }
    if( err )
        {
        // if error, complete message, otherwise waiting for devman callback
        CompleteMessage(err); 
        }
    }

void CBTNotifierBase::ChangeAuthorizeState( TBool aTrust )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::ChangeAuthorizeState()"));   
    TBTDeviceSecurity sec = iDevice->GlobalSecurity();
    sec.SetBanned(EFalse);
    sec.SetNoAuthorise( ( aTrust ) ? ETrue : EFalse ) ;
    iDevice->SetGlobalSecurity(sec);
 
    iBTRegistryQueryState = ESetDeviceAuthorizeState;
    TInt err = KErrNone;
    if( !iDevMan )
        {    
        TRAP(err, iDevMan = CBTEngDevMan::NewL( this ));
        }
    if( !err )
        {
        err = iDevMan->ModifyDevice( *iDevice );
        }
    if( err )
        {
        // if error, complete message, otherwise waiting for devman callback        
        CompleteMessage(err); 
        TBTNotifLockPublish::DeleteNotifLocks( 
                EBTNotiferLockPairedDeviceSetting, iDevice->BDAddr() );
        }
    }

// ----------------------------------------------------------
// CBTNotifierBase::CheckAndHandleQueryIntervalL
// ----------------------------------------------------------
//
void CBTNotifierBase::CheckAndHandleQueryIntervalL()
    {
    FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval start." ) );
    TBool res = CheckQueryInterval();
    if( res )
        {
        FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval check on/off query status." ) );
        // Check if we're already showing Turn BT off query
        TInt queryValue=EBTQueryOff;
        RProperty::Get( KPSUidBluetoothEnginePrivateCategory, KBTTurnBTOffQueryOn, queryValue );
        if( queryValue==EBTQueryOff )
            {
            FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval asking to turn BT off." ) );
            RProperty::Set( KPSUidBluetoothEnginePrivateCategory, KBTTurnBTOffQueryOn, EBTQueryOn );
            TInt keypress = iNotifUiUtil->ShowQueryL( KErrNone, R_BT_TURN_BT_OFF_NOTE, ECmdBTnotifUnavailable );
            RProperty::Set( KPSUidBluetoothEnginePrivateCategory, KBTTurnBTOffQueryOn, EBTQueryOff );
            if( keypress )  // User has accepted the dialog
                {
                FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval turning BT off." ) );
                (void) iBTEngSettings->SetPowerState( EBTPowerOff );
                CompleteMessage( KErrAccessDenied );
                }
            }
        }
    FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckAndHandleQueryIntervalL done." ) );
    }

void CBTNotifierBase::DoHandleGetDevicesCompleteL( TInt aErr, CBTDeviceArray* aDeviceArray)
    {
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifierBase::DoHandleGetDevicesCompleteL(%d)"), aErr ));
    TBTDeviceName devName;
    if (!aErr && aDeviceArray && aDeviceArray->Count())
        {
        // Reassign so that we won't lose the device name as it is
        // needed if registry doesn't have the name for this device yet:
        CBTDevice* temp = iDevice;
        CleanupStack::PushL(temp);
        // Taking the object returned by registry so that iDevice have
        // all the up-to-date information except device name.
        iDevice = aDeviceArray->At(0);
        aDeviceArray->Delete( 0 );
        // we show user the device name from registry if it is available;
        // Otherwise, the name passed in by the notifier request shall be 
        // used.
        if( ( !iDevice->IsValidDeviceName() || 
              !iDevice->DeviceName().Length() ) && 
            temp->IsValidDeviceName() )
            {
            // We are using a stored device name, which will
            // already have been processed
            iDevice->SetDeviceNameL( temp->DeviceName() );
            }
        CleanupStack::PopAndDestroy(temp);
        }
    BtNotifNameUtils::GetDeviceName(devName, iDevice);   
    
    // It is possible that iDevice hasn't got a name so far. Use the default BT name
    // got from GetDeviceName().
    if ( !iDevice->IsValidDeviceName() || !iDevice->DeviceName().Length() )
        {
        BtNotifNameUtils::SetDeviceNameL(devName, *iDevice);
        }
    HandleGetDeviceCompletedL( iDevice ); 
    }

// ----------------------------------------------------------
// CBTNotifierBase::CompleteMessage
// ----------------------------------------------------------
//	
void CBTNotifierBase::CompleteMessage(TInt aErr)
    {	   
	if( !iMessage.IsNull() )
	    {
	    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifierBase::CompleteMessage(%d)"), aErr ));
	    iMessage.Complete( aErr );
	    }
	}

// ----------------------------------------------------------
// CBTNotifierBase::CompleteMessage
// ----------------------------------------------------------
//	
void CBTNotifierBase::CompleteMessage(TInt aValueToReplySlot, TInt aErr)
    {
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifierBase::CompleteMessage(%d, %d)"), 
            aValueToReplySlot, aErr ) );
    CompleteMessage(TPckgBuf<TInt>( aValueToReplySlot ), aErr);
	}

// ----------------------------------------------------------
// CBTNotifierBase::CompleteMessage
// ----------------------------------------------------------
//	
void CBTNotifierBase::CompleteMessage(const TDesC8& aDesToReplySlot, TInt aErr)
    {	   
	if( !iMessage.IsNull() )
	    {
	    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifierBase::CompleteMessage(TDesC8&, %d)"), aErr ) );
	    if ( !aErr )
	        {
	        aErr = iMessage.Write( iReplySlot, aDesToReplySlot );
	        }
	    iMessage.Complete( aErr );
	    }
	}

#ifdef __SERIES60_HELP    
//-------------------------------------------------------------
// CBTNotifierBase::LaunchHelp
//--------------------------------------------------------------
TInt CBTNotifierBase::LaunchHelp(TAny * tCoeHelpContext )
	{	
	TCoeHelpContext hc;
	
	if( tCoeHelpContext==NULL)
		hc = TCoeHelpContext(KUidBTUI,KBT_HLP_BLOCKED);
	else
		hc = *static_cast<TCoeHelpContext*>(tCoeHelpContext);
		 
		CArrayFix< TCoeHelpContext >* array = new CArrayFixFlat< TCoeHelpContext >(1);
	TRAPD(err,
		{
		CleanupStack::PushL(array);
		array->AppendL(hc);		
		HlpLauncher::LaunchHelpApplicationL( CCoeEnv::Static()->WsSession() , array );
    	CleanupStack::Pop(array);
		});

    // the array is not deleted, since deleting it will result
    // to E32User-CBase 3. It is assumed that HlpLancher is taking care of it.
    // however this has not been documented.
    return err;
	}
#endif

void CBTNotifierBase::HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray)
    {
    TRAP_IGNORE(DoHandleGetDevicesCompleteL(aErr, aDeviceArray));
    }

void CBTNotifierBase::HandleDevManComplete(TInt aErr)
    {
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifierBase::HandleDevManComplete()  aErr = %d"), aErr ));
    
    switch (iBTRegistryQueryState)
        {
        case ESetDeviceBlocked:
            {
            // Blocking device was demanded by user after the user
            // rejected incoming pairing or connect request. The message
            // to be completed here is the original pair or authorization request
            // which has been rejected by the user.
            CompleteMessage(KErrCancel);
            }
        case ESetDeviceAuthorizeState:
            {
            TBTNotifLockPublish::DeleteNotifLocks( 
                    EBTNotiferLockPairedDeviceSetting, iDevice->BDAddr() );
            CompleteMessage(aErr);
            break;
            }
        }
    FLOG(_L("[BTNOTIF]\t CBTNotifierBase::HandleDevManComplete() Complete"));           
    }

void CBTNotifierBase::HandleGetDeviceCompletedL(const CBTDevice* /*aDev*/)
    {    
    }

// ----------------------------------------------------------
// CBTNotifierBase::CheckQueryInterval
// ----------------------------------------------------------
//
TBool CBTNotifierBase::CheckQueryInterval()
    {
    FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval()" ) );
    TBool result = EFalse;
    // Check if the user gets too many queries within a certain time interval from unpaired devices
    if( iDevice && !iDevice->IsValidLinkKey() )
        {
        TBuf8<32> buf;
        TTime now;
        now.UniversalTime();    // Get current time
        TInt64 writeTime = ( now + REJECT_ADD_TIME ).Int64();

        // get last connection time
        TInt err = RProperty::Get( KPSUidBluetoothEnginePrivateCategory, KBTConnectionTimeStamp, buf );
        if( !err )
            {
            FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval Checking query interval." ) );
            // Parse recorded time stamp
            TInt64 num(0);
            TLex8 lex;
            lex = buf;
            lex.Val( num );
            TTime lastTime( num );

            // detection of clock adjusment
            TInt timetravel = 2 * REJECT_ADD_TIME.Int();
            if( lastTime > now + TTimeIntervalSeconds(timetravel) )
                {
                FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval  time travel detection." ) );
                lastTime = now;
                }
            // new request too quickly, move booked time
            if( lastTime + CONNECT_ADD_TIME > now )
                {
                FLOG( _L( "[BTNOTIF]\t CBTAuthNotifier::IsDosAttack  CONNECT_ADD_TIME added." ) );
                writeTime = ( lastTime + CONNECT_ADD_TIME ).Int64();
                }

            FTRACE( FPrint( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval() lastTime = %d" ), lastTime.Int64() ) );
            FTRACE( FPrint( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval now + treshold: %d" ), (now + DENY_THRESHOLD).Int64() ) );
            // If threshold is exceeded, ask user to turn Bluetooth OFF
            if( lastTime > now + DENY_THRESHOLD )
                {
                FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval threshold exceeded!." ) );
                result = ETrue;
                }
            }
        else
            {
            FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval setting connection timestamp first time." ) );
            }
        // Write back the timestamp
        buf.Num( writeTime );
        FTRACE( FPrint( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval updating timestamp: %d." ), writeTime ) );
        RProperty::Set( KPSUidBluetoothEnginePrivateCategory,KBTConnectionTimeStamp, buf );
        } // end of query interval check
    FLOG( _L( "[BTNOTIF]\t CBTNotifierBase::CheckQueryInterval() complete" ) );
    return result;
    }
// End of File
