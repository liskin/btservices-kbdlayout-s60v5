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
* Description:  Show pairing status and set authorized, unique devicename of paired device.
*
*/


// INCLUDE FILES

#include <BTNotif.rsg>          // Own resources
#include <btnotif.h>
#include <bluetoothuiutil.h>
#include "btnotiflock.h"
#include "btnpaireddevsettnotifier.h"      // Own class definition
#include "btNotifDebug.h"       // Debugging macros
#include <bluetooth/hci/hcierrors.h>
#include "btnotifnameutils.h"

#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>

#include <e32cmn.h>
#include <AknMediatorFacade.h>  // CoverUI 

#ifdef __SERIES60_HELP
#include <hlplch.h>
#include <csxhelp/bt.hlp.hrh> // The bt hrh info is needed, for help launching
#endif

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTPairedDevSettNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTPairedDevSettNotifier* CBTPairedDevSettNotifier::NewL()
    {
    CBTPairedDevSettNotifier* self=new (ELeave) CBTPairedDevSettNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CBTPairedDevSettNotifier::CBTPairedDevSettNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTPairedDevSettNotifier::CBTPairedDevSettNotifier()
    {	
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTPairedDevSettNotifier::~CBTPairedDevSettNotifier()
    {
    }

// ----------------------------------------------------------
// CBTPairedDevSettNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTPairedDevSettNotifier::TNotifierInfo CBTPairedDevSettNotifier::RegisterL()
    {
	iInfo.iUid = KBTPairedDeviceSettingNotifierUid;
    iInfo.iChannel = KBTPairedDeviceSettingChannel;
    iInfo.iPriority = ENotifierPriorityVHigh;
    return iInfo;
    }


// ----------------------------------------------------------
// CBTPairedDevSettNotifier::ProcessStartParamsL
// Mandatory for BT Notifiers when using asynchronous launch. 
// This notifier is synchronous so no implementation is needed.
// ----------------------------------------------------------
//
void CBTPairedDevSettNotifier::ProcessStartParamsL()
    {
   	FLOG(_L("[BTNOTIF]\t CBTPairedDevSettNotifier::ProcessStartParamsL"));    

	ProcessParamBufferL(*iParamBuffer); 

	FLOG(_L("[BTNOTIF]\t CBTPairedDevSettNotifier::ProcessStartParamsL Complete"));    
  	}

// ----------------------------------------------------------
// CBTPairedDevSettNotifier::UpdateL
// Update notifier according to new data
// ----------------------------------------------------------
//	
TPtrC8 CBTPairedDevSettNotifier::UpdateL(const TDesC8& /*aBuffer*/)
	{
 	FLOG(_L("[BTNOTIF]\t CBTPairedDevSettNotifier::UpdateL"));
    TPtrC8 ret(KNullDesC8);
    return (ret);
	}

void CBTPairedDevSettNotifier::HandleGetDeviceCompletedL(const CBTDevice* /*aDev*/)
    {
    FLOG(_L("[BTNOTIF]\t CBTPairedDevSettNotifier::HandleGetDeviceCompletedL >>"));    
    TBTDeviceName name;
    BtNotifNameUtils::GetDeviceDisplayName(name, iDevice);
    
    RBuf stringholder;
    stringholder.CleanupClosePushL();
    
    // 1. Show pairing status note
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTPairedDevSettNotifier iPairingStatus: %d"), iPairingStatus));
    TInt resourceId = ProcessPairingErrorCode( iPairingStatus );    
    BluetoothUiUtil::LoadResourceAndSubstringL( stringholder, resourceId, name, 0 );

    if (!iPairingStatus)
        {
        iNotifUiUtil->ShowConfirmationNoteL( ETrue, stringholder, iSecondaryDisplayCommand, name );
        }
    else
        {
        iNotifUiUtil->ShowErrorNoteL( EFalse, stringholder, iSecondaryDisplayCommand, name );
        }
    CleanupStack::PopAndDestroy( &stringholder );
      
    if( iPairingStatus || (iDevice && iDevice->GlobalSecurity().Banned() ) )
        {
        TBTNotifLockPublish::DeleteNotifLocks( 
                EBTNotiferLockPairedDeviceSetting, iDevice->BDAddr() );
        CompleteMessage(KErrNone);
        return;
        }
    
    // 2. Show Authorization query
    //
    TInt keypress = iNotifUiUtil->ShowQueryL( R_BT_AUTHORIZATION_PROMPT, R_BT_AUTHORISATION_QUERY, 
            iSecondaryDisplayCommand, CAknQueryDialog::EConfirmationTone);

    if (!iMessage.IsNull())
        {
        // this notifier is not cancelled, continue the task:
        ChangeAuthorizeState(keypress);
        }
    }

// ----------------------------------------------------------
// CBTGenericQueryNotifier::ProcessParamBufferL
// Parse the data out of the message that is sent by the
// client of the notifier.
// ----------------------------------------------------------
void CBTPairedDevSettNotifier::ProcessParamBufferL(const TDesC8& aBuffer)
	{
    FLOG(_L("[BTNOTIF]\t CBTPairedDevSettNotifier::ProcessParamBufferL()"));
    
    TBTPairedDeviceSettingParamsPckg pckg; 
    pckg.Copy( aBuffer );

    iPairingStatus = pckg().iPairingStatus;
    if( iPairingStatus <= KHCIErrorBase )
        {
        // Pairing status could be KErrNone, or the Symbian error code
        // used for indicating HCI error codes.
        iPairingStatus -= KHCIErrorBase;
        }
    if ( !iPairingStatus )
        {
        TBTNotifLockPublish::AddNotifLocks( 
                EBTNotiferLockPairedDeviceSetting, pckg().iRemoteAddr );
        }

    iDevice = CBTDevice::NewL(pckg().iRemoteAddr);
    GetDeviceFromRegL( pckg().iRemoteAddr );

    FLOG(_L("[BTNOTIF]\t CBTPairedDevSettNotifier::ProcessParamBufferL() end"));
	}

// ----------------------------------------------------------
// CBTPairedDevSettNotifier::Cancel
// Release all own resources (member variables)
// ----------------------------------------------------------
//
void CBTPairedDevSettNotifier::Cancel()
    {
    FLOG(_L("[BTNOTIF]\t CBTPairedDevSettNotifier::Cancel()"));

    TBTNotifLockPublish::DeleteNotifLocks( 
            EBTNotiferLockPairedDeviceSetting, iDevice->BDAddr() );    	
    CBTNotifierBase::Cancel();

    FLOG(_L("[BTNOTIF]\t CBTPairedDevSettNotifier::Cancel() completed"));
    }
	
// ----------------------------------------------------------
// CBTPairedDevSettNotifier::ProcessPairingErrorCode
// ----------------------------------------------------------
//
TInt CBTPairedDevSettNotifier::ProcessPairingErrorCode( const TInt aErr )
    {
    if( !aErr )
        {
        return R_BT_SUCCESFULLY_PAIRED_PROMPT;
        }
    else if( aErr == -EHostBusyPairing )
        {
        return R_BT_SSP_BUSY;
        }
    else
        {
        return R_BT_PAIRING_FAILED_PROMPT;
        }
    }


void CBTPairedDevSettNotifier::QueryRenameDeviceL(const CBTDevice& /*aDevice*/) 
    {
    // No implementation for now.   
    }

// End of File
