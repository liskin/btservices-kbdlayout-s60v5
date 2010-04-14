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
* Description:  Bluetooth visibility timeout notifier class.
*
*/


// INCLUDE FILES

#include <StringLoader.h>       // Localisation stringloader
#include <BTNotif.rsg>          // Own resources
#include "BTNGenericInfoNotifier.h"      // Own class definition
#include "btNotifDebug.h"       // Debugging macros
#include <secondarydisplay/BTnotifSecondaryDisplayAPI.h>
#include <e32cmn.h>
#include <bluetoothuiutil.h>
#include "btnotifnameutils.h"

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTGenericInfoNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTGenericInfoNotifier* CBTGenericInfoNotifier::NewL()
    {
    CBTGenericInfoNotifier* self=new (ELeave) CBTGenericInfoNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CBTGenericInfoNotifier::CBTGenericInfoNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTGenericInfoNotifier::CBTGenericInfoNotifier() 
    {
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTGenericInfoNotifier::~CBTGenericInfoNotifier()
    {
    Cancel();   // Free own resources
    iQueryMessage.Close(); 
    }

// ----------------------------------------------------------
// CBTGenericInfoNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTGenericInfoNotifier::TNotifierInfo CBTGenericInfoNotifier::RegisterL()
    {
    iInfo.iUid=KBTGenericInfoNotifierUid;
    iInfo.iChannel=KBTGenericInfoNotifierUid;
    iInfo.iPriority=ENotifierPriorityHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTGenericInfoNotifier::StartL
// Synchronic notifier launch. Contructs and shows a global
// note when temp visibility expire, no parameters need here.
// ----------------------------------------------------------
//
TPtrC8 CBTGenericInfoNotifier::StartL( const TDesC8& aBuffer )
    {
    FLOG(_L("[BTNOTIF]\t CBTGenericInfoNotifier::StartL()"));
    if( !iNotifUiUtil )
        {
        iNotifUiUtil = CBTNotifUIUtil::NewL( iIsCoverUI );
        }
    ProcessParamBufferL(aBuffer, ETrue);
    FLOG(_L("[BTNOTIF]\t CBTGenericInfoNotifier::StartL() completed"));

    TPtrC8 ret(KNullDesC8);
    return (ret);
    }

// ----------------------------------------------------------
// CBTGenericInfoNotifier::GetParamsL
// Mandatory for BT Notifiers when using asynchronous launch. 
// ----------------------------------------------------------
//
void CBTGenericInfoNotifier::GetParamsL(const TDesC8& aBuffer, 
                                     TInt /*aReplySlot*/, 
                                     const RMessagePtr2& aMessage )
    {
	FLOG(_L("[BTNOTIF]\t CBTGenericInfoNotifier::GetParamsL"));  
	if (!iMessage.IsNull())
	    {
	    aMessage.Complete(KErrInUse);
	    return;
	    }
    iMessage = aMessage;
 	ProcessParamBufferL(aBuffer, EFalse);
    }

// ----------------------------------------------------------
// CBTGenericInfoNotifier::ProcessParamBufferL
// Parse the data out of the message that is sent by the
// client of the notifier.
// ----------------------------------------------------------
void CBTGenericInfoNotifier::ProcessParamBufferL(const TDesC8& aBuffer, TBool aSyncCall)
	{
	TBTGenericInfoNotifierParams bParams;
	TPckgC<TBTGenericInfoNotifierParams> bPckg(bParams);
		
	bPckg.Set( aBuffer );
	iSecondaryDisplayCommand = ECmdBTnotifUnavailable;
	switch (bPckg().iMessageType)
		{
		case EBTConnected: 			
			iMessageResourceId= R_BT_CONF_CONNECTED_PROMPT; 
			iSecondaryDisplayCommand=ECmdShowBtConnectedNote;
			break;
		case EBTDisconnected: 		
			iMessageResourceId= R_BT_CONF_DISCONNECTED_PROMPT; 
			iSecondaryDisplayCommand=ECmdShowBtDisconnectedNote;
			break;
		case EBTAudioAccessory: 	
			iMessageResourceId= R_BT_AUDIO_ACCESSORY_PROMPT; 
			iSecondaryDisplayCommand=ECmdShowBtAudioAccessoryNote;
			break;
		case EBTAudioHandset: 		
			iMessageResourceId= R_BT_AUDIO_HANDSET_PROMPT; 
			iSecondaryDisplayCommand=ECmdShowBtAudioHandsetNote;
			break;
		case EBTClosed: 			
			iMessageResourceId= R_BT_CONN_CLOSED_PROMPT; 
			iSecondaryDisplayCommand=ECmdShowBtDisconnectedNote;
			break;
		case EBTDeviceNotAvailable: 
			iMessageResourceId= R_BT_DEVICE_NOT_AVAIL; 
			iSecondaryDisplayCommand=ECmdShowBtDeviceNotAvailableNote;
			break;
		case EBTOfflineDisabled: 	
			iMessageResourceId= R_BT_OFFLINE_DISABLED; 
			iSecondaryDisplayCommand=ECmdShowBtOfflineDisableNote;
			break;	
		case EBTVisibilityTimeout: 	
			iMessageResourceId= R_BT_HIDDEN_FROM_OTHERS_NOTE; 
			iSecondaryDisplayCommand=ECmdShowBtHiddenFromOthersNote;
			break;	
		case EBTEnterSap: 			
			iMessageResourceId= R_BT_ENTER_SAP_NOTE; 
			iSecondaryDisplayCommand=ECmdShowBtSapEnteringNote;
			break;
		case EBTSapOk: 				
			iMessageResourceId= R_BT_ENTER_SAP_SUCCEED_NOTE; 
			iSecondaryDisplayCommand=ECmdShowBtSapEnteringSucceededNote;
			break;
		case EBTSapFailed: 			
			iMessageResourceId= R_BT_ENTER_SAP_FAILED_NOTE; 
			iSecondaryDisplayCommand=ECmdShowBtSapEnteringFailedNote;
			break;
		case EBTSapNoSim: 			
			iMessageResourceId= R_BT_SAP_NO_SIM_NOTE; 
			iSecondaryDisplayCommand=ECmdShowBtSapNoSimNote;
			break;			
		case EBTDeviceBusy: 
			iMessageResourceId=R_BT_BUSY_TEXT;
			iSecondaryDisplayCommand=ECmdShowBtBusyNote;
			break;
		case EIRNotSupported: 
			iMessageResourceId=R_IR_NOT_SUPPORTED;
			iSecondaryDisplayCommand=ECmdShowIrNotSupportedNote;
			break;
		case ECmdShowBtBatteryLow:
			iMessageResourceId= R_BT_ACCESSORY_LOW;
			iSecondaryDisplayCommand= ECmdShowBtBatteryLowNote;
			break;
		case ECmdShowBtBatteryCritical:
			iMessageResourceId= R_BT_ACCESSORY_CRITICAL; 
			iSecondaryDisplayCommand= ECmdShowBtBatteryCriticalNote;
			break;
        case EBTStayPowerOn:
            iMessageResourceId= R_BT_PERMANENTLY_ON; 
            break;
    	case EBTSwitchedOn:
    		iMessageResourceId= R_BT_SWITCHED_ON; 
    		break;
    	case EBTSwitchedOff:
    		iMessageResourceId= R_BT_SWITCHED_OFF;
    		break;
		default:
			FLOG(_L("[BTNOTIF]\t CBTGenericInfoNotifier:: Unkown messageType! ")); 
			User::Leave(KErrNotFound);		
		}
    
	// if the logic string contains substitute indicator "%U", replace it with device name:
	HBufC* buf = StringLoader::LoadL( iMessageResourceId);
	iQueryMessage.Assign( buf );
	
    TInt keyLen;
    TInt pos = BluetoothUiUtil::GetStringSubstringKeyPos( 
            iQueryMessage, 0, keyLen );
    if( pos > KErrNotFound)
		{
		iBTAddr = TBTDevAddr( bPckg().iRemoteAddr );
		if( !iDevice )
		    {
            iDevice = CBTDevice::NewL(iBTAddr);
		    }
	    GetDeviceFromRegL( iBTAddr );
		}
	else
	    {
	    ShowNoteAndCompleteL(aSyncCall);
	    }
	}
	
// ----------------------------------------------------------
// CBTGenericInfoNotifier::ShowNoteAndCompleteL
// Shows the notifier in backround 
// ----------------------------------------------------------
//
void CBTGenericInfoNotifier::ShowNoteAndCompleteL(TBool aSyncCall)
	{
	iNotifUiUtil->ShowInfoNoteL( iQueryMessage, iSecondaryDisplayCommand );
	if (!aSyncCall)
		{
        CompleteMessage(KErrNone);
        }
    FLOG(_L("[BTNOTIF]\t CBTGenericInfoNotifier::ShowNoteAndComplete() complete"));
	}

void CBTGenericInfoNotifier::HandleGetDeviceCompletedL(const CBTDevice* /*aDev*/)
    {
    FLOG(_L("[BTNOTIF]\t CBTGenericInfoNotifier::HandleGetDeviceCompleted()"));
    
    TBTDeviceName name;
    BtNotifNameUtils::GetDeviceDisplayName(name, iDevice);
    iQueryMessage.Zero();
    BluetoothUiUtil::LoadResourceAndSubstringL( 
            iQueryMessage, iMessageResourceId, name, 0);      
    
    ShowNoteAndCompleteL(EFalse);

    FLOG(_L("[BTNOTIF]\t CBTGenericInfoNotifier::HandleGetDeviceComplete() Complete"));        
    }

// End of File
