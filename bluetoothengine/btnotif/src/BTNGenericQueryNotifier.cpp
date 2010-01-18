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
#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>
#include <e32cmn.h>
#include <BTNotif.rsg>          // Own resources
#include <btengsettings.h>
#include "BTNGenericQueryNotifier.h"      // Own class definition
#include "btNotifDebug.h"       // Debugging macros

#ifdef __SERIES60_HELP
#include <hlplch.h>
#include <csxhelp/bt.hlp.hrh> // The bt hrh info is needed, for help launching
#endif

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTGenericQueryNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTGenericQueryNotifier* CBTGenericQueryNotifier::NewL()
    {
    CBTGenericQueryNotifier* self=new (ELeave) CBTGenericQueryNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CBTGenericQueryNotifier::CBTGenericQueryNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTGenericQueryNotifier::CBTGenericQueryNotifier()
    {	
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTGenericQueryNotifier::~CBTGenericQueryNotifier()
    {
    Cancel();   // Free own resources
    delete iName; 
    delete iQueryMessage; 
	delete iQueryHeader;
    }

// ----------------------------------------------------------
// CBTGenericQueryNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTGenericQueryNotifier::TNotifierInfo CBTGenericQueryNotifier::RegisterL()
    {
    iInfo.iUid=KBTGenericQueryNotifierUid;
    iInfo.iChannel=KBTGenericQueryNotifierUid;
    iInfo.iPriority=ENotifierPriorityHigh;
    return iInfo;
    }


// ----------------------------------------------------------
// CBTGenericQueryNotifier::StartL
// Synchronic notifier launch. Contructs and shows a global
// note, no parameters need here.
// ----------------------------------------------------------
//
TPtrC8 CBTGenericQueryNotifier::StartL( const TDesC8& /*aBuffer*/ )
    {
    FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier::StartL()"));

    TPtrC8 ret(KNullDesC8);
    return (ret);    
    
    }

// ----------------------------------------------------------
// CBTGenericQueryNotifier::GetParamsL
// Mandatory for BT Notifiers when using asynchronous launch. 
// This notifier is synchronous so no implementation is needed.
// ----------------------------------------------------------
//
void CBTGenericQueryNotifier::GetParamsL(const TDesC8& aBuffer, 
                                     TInt aReplySlot, 
                                     const RMessagePtr2& aMessage)
    {
   	FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier::GetParamsL"));    

   	if (iMessage.Handle())
   	    {
   	    aMessage.Complete(KErrInUse);
   	    return;
   	    }
   	
	ProcessParamBufferL(aBuffer);
	
    iMessage = aMessage;
    iReplySlot = aReplySlot;
    
    ShowQueryAndCompleteL();
    
   	FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier::GetParamsL Complete"));    
    }

// ----------------------------------------------------------
// CBTGenericQueryNotifier::ProcessParamBufferL
// Parse the data out of the message that is sent by the
// client of the notifier.
// ----------------------------------------------------------
void CBTGenericQueryNotifier::ProcessParamBufferL(const TDesC8& aBuffer)
	{
	
	TBTGenericQueryNotifierParams bParams;
	TPckgC<TBTGenericQueryNotifierParams> bPckg(bParams);
	bPckg.Set( aBuffer );

	if( bPckg().iMessageType == EBTNameQuery )
	    {
	    iIsNameQuery = ETrue;
	    return;
	    }
	iSecondaryDisplayCommand = ECmdBTnotifUnavailable;
	switch (bPckg().iMessageType)
		{
		case EBTReceiveMessageQuery:
			iIsMessageQuery=EFalse;
			iMessageResourceId=R_BT_RECEIVE_MESSAGE	;	
			iSecondaryDisplayCommand=ECmdShowReceiveMessageFromDeviceDlg; 
			break;
		case EBTReceiveMessagePairedQuery:
			iIsMessageQuery=EFalse;		
			iMessageResourceId=R_BT_RECEIVE_MESSAGE_PAIRED;
			iSecondaryDisplayCommand= ECmdShowReceiveMessageFromPairedDeviceDlg; 
			break;			
		case EBTIsOffQuery:
			iIsMessageQuery=EFalse;		
    		iMessageResourceId=R_BT_POWER_IS_OFF ;
			iSecondaryDisplayCommand=ECmdShowBtIsOffDlg;		
			break;			
		case EBTActivateOffLineQuery:
			iIsMessageQuery=EFalse;		
    		iMessageResourceId=R_BT_ACTIVATE_IN_OFFLINE;
			iSecondaryDisplayCommand=ECmdShowBtActivateInOfflineDlg;		
			break;			
		case EBTNoDevicesFoundQuery:
			iIsMessageQuery=EFalse;		
			iMessageResourceId=R_BT_NO_DEVICES_FOUND;
			break;	
		case EBTAcceptRequestQuery:
			iIsMessageQuery=EFalse;		
			iMessageResourceId=R_BT_AUTHORISATION_NAME ;
			iSecondaryDisplayCommand=ECmdShowAcceptConnRequestDlg;
			break;		
			
		case EBTIsOffJavaQuery:	
			iIsMessageQuery=ETrue; 	
			if( bPckg().iNameExists )			
				{
				iMessageResourceId=R_BT_IS_OFF_JAVA_APPNAME; // when caller provide Java application name.	
				}
			else
				{
				iMessageResourceId=R_BT_IS_OFF_JAVA; //r_bt_is_hidden_java	
				}		
			iSecondaryDisplayCommand=ECmdShowBtBtIsOffJavaDlg;
			iQueryHeader=StringLoader::LoadL( R_BT_IS_OFF_JAVA_HEADER);		
			break;

		case EBTIsNotShownQuery:
			iIsMessageQuery=ETrue;			
			if( bPckg().iNameExists )
				{
				iMessageResourceId=R_BT_IS_HIDDEN_JAVA_APPNAME; // when caller provide Java application name.	
				}
			else
				{
				iMessageResourceId=R_BT_IS_HIDDEN_JAVA; //r_bt_is_hidden_java	
				}									
			iSecondaryDisplayCommand=ECmdShowBtIsNotVisibleDlg;
			iQueryHeader=StringLoader::LoadL( R_BT_IS_HIDDEN_JAVA_HEADER);
			break;
			
		case EBTBlockConnectionQuery:
		case EBTBlockPairedConnectionQuery:		
			iIsMessageQuery=ETrue;		
			if( bPckg().iMessageType == EBTBlockConnectionQuery )
				iMessageResourceId = R_BT_BLOCK_DEVICE_NOHELP; 
			else
				iMessageResourceId = R_BT_BLOCK_PAIRED_DEVICE_NOHELP ; 
				
			iQueryHeader= StringLoader::LoadL( R_BT_BLOCK_DEVICE_HEADER );
			break;
			
		case EBTSwitchOffAnyway:
			iIsMessageQuery=EFalse;		
			iMessageResourceId=R_BT_SWITCH_OFF_ANYWAY;
			break;		
					
		default:
			FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier:: Unkown messageType! ")); 
 			User::Leave(KErrNotFound);		
		}
	
	// if the logic string contains substitute indicator "%U", replace it with device name:
	//		
	iQueryMessage = StringLoader::LoadL( iMessageResourceId);
	_LIT(PU,"%U");
	if( iQueryMessage->Find(PU) != KErrNotFound)
		{		
		delete iQueryMessage;
		iQueryMessage=NULL;
		
		//if no device name provided, default name will be used:
		if( !bPckg().iNameExists )			
			iName=StringLoader::LoadL(R_BT_DIALOG_DEF_NAME);
		else
			{
			iName=HBufC::NewL(bPckg().iName.Length() );
			iName->Des().Copy(bPckg().iName);
			}
		iQueryMessage = StringLoader::LoadL( iMessageResourceId,*iName);		
		}
	else
		{
		iName=NULL;		
		}		
	}

// ----------------------------------------------------------
// CBTGenericQueryNotifier::UpdateL
// Update notifier according to new data
// ----------------------------------------------------------
//	
TPtrC8 CBTGenericQueryNotifier::UpdateL(const TDesC8& aBuffer)
	{
   	FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier::UpdateL")); 
    delete iQueryMessage;
    iQueryMessage = NULL;
	ProcessParamBufferL(aBuffer);
	if( !iNotifUiUtil->IsQueryReleased() )
		{
		if(iIsMessageQuery )
		    {
		    iNotifUiUtil->UpdateMessageQueryDlgL(*iQueryMessage);
		    }
		else
		    {
		    iNotifUiUtil->UpdateQueryDlgL(*iQueryMessage);
		    }
		}
   	FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier::UpdateL complete")); 	

    TPtrC8 ret(KNullDesC8);
    return (ret);   	
	}

// ----------------------------------------------------------
// CBTGenericQueryNotifier::ShowQueryAndCompleteL
// Shows the notifier in backround and complete message
// ----------------------------------------------------------
//
void CBTGenericQueryNotifier::ShowQueryAndCompleteL()
	{ 
   	FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier::ShowQueryAndCompleteL")); 	
	
	// Turn lights on and deactivate apps -key
	//
   	iNotifUiUtil->TurnLightsOn();  	
	if( iIsNameQuery )
	    {
	    TBool nameStatus = IsLocalNameModifiedL();
	    if( !nameStatus )
	        {
	        (void) AskLocalBTNameQueryL();
	        }
        // Check if Local name is set again before turn BT on. 
        // Turn BT on only when there is local name, since user still has chance
        // to ignore the asking name query - like pressing End-key     
        //             
        TBTDeviceName localName;           
        localName.Zero();
        TInt err = iBTEngSettings->GetLocalName(localName);
              
        if (localName.Length() <= 0)
            {   
            err = KErrCancel;
            }
	    	    
	    CompleteMessage( (!err) ? ETrue : EFalse, KErrNone );

	    FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier::ShowQueryAndCompleteL iIsNameQuery complete")); 
	    return;
	    }
	
	TInt keypress;
    TBTDeviceName name(KNullDesC);
    if( iName )
	    {
	    name.Copy( *iName );
	    }
    if( iIsMessageQuery	)
		{
		
        keypress = iNotifUiUtil->ShowMessageQueryL(*iQueryMessage, *iQueryHeader, 
                        R_BT_GENERIC_MESSAGE_QUERY, CAknQueryDialog::EConfirmationTone );        
		}
	else
    	{
    	keypress = iNotifUiUtil->ShowQueryL( *iQueryMessage, R_BT_GENERIC_QUERY, 
    	        iSecondaryDisplayCommand, name, CAknQueryDialog::EConfirmationTone );
    	}
	
   	CompleteMessage( (keypress) ? ETrue: EFalse, KErrNone );

   	FLOG(_L("[BTNOTIF]\t CBTGenericQueryNotifier::ShowQueryAndCompleteL complete")); 			
	}

// End of File
