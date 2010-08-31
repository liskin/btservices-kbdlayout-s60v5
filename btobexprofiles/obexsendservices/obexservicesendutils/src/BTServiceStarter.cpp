/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Service starter implementation
*
*/



// INCLUDE FILES
#include "BTServiceStarter.h"
#include "BTSUDebug.h"
#include "BTSOPPController.h"
#include "BTSBIPController.h"
#include "BTSBPPController.h"

#include <obexutilsmessagehandler.h>
#include <featmgr.h>
#include <hbdevicenotificationdialogsymbian.h>
#include <hbtextresolversymbian.h>

// CONSTANTS

// From BT SIG - Assigned numbers
const TUint KBTServiceOPPSending        = 0x1105;
const TUint KBTServiceDirectPrinting    = 0x1118;
const TUint KBTServiceImagingResponder  = 0x111B;

const TUint KBTProgressInterval         = 1000000;

const TUid KUidMsgTypeBt                 = {0x10009ED5};
//todo  need to find a suitable header to include. This is from obexutilslayer.h

const TInt KMaxDesCLength  = 256;


//_LIT(KSendingDialog,"com.nokia.hb.btdevicedialog/1.0");

_LIT(KLocFileName, "btdialogs_");
_LIT(KPath, "z:/resource/qt/translations/");  

_LIT(KFilesSentText, "txt_bt_dpophead_all_files_sent");//All files sent 
_LIT(KDeviceText,"txt_bt_dpopinfo_sent_to_1");
//_LIT(KNotConnectedText,"txt_bt_info_unable_to_connect_with_bluetooth");
_LIT(KSendingFailedText,"txt_bt_dpophead_sending_failed");

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTServiceStarter::CBTServiceStarter
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTServiceStarter::CBTServiceStarter() 
    : CActive( CActive::EPriorityStandard ),
      iBTEngDiscovery(NULL),
      iService( EBTSendingService ),
      iServiceStarted( EFalse ),
      iMessageServerIndex(0),
      iBytesSendWithBIP(0),
      iProgressDialogActive(EFalse),
      iUserCancel(EFalse), 
      iFeatureManagerInitialized(EFalse),
      iTriedBIP(EFalse),
      iTriedOPP(EFalse)
    {    
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ConstructL()"));
    iDevice = CBTDevice::NewL();
    iDialog = CObexUtilsDialog::NewL( this );
    iDelayedDestroyer = CBTServiceDelayedDestroyer::NewL(CActive::EPriorityStandard);
    FeatureManager::InitializeLibL();
    iFeatureManagerInitialized = ETrue;
    iLocalisationInit = HbTextResolverSymbian::Init(KLocFileName, KPath);
    FLOG(_L("[BTSU]\t CBTServiceStarter::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTServiceStarter* CBTServiceStarter::NewL()
    {
    CBTServiceStarter* self = new( ELeave ) CBTServiceStarter();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

CBTServiceStarter::~CBTServiceStarter()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::Destructor()"));
    if ( iMessageServerIndex != 0 )
        {
        TRAPD( notUsedRetVal, TObexUtilsMessageHandler::DeleteOutboxEntryL( iMessageServerIndex ) );
        notUsedRetVal=notUsedRetVal;
        FTRACE(FPrint(_L("[BTSU]\t ~CBTServiceStarter() delete ob entry %d"), notUsedRetVal ) );
        }
    StopTransfer(KErrCancel); // Cancels active object
    
    delete iList;
    delete iDevice;

    delete iController;
    delete iBTEngDiscovery;
    delete iDialog;
    delete iDelayedDestroyer;
    
    if(iWaiter && iWaiter->IsStarted() )
        {
        iWaiter->AsyncStop();
        }
    delete iBTEngSettings;
    
    if ( iFeatureManagerInitialized )
        {
        FeatureManager::UnInitializeLib();
        }
    
    FLOG(_L("[BTSU]\t CBTServiceStarter::Destructor() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::StartServiceL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::StartServiceL( TBTServiceType aService, 
                                       CBTServiceParameterList* aList,
                                       CActiveSchedulerWait* aWaiter )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::StartServiceL()"));
   
	// Delete old list and take ownership of the new one
    //
	
	delete iList;
	iList = aList;

    if ( iServiceStarted )
        {
        User::Leave( KErrInUse );
        }
    if ( !ValidParameters( aService, aList ) )
        {
        User::Leave( KErrArgument );
        }	
    
    // Store the waiter pointer, a NULL value is also accepted
    //
    iWaiter = aWaiter;

    // Store the requested service
    //
    iService = aService;

    if( !iBTEngSettings )
        {
        iBTEngSettings = CBTEngSettings::NewL( this );
        }
    TBTPowerStateValue power = EBTPowerOff;
    User::LeaveIfError( iBTEngSettings->GetPowerState( power ) );
    TBool offline = EFalse;
    if( !power )
        {
        //offline = CheckOfflineModeL();
         offline = EFalse;
        }
    if( !offline )
        {
        FLOG(_L("[BTSU]\t CBTServiceStarter::StartServiceL() Phone is online, request temporary power on."));
		TurnBTPowerOnL( power );
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::StartServiceL() completed"));
    }
    


// -----------------------------------------------------------------------------
// CBTServiceStarter::ControllerComplete
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::ControllerComplete( TInt aStatus )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ControllerComplete()"));    
    TInt error;
    if ( iAllSend ) //stop transfer if everything is send
        {
        StopTransfer( aStatus );	      	        
        return;    
        }
    if(( aStatus==KErrNone || aStatus==EBTSNoError ) && 
        iState == EBTSStarterFindingBIP )
        {
        iBytesSendWithBIP=0;
        if( iProgressGetter )
            {
            iBytesSendWithBIP=iProgressGetter->GetProgressStatus();
            iProgressGetter=NULL;
            delete iController; 
            iController = NULL;            
            }
        if ( !iBTEngDiscovery )
            {
            TRAPD(err, iBTEngDiscovery = CBTEngDiscovery::NewL(this) );
            if (err != KErrNone )
                {
                StopTransfer(EBTSPuttingFailed);	      	
                return;
                }
            }
        error=iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),
                               TUUID(KBTServiceOPPSending));	                 
        if( error == KErrNone )
	      	{
	      	iState = EBTSStarterFindingOPP;	
        	}
        else
        	{
        	StopTransfer(EBTSPuttingFailed);	      	
       		}		                           
        }
    else
        {
        StopTransfer( aStatus );	      	        
        }   
    FLOG(_L("[BTSU]\t CBTServiceStarter::ControllerComplete() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::GetProgressStatus
// -----------------------------------------------------------------------------
//
TInt CBTServiceStarter::GetProgressStatus()
    {    
    if ( iProgressGetter )
        {
        return iProgressGetter->GetProgressStatus()+iBytesSendWithBIP;       
        }
    else
        {
        return iBytesSendWithBIP;
        }
    }


// -----------------------------------------------------------------------------
// CBTServiceStarter::ValidParameters
// -----------------------------------------------------------------------------
//
TBool CBTServiceStarter::ValidParameters( 
    TBTServiceType aService, const CBTServiceParameterList* aList) const
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ValidParameters()"));

    TBool result = EFalse;

    if ( aList != NULL )
        {
        switch ( aService )
            {
            case EBTSendingService:
                {
                // Sending service must have object or image parameters
                //
                if ( aList->ObjectCount() > 0 || aList->ImageCount() > 0 )
                    {
                    result = ETrue;
                    }
                break;
                }
            case EBTPrintingService:
                {
                // Printing service must have xhtml parameters
                //
                if ( aList->XhtmlCount() > 0 )
                    {
                    result = ETrue;
                    }       
                break;
                }
            case EBTObjectPushService:
                {
                // Sending service must have object or image parameters
                //
                if ( aList->ObjectCount() > 0 || aList->ImageCount() > 0 )
                    {
                    result = ETrue;
                    }      
                break;
                }
            default:
                {
                result = EFalse;
                break;
                }
            }
        }
    FTRACE(FPrint(_L("[BTSU]\t CBTServiceStarter::ValidParameters() completed with %d"), result ) );

    return result;
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::StartProfileSelectL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::StartProfileSelectL( TBTServiceType aService )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL()"));    
    LaunchWaitNoteL();   
    iAllSend=EFalse;  
    iSendToBIPOnlyDevice = EFalse;
    
    if ( !iBTEngDiscovery )
        {
        iBTEngDiscovery = CBTEngDiscovery::NewL(this);
        }
    
    if ( !FeatureManager::FeatureSupported( KFeatureIdBtImagingProfile ) && (aService != EBTPrintingService) )
        {
        // If BTimagingProfile is disabled, use OPP instead.
        User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(), 
                                                                                       TUUID(KBTServiceOPPSending)));
        iState = EBTSStarterFindingOPP;  
        return;
        }
    
    switch ( aService  )
        {
        case EBTSendingService: // Find OPP
            {            
            if ( iList->ObjectCount() > 0 )  // We try to send files with OPP profile as long as it contains non-bip objects
                {
                FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() OPP"));    
                User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(), 
                                                                                TUUID(KBTServiceOPPSending)));
                iState = EBTSStarterFindingOPP;          
                }
            else if(iList->ObjectCount() == 0 && iList->ImageCount() > 0)
                {
                FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() BIP")); 
                User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),
                                                                                TUUID(KBTServiceImagingResponder)));
                iState = EBTSStarterFindingBIP;
                }
            break;
            }
        case EBTPrintingService: // Find BPP
            {
            FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() BPP"));
            User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),
                TUUID(KBTServiceDirectPrinting)));
            iState = EBTSStarterFindingBPP;            
            break;
            }
        case EBTObjectPushService: // Find BIP
            {
            FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() BIP"));
            User::LeaveIfError( iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),
                TUUID(KBTServiceOPPSending)));
            iState = EBTSStarterFindingOPP;            
            break;
            }
        default:
            {
            FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() ERROR, unhandled case"));            
            break;
            }
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileSelectL() completed"));
    }
    

// -----------------------------------------------------------------------------
// CBTServiceStarter::StartProfileL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::StartProfileL( TBTServiceProfile aProfile )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileL()"));

    switch ( aProfile )
        {
        case EBTSBPP:
            {            
            iController = CBTSBPPController::NewL( this, iClientChannel, 
                                                   iDevice->BDAddr(), iList,
                                                   iBTEngDiscovery );
            break;
            }
        case EBTSOPP:
            {
            iController = CBTSOPPController::NewL( this, iClientChannel, 
                                                   iDevice->BDAddr(), iList );
            break;
            }
        case EBTSBIP:
            {
            iController = CBTSBIPController::NewL( this, iClientChannel, 
                                                   iDevice->BDAddr(), iList );
            break;
            }
        case EBTSNone:
        default:
            {
            FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileL() ERROR, unhandled case"));            
            break;
            }
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::StartProfileL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::LaunchWaitNoteL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::LaunchWaitNoteL()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchWaitNoteL()"));
    if ( iService == EBTPrintingService )
        {
 //       iDialog->LaunchWaitDialogL( R_BT_PRINTING_WAIT_NOTE );
        }
    else
        {
    //todo need to do the localisation here
        _LIT(KConnectText, "Connecting...");
        iDialog->LaunchWaitDialogL(KConnectText);
        
        }    
    FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchWaitNoteL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::CancelWaitNote
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::CancelWaitNote()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::CancelWaitNote()"));

    if ( iDialog )
        {
        iDialog->CancelWaitDialog();
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::CancelWaitNote() completed"));
    }


// -----------------------------------------------------------------------------
// CBTServiceStarter::LaunchProgressNoteL
// -----------------------------------------------------------------------------
//
//todo reusing the second param aTotalSize to aFileCount as it is no longer used check it being used in other places
void CBTServiceStarter::LaunchProgressNoteL( MBTServiceProgressGetter* aGetter,
                                              TInt aFileCount)
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchProgressNoteL()"));
    RBuf deviceName;
    if ( iService != EBTPrintingService )
        {    
        iProgressGetter = aGetter;     
        CancelWaitNote();        
        
        if ( !iProgressDialogActive )
        	{
            // todo need to change the last parameter because we are now using the textmap id which is a string
            // whether we replace it with that or remove the parameter is to be decided
        	iMessageServerIndex = TObexUtilsMessageHandler::CreateOutboxEntryL( 
            KUidMsgTypeBt, 0 );     

            deviceName.CreateL(KMaxDesCLength);
            
            if ( iDevice->IsValidFriendlyName() )
                {
                deviceName.Copy( iDevice->FriendlyName() );
                }
            else 
                {
                deviceName.Copy( BTDeviceNameConverter::ToUnicodeL(iDevice->DeviceName()));
                }        	
        	iDialog->LaunchProgressDialogL( this, aFileCount, 
                                deviceName, KBTProgressInterval );	
        	deviceName.Close();
        	}        
        iProgressDialogActive=ETrue;     
        }

    FLOG(_L("[BTSU]\t CBTServiceStarter::LaunchProgressNoteL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::UpdateProgressNoteL
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::UpdateProgressNoteL(TInt aFileSize,TInt aFileIndex, const TDesC& aFileName )
    {
    
    iDialog->UpdateProgressNoteL(aFileSize,aFileIndex,aFileName);
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::CancelProgressNote
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::CancelProgressNote()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::CancelProgressNote()"));

    if ( iDialog )
        {
        iDialog->CancelProgressDialog();
        }
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::DialogDismissed
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::DialogDismissed(TInt aButtonId )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::DialogDismissed()"));   
    if( aButtonId == ECancelButton )
        { // this condition is hit for the progress dialog and connecting dialog cancel
        FLOG(_L("[BTSU]\t CBTServiceStarter::DialogDissmissed(), cancelled by user"));        
        iUserCancel=ETrue;
        if ( iController )
            {
            iController->Abort();
            }
        else 
           {
           StopTransfer(KErrCancel);
           }    
        }
    else if ( aButtonId == ENoButton )
        {
        // user abortion
        //
        iUserCancel = ETrue;
        StopTransfer( KErrCancel );
        CancelWaitNote();
        }
    else //EYesButton
        {// this condition is hit when user is ok with sending the unsupported images
        iController->SendUnSupportedFiles();
        }
    FLOG(_L("[BTSU]\t CBTServiceStarter::DialogDismissed() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::ShowErrorNote
// -----------------------------------------------------------------------------
//
void CBTServiceStarter::ShowErrorNote( TInt aReason ) const
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ShowErrorNote()"));
     
    TBuf<KMaxDesCLength> buf;
    TPtrC sendTextMapId;
    
    if ( iDevice->IsValidFriendlyName() )
        {
        buf.Copy( iDevice->FriendlyName() );
        }
    else 
        {
        TRAP_IGNORE( buf.Copy( BTDeviceNameConverter::ToUnicodeL(iDevice->DeviceName())));
        }
    

    switch ( aReason )
        {
        case EBTSNoError:
            {
            sendTextMapId.Set(KFilesSentText());
            break;
            }
        case EBTSConnectingFailed:
        case EBTSGettingFailed:
        case EBTSPuttingFailed:
        case EBTSNoSuitableProfiles:
      //todo below three enums are not valid and it is not being used at anywhere do we need to have it 
            
//        case EBTSBIPSomeSend:
//        case EBTSBIPOneNotSend:
//        case EBTSBIPNoneSend:
        default:            
            {
            sendTextMapId.Set(KSendingFailedText());
            break;
            }
        }        
    
    
    if(iLocalisationInit)
        {
        TRAP_IGNORE(
               HBufC* sendText = HbTextResolverSymbian::LoadLC(sendTextMapId);
               HBufC* deviceName =  HbTextResolverSymbian::LoadLC(KDeviceText,buf);
               CHbDeviceNotificationDialogSymbian::NotificationL(KNullDesC, deviceName->Des(), sendText->Des());
               CleanupStack::PopAndDestroy( deviceName );
               CleanupStack::PopAndDestroy( sendText );
               );
        }
    else
        {
        TRAP_IGNORE(CHbDeviceNotificationDialogSymbian::NotificationL(KNullDesC, KDeviceText(), sendTextMapId));
        }
    FLOG(_L("[BTSU]\t CBTServiceStarter::ShowErrorNote() completed"));
    }



// -----------------------------------------------------------------------------
// CBTServiceStarter::LaunchConfirmationQuery
// -----------------------------------------------------------------------------
//
void  CBTServiceStarter::LaunchConfirmationQuery(const TDesC& aConfirmText)
    {
    TRAP_IGNORE(iDialog->LaunchQueryDialogL(aConfirmText ));
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::StopTransfer
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::StopTransfer(TInt aError)
	{
      
	FLOG(_L("[BTSU]\t CBTServiceStarter::StopTransfer()"));
    Cancel();
	if( !iUserCancel )
	    {
	    CancelWaitNote();
        CancelProgressNote();
	
        if ( aError != KErrCancel )
            {
            ShowErrorNote( aError );
            }     
        }
    if ( iMessageServerIndex != 0 )
        {                 
        TRAPD( notUsedRetVal, TObexUtilsMessageHandler::DeleteOutboxEntryL( iMessageServerIndex ) );
        notUsedRetVal=notUsedRetVal;
        iMessageServerIndex=0;
        FTRACE(FPrint(_L("[BTSU]\t CBTServiceStarter::StopTransfer() delete ob entry %d"), notUsedRetVal ) );
        }
    // Release resources
    //
    if ( iList )
        {
        delete iList;
        iList = NULL;    
        }
        
    if ( iController )
        {
        delete iController; 
        iController = NULL;
        }
    
    if ( iNotifier.Handle() )
        {
        iNotifier.Close();
        }

    // Reset states
    //
    iServiceStarted = EFalse;
    if ( iWaiter && iWaiter->IsStarted() )
        {                
        iWaiter->AsyncStop();                    
        }    
        
    iState = EBTSStarterStoppingService;	    
	}
	
// -----------------------------------------------------------------------------
// CBTServiceStarter::ConnectTimedOut()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::ConnectTimedOut()
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ConnectTimedOut()"));            
    StopTransfer(EBTSConnectingFailed);    
    FLOG(_L("[BTSU]\t CBTServiceStarter::ConnectTimedOut() completed"));
    }
// -----------------------------------------------------------------------------
// CBTServiceStarter::ServiceSearchComplete()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::ServiceSearchComplete( const RSdpRecHandleArray& /*aResult*/, 
                                         TUint /*aTotalRecordsCount*/, TInt /*aErr */)
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ServiceSearchComplete()"));        
    }

// -----------------------------------------------------------------------------
// CBTServiceStarter::AttributeSearchComplete()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::AttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, 
                                           const RSdpResultArray& /*aAttr*/, 
                                           TInt /*aErr*/ )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::AttributeSearchComplete()"));           
    }
// -----------------------------------------------------------------------------
// CBTServiceStarter::ServiceAttributeSearchComplete()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::ServiceAttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, 
                                                          const RSdpResultArray& aAttr, 
                                                          TInt aErr )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::ServiceAttributeSearchComplete()"));               
    TInt err = KErrNone;
    if ((aErr==KErrEof || aErr==KErrNone) && aAttr.Count()>0 )
        {            
        RSdpResultArray results=aAttr;    
        iBTEngDiscovery->ParseRfcommChannel(results,iClientChannel);          

        iBTEngDiscovery->CancelRemoteSdpQuery();
               
        switch (iState)
            {
            case EBTSStarterFindingBIP:
                {
                TRAP(err, StartProfileL( EBTSBIP ));  
                iTriedBIP = ETrue;
                if (err != KErrNone)
                    {
                    StopTransfer(EBTSConnectingFailed);        
                    }
                if ( iSendToBIPOnlyDevice )
                    {
                    iAllSend = ETrue;
                    }
                else
                    {
                    if(iList->ObjectCount() == 0)
                        {
                        iAllSend=ETrue;
                        }
                    }
                break;
                }
            case EBTSStarterFindingOPP:
                {
                TRAP(err, StartProfileL( EBTSOPP ) ); 
                iTriedOPP = ETrue;
                if (err != KErrNone)
                    {
                    StopTransfer(EBTSConnectingFailed);        
                    }
                iAllSend=ETrue;  
                break;
                }            
            case EBTSStarterFindingBPP:          
                {
                TRAP(err, StartProfileL( EBTSBPP ));           
                if (err != KErrNone)
                    {
                    StopTransfer(EBTSConnectingFailed);        
                    }               
                break;    
                }            
            }       
        }      
    else if ( aErr==KErrEof && aAttr.Count()==0 && 
              iState == EBTSStarterFindingBIP && !iTriedBIP )
        {
        iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),TUUID(KBTServiceOPPSending));
        iState = EBTSStarterFindingOPP;    
		iTriedBIP = ETrue;
        }    
    else if ( aErr==KErrEof && aAttr.Count()==0 &&
              iState == EBTSStarterFindingOPP && !iTriedOPP &&
              iList->ImageCount() > 0 &&
              FeatureManager::FeatureSupported( KFeatureIdBtImagingProfile ) )
        {
        iSendToBIPOnlyDevice = ETrue;
        iBTEngDiscovery->RemoteProtocolChannelQuery(iDevice->BDAddr(),TUUID(KBTServiceImagingResponder));
        iState = EBTSStarterFindingBIP;  
        iTriedOPP = ETrue;
        }
    else if (aErr==KErrNone && aAttr.Count()==0)
        {
        // This isn't KErrEof so we aren't done yet, wait for further matches
        }
    else
        {
        // Set destroyer AO active (destroys CBTEngDiscovery/CBTEngSdpQuery classes). This is done
        // to ensure that CBTEngDiscovery/CBTEngSdpQuery classes have finished all their activities,
        // callbacks etc.. Destructing it self is handled in CBTServiceDelayedDestroyer's RunL.
        iDelayedDestroyer->SetDestructPointer(iBTEngDiscovery);
        iDelayedDestroyer->GoActive();
        // Set iBTEngDiscovery pointer to zero. Pointer doesn't exist CBTServiceStarter point of view anymore.
        iBTEngDiscovery = NULL;    
        StopTransfer(EBTSConnectingFailed);    
        }    
    FLOG(_L("[BTSU]\t CBTServiceStarter::ServiceAttributeSearchComplete() done"));               
    }
// -----------------------------------------------------------------------------
// CBTServiceStarter::DeviceSearchComplete()
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::DeviceSearchComplete( CBTDevice* /*aDevice*/, TInt aErr )
    {
    FLOG(_L("[BTSU]\t CBTServiceStarter::DeviceSearchComplete()"));          
    FTRACE(FPrint(_L("[BTSU]\t CBTServiceStarter DeviceSearchComplete()aErr = %d"), aErr) );     
    if ( aErr == KErrNone )
        {    
        TRAPD(err, StartProfileSelectL( iService ));
        if (err != KErrNone )
            {
            StopTransfer(err);            
            }
            
        iServiceStarted = ETrue;
        }
    else
        {
        if ( aErr == KErrCancel )
            {
            iUserCancel=ETrue;
            }
        StopTransfer(aErr);    
        }    
    FLOG(_L("[BTSU]\t CBTServiceStarter::DeviceSearchComplete() done"));                   
    }        

// -----------------------------------------------------------------------------
// From class MBTEngSettingsObserver.
// Power has changed, start searching for BT devices.
// -----------------------------------------------------------------------------
//  
void CBTServiceStarter::PowerStateChanged( TBTPowerStateValue aState )
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::PowerStateChanged()"));	
    if( aState == EBTPowerOn && iWaitingForBTPower )
        {
        iWaitingForBTPower = EFalse;
        TRAP_IGNORE( StartDiscoveryL() );
        }
	FLOG(_L("[BTSU]\t CBTServiceStarter::PowerStateChanged() - completed"));
    }


// -----------------------------------------------------------------------------
// From class MBTEngSettingsObserver.
// Visibility has changed, ignore event.
// -----------------------------------------------------------------------------
//  
void CBTServiceStarter::VisibilityModeChanged( TBTVisibilityMode aState )
    {
    (void) aState;
    }

// -----------------------------------------------------------------------------
// Check if the phone is in offline mode, and ask the user if it is.
// -----------------------------------------------------------------------------
//  
TBool CBTServiceStarter::CheckOfflineModeL()
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::CheckOfflineModeL()"));	
    TCoreAppUIsNetworkConnectionAllowed offline = ECoreAppUIsNetworkConnectionNotAllowed;
    TBTEnabledInOfflineMode offlineAllowed = EBTDisabledInOfflineMode;
    User::LeaveIfError( iBTEngSettings->GetOfflineModeSettings( offline, offlineAllowed ) );
    if( offline == ECoreAppUIsNetworkConnectionNotAllowed && 
         offlineAllowed == EBTEnabledInOfflineMode )
        {
        User::LeaveIfError( iNotifier.Connect() );
        TBTGenericQueryNotiferParamsPckg pckg;
        pckg().iMessageType = EBTActivateOffLineQuery;
        pckg().iNameExists = EFalse;
        iActiveNotifier = EOfflineQuery;
        iNotifier.StartNotifierAndGetResponse( iStatus, KBTGenericQueryNotifierUid, 
                                                pckg, iOffline );
        SetActive();
        }
    else if( offline == ECoreAppUIsNetworkConnectionNotAllowed && 
            offlineAllowed == EBTDisabledInOfflineMode )
           {
           StopTransfer( KErrNotSupported );
           }
	FLOG(_L("[BTSU]\t CBTServiceStarter::CheckOfflineModeL() - completed"));
    return ( offline == ECoreAppUIsNetworkConnectionNotAllowed );
    }


// -----------------------------------------------------------------------------
// Start BT device discovery.
// -----------------------------------------------------------------------------
//	
void CBTServiceStarter::StartDiscoveryL()
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::StartDiscoveryL()"));	
    if( !iBTEngDiscovery )
        {
        iBTEngDiscovery = CBTEngDiscovery::NewL(this);
        }
    TInt err = iBTEngDiscovery->SearchRemoteDevice(iDevice );
    if( err )
        {
        StopTransfer( err );
        }
	FLOG(_L("[BTSU]\t CBTServiceStarter::StartDiscoveryL() - completed"));		
    }


// -----------------------------------------------------------------------------
// Turn BT on and start BT device discovery if possible.
// -----------------------------------------------------------------------------
//  
void CBTServiceStarter::TurnBTPowerOnL( const TBTPowerStateValue aState )
    {
	FLOG( _L("[BTSU]\t CBTServiceStarter::TurnBTPowerOnL()") );
//    if (iName() != EFalse) 
    	{
    	if( !iBTEngSettings )
	        {
	        iBTEngSettings = CBTEngSettings::NewL( this );
	        }
	    TInt err = iBTEngSettings->ChangePowerStateTemporarily();
	    iWaitingForBTPower = ETrue;
	    if( err )
	        {
	        iWaitingForBTPower = EFalse;
	        StopTransfer( err );
	        }
	    else if( aState == EBTPowerOn )
	        {
	        // Power is already on, we just registered for turning it off if needed.
	        // Since there is no callback at this point (power is already on), start 
	        // searching straight away.
	        iWaitingForBTPower = EFalse;
	        StartDiscoveryL();
	        }
    	}
 /*   else
    	{
        if ( !iNotifier.Handle() )
	        {
			User::LeaveIfError( iNotifier.Connect() );
	        }
		TBTGenericQueryNotiferParamsPckg pckg;
        pckg().iMessageType = EBTNameQuery;
        pckg().iNameExists = EFalse;
        iActiveNotifier = ENameQuery;
        iNotifier.StartNotifierAndGetResponse( iStatus, KBTGenericQueryNotifierUid, 
                                                  pckg, iName );
        SetActive();
    	}*/
	FLOG(_L("[BTSU]\t CBTServiceStarter::TurnBTPowerOnL() - completed"));
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been cancelled.
// ---------------------------------------------------------------------------
//
void CBTServiceStarter::DoCancel()
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::DoCancel()"));
    iNotifier.CancelNotifier( KBTGenericQueryNotifierUid );
    iNotifier.Close();
	FLOG(_L("[BTSU]\t CBTServiceStarter::DoCancel() - completed"));
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been completed.
// ---------------------------------------------------------------------------
//
void CBTServiceStarter::RunL()
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::RunL()"));
    TInt err = iStatus.Int();
    if( !err )
        {
        if ( (iActiveNotifier == ENameQuery && iName() != EFalse ) || ( iActiveNotifier == EOfflineQuery && iOffline() != EFalse ) )
            {
            TBTPowerStateValue power = EBTPowerOff;
            if ( iNotifier.Handle() )
                {
                iNotifier.Close();
                }
            User::LeaveIfError( iBTEngSettings->GetPowerState( power ) );
            TurnBTPowerOnL( power );
            }
        else
            {
            err = KErrCancel;
            }    
        }

    if( err )
        {
      
        err = ( err == KErrNotSupported ? KErrCancel : err );
        if ( iWaiter && err != KErrInUse && err != KErrCancel )
            {
            err = EBTSPuttingFailed;
            }
        StopTransfer( err );
        }
    
    FLOG(_L("[BTSU]\t CBTServiceStarter::RunL() - completed"));	
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when an error in RunL has occurred.
// ---------------------------------------------------------------------------
//
TInt CBTServiceStarter::RunError( TInt aError )
    {
	FLOG(_L("[BTSU]\t CBTServiceStarter::RunError()"));
    StopTransfer( aError );
	FLOG(_L("[BTSU]\t CBTServiceStarter::RunError() - completed"));
    return KErrNone;
    }




