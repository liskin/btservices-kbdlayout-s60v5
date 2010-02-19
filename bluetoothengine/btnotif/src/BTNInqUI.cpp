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
* Description:  Declares Bluetooth device inquiry user interface class.
*
*/


// INCLUDE FILES
#include <avkon.hrh>            // AVKON components
#include <AknIconArray.h>
#include <aknlists.h>
#include <aknPopup.h>
#include <avkon.mbg>
#include <wlaninternalpskeys.h> // For WLAN state checking
#include <ctsydomainpskeys.h>
#include <AknNotiferAppServerApplication.h> 
#include <barsread.h>           // Resource reader
#include <BTNotif.rsg>          // Own resources
#include <bt_subscribe.h>
#include <btnotif.mbg>
#include <wlanplugin.mbg>       // Borrow WLan signal strenth bar to show RSSI  
#include <devui_const.h>
#include <btengutil.h>
#include <bluetoothuiutil.h>
#include "btninqui.h"           // Own class definition
#include "btNotifDebug.h"       // Debugging macros
#include "btnotifnameutils.h"


const TInt KBTAllPurposeBufferLength = 266;
const TInt KBTNotifNonPairedUsedDevicesMaxNumber= 5;
// RSSI value range: -127dB ~ +20dB
const TInt KRssiRangeOffset = 127 + 1;  // Offset for getting a non-zero positive value
const TInt KMediumStrength = 53;
const TInt KHighStrength = 82;
const TUint32 ExcludePairedDeviceMask = 0x800000;
const TUint32 ResetExcludePairedDeviceMask = 0xFF7FFFFF;
_LIT(KBtnotifBmpFileName,"btnotif.mbm");
_LIT(KWLanBmpFileName,"wlanplugin.mif");

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTInqUI::CBTInqUI
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTInqUI::CBTInqUI( MBTNDeviceSearchObserver* aObserver, 
        CBTNotifUIUtil* aUtil, 
        const TBTDeviceClass& aDesiredDevClass): 
        iUiUtil (aUtil),
        iEikonEnvRef( iUiUtil->LocalEikonEnv() ),
        iDevicesFound (EFalse),
        iBroughtForwards (EFalse),
        iDesiredDeviceClass (aDesiredDevClass),
        iSystemCancel (EFalse), 
        iPageForName ( EFalse ),
        iIndex (0),
        iDevSearchObserver (aObserver)
    {
	iBTRegistryQueryState=ENoQuery;
	}

// ----------------------------------------------------------
// CBTInqUI::ConstructL
// Symbian 2nd phase constructor can leave.
// Creates first device list (not shown yet) and initializes 
// couple of member variables with default values.
// ----------------------------------------------------------
//
void CBTInqUI::ConstructL()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::ConstructL()"));
    
    // Read default device names to an array
    iDefaultDeviceNamesArray = iEikonEnvRef.ReadDesCArrayResourceL( R_BT_DEFAULT_DEVICE_NAMES );

    // create the timer
    iPeriodicTimer = CPeriodic::NewL(CActive::EPriorityIdle);
    
    // Create device array
    //
    iDeviceListRows = new(ELeave) CDesCArrayFlat(1);
    
    iPairedDevicesArray = new (ELeave) CBTDeviceArray(1);
    iLastUsedDevicesArray = new(ELeave) CBTDeviceArray(1);
    iLastSeenDevicesArray = new(ELeave) CBTDeviceArray(1);
	iAdjustedUsedDeviceArray = new(ELeave) CBTDeviceArray(1);	
	
	iDevMan=CBTEngDevMan::NewL(this);
	
	iExcludePairedDevices = iDesiredDeviceClass.DeviceClass() & ExcludePairedDeviceMask ? 
                            ETrue : EFalse;	
    if ( iExcludePairedDevices )
        {         
        FLOG (_L("[BTNOTIF]\t CBTInqUI::ConstrucL Excluding paired devices"));
        iDesiredDeviceClass = iDesiredDeviceClass.DeviceClass() & ResetExcludePairedDeviceMask;
        }
    // Retrieve paired devices to a local array
    RetrievePairedDevices();

    FLOG(_L("[BTNOTIF]\t CBTInqUI::ConstructL() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::RetrieveUsedDevices- Substitute
// ----------------------------------------------------------
//
void CBTInqUI::RetrieveUsedDevices()
	{
	FLOG(_L("[BTNOTIF]\t CBTInqUI::RetrieveUsedDevicesL()"));	

	TBTRegistrySearch searchPattern;
	searchPattern.FindAll();
	
    //Ignore this error. Continue device search.
	TInt err=iDevMan->GetDevices( searchPattern, iLastUsedDevicesArray );	
	if(err)
	    {
	    TRAP_IGNORE(AdjustDeviceArrayL(iAdjustedUsedDeviceArray));
	    DoDeviceFrontListSelection();
	    FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI: get useddevice err %d"), err));  
	    }
	else
	    {
	    iBTRegistryQueryState=EQueryUsed;
	    }
	FLOG(_L("[BTNOTIF]\t CBTInqUI::RetrieveUsedDevicesL() returned"));
	}
	
//---------------------------------------------------------------
// CBTInqUI::RetrievePairedDevices
//---------------------------------------------------------------
void CBTInqUI::RetrievePairedDevices()
    {
	FLOG(_L("[BTNOTIF]\t CBTInqUI::RetrievePairedDevicesL()"));

	TBTRegistrySearch searchPattern;
    searchPattern.FindBonded();

	TInt err= iDevMan->GetDevices( searchPattern, iPairedDevicesArray );	
	FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI: get paireddevice err %d"), err));  
	if(err)
	    {
        //Next: try to get used devices
	    RetrieveUsedDevices();
	    }
	else
	    {
	    iBTRegistryQueryState=EQueryPaired;	
	    }
	FLOG(_L("[BTNOTIF]\t CBTInqUI::RetrievePairedDevicesL() returned"));
    }
    
// ----------------------------------------------------------
// CBTInqUI::HandleGetDevicesComplete
// ----------------------------------------------------------
//
void CBTInqUI::HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* /*aDeviceArray*/)
	{
	FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleGetDevicesComplete() err: %d"), aErr));	
	(void) aErr;
	switch(iBTRegistryQueryState)
		{
		case EQueryPaired:
		    // If a device is paired in Just Works model but without user awareness,
		    // This device won't be shown in UI paired view, so we have to 
		    // remove it from this array so that it will be shown to user
		    // in the last used device list.
		    for (int i = iPairedDevicesArray->Count() - 1; i >= 0; --i )
		        {
		        if ( !IsUserAwarePaired( iPairedDevicesArray->At( i )->AsNamelessDevice() ) )
		            {
		            delete iPairedDevicesArray->At( i );
		            iPairedDevicesArray->Delete( i );
		            }
		        }
			RetrieveUsedDevices();
			break;
		case EQueryUsed:
			// fill in the list of last seen devices, so it can be shown
			TRAP_IGNORE(AdjustDeviceArrayL(iAdjustedUsedDeviceArray));
			DoDeviceFrontListSelection();
			break;
		default:
			break;
		}
	
	FLOG(_L("[BTNOTIF]\t CBTInqUI::HandleGetDevicesComplete() Complete"));		
	}

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTInqUI::~CBTInqUI()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::~CBTInqUI()"));

    if( iBroughtForwards )
        {
        iEikonEnvRef.BringForwards(EFalse);
        iBroughtForwards = EFalse;
        }

    if( iPairedDevicesArray ) 
        {
        iPairedDevicesArray->ResetAndDestroy();	            
        delete iPairedDevicesArray; 
        }
    if( iLastUsedDevicesArray ) 
        {  
        iLastUsedDevicesArray->ResetAndDestroy();	    
        delete iLastUsedDevicesArray; 
        }       
    if( iLastSeenDevicesArray ) 
        {  
        iLastSeenDevicesArray->ResetAndDestroy();
        delete iLastSeenDevicesArray; 
        }
    if( iAdjustedUsedDeviceArray )
        {  
        iAdjustedUsedDeviceArray->ResetAndDestroy();
        delete iAdjustedUsedDeviceArray; 
        }
    if( iDefaultDeviceNamesArray )
        {
        iDefaultDeviceNamesArray->Reset();
        delete iDefaultDeviceNamesArray;
        }
    
	delete iDevMan;    
    delete iDeviceListBox;
    delete iDeviceListRows;
	delete iPeriodicTimer;	
	    
    FLOG(_L("[BTNOTIF]\t CBTInqUI::~CBTInqUI() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::Cancel
// ----------------------------------------------------------
//
void CBTInqUI::Cancel()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::Cancel()"));  
    
    iSystemCancel = ETrue;
    if(iDevMan)
 	   {
 	   iDevMan->Cancel();  	
  	   }

    if( iSearchFilterPopupList )
        {
        iSearchFilterPopupList->CancelPopup();
        }

    if(iPeriodicTimer)
        {
        iPeriodicTimer->Cancel();
        }
    
    RemoveScanner();
    InquiryComplete(KErrCancel);

    FLOG(_L("[BTNOTIF]\t CBTInqUI::Cancel() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::DisplayDevicesFrontListL
// Display last seen devices popuplist
// ----------------------------------------------------------
//
void CBTInqUI::DisplayDevicesFrontListL()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::DisplayDevicesFrontListL()"));

    
    if ( iAdjustedUsedDeviceArray->Count() <= 0 )
        {
        SelectSearchCategoryL();
        return;
        }
    
    if( !iBroughtForwards )
        {
        iEikonEnvRef.BringForwards(ETrue); // Destructor will release this later on
        iBroughtForwards = ETrue;
        }

    // Create the devices popuplist
    CreatePopupListL( R_AVKON_SOFTKEYS_SELECT_CANCEL__SELECT , R_BT_LAST_USED_DEVS_POPUP_TITLE );

    // Add devices into device list.
    for( TInt index = 0; index < iAdjustedUsedDeviceArray->Count(); index++ )
    	{        
   	    UpdateDeviceListL ( iAdjustedUsedDeviceArray->At(index) );  
        }  

    // Add "more devices" command as first item of list
    HBufC* formatString = HBufC::NewLC( KBTAllPurposeBufferLength );
    formatString->Des().Copy( TPtrC(KDeviceIconFormatTable[EDeviceIconBlank].iFormat ) );
    HBufC* moreDevicesString = iEikonEnvRef.AllocReadResourceLC( R_BT_MORE_DEVICES_ITEM );
    formatString->Des().Append( *moreDevicesString );
    CleanupStack::PopAndDestroy(); // moreDevicesString
    iDeviceListRows->InsertL( 0, *formatString );
    CleanupStack::Pop(); // formatString

    // Launch popup list and wait for user input 
    AllowDialerAndAppKeyPress( EFalse );
    TInt popupRetVal = iDevicePopupList->ExecuteLD();    
    iDevicePopupList = NULL;   
    AllowDialerAndAppKeyPress( ETrue );
    
    if( !popupRetVal )
        { 
        // query cancelled by user or system, complete device search:
        iDevSearchObserver->NotifyDeviceSearchCompleted(KErrCancel);
        return;
        }
    TInt currentDeviceIndex = iDeviceListBox->CurrentItemIndex();
    
    // Delete device list box and reset device items array
    delete iDeviceListBox;
    iDeviceListBox = NULL;
    iDeviceListRows->Reset();

    if( currentDeviceIndex > 0 ) // User selected "a used device"
        {
        HandleDeviceSelectionL( iAdjustedUsedDeviceArray->At( currentDeviceIndex - 1 ) );   
        return;
        }
    
    // user selected more devices.
    SelectSearchCategoryL();
    FLOG(_L("[BTNOTIF]\t CBTInqUI::DisplayDevicesFrontListL() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::DeviceSearchUiL
// Execute bluetooth device search/inquiry.
// Parameters:
//  OUT: aDevice     - selected device after inquiry
//  IN:  aObexOnly   - flag if OBEX capable devices is requested
// ----------------------------------------------------------
//
void CBTInqUI::DeviceSearchUiL()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::DeviceSearchUiL()"));
    
    TSearchFlowState searchFlow;
    TInt err;
    AllowDialerAndAppKeyPress( EFalse );
    do
        {
        searchFlow = InitInquiryL(err);
        } while (searchFlow == ESearchAgain);
    
    if (searchFlow == ESearchCompleted)
        {
        // search completed caused by either user cancel or failure
        iDevSearchObserver->NotifyDeviceSearchCompleted(err);
        AllowDialerAndAppKeyPress( ETrue );
        return;
        }
 
    FLOG(_L("[BTNOTIF]\t CBTInqUI, Displaying devices already found"));

    TInt keypress(0);
    // Show devices popup list for received devices while
    // inquiry is still active and list may be updated.
    if( !iInquiryComplete )
        {
        CreatePopupListL( R_BT_SOFTKEYS_SELECT_STOP, R_BT_SEARCH_DEVS_POPUP_TITLE );

        // Set animation
        iDevicePopupList->Heading()->SetHeaderAnimationL( R_BT_ANIMATION_FOR_POPUPLIST );

        // Launch popup list and wait for user input
        keypress = iDevicePopupList->ExecuteLD();    
        iDevicePopupList = NULL;
        }

    // If Cancel() is called while DoInquery() is still running,
    // such as taking out MMC while bt devices discovery or user pressed redkey.                 
    // In such cases, We need to dismiss anything and function returns immediately.
    //
    if (iSystemCancel )
        {
        FLOG(_L("[BTNOTIF]\t inquiry was canncelled by the system!"));
        iSystemCancel = EFalse;
        AllowDialerAndAppKeyPress( ETrue );
        iDevSearchObserver->NotifyDeviceSearchCompleted(KErrCancel);
        return;
        }

    // Cancel inquiry
    RemoveScanner();
    
    // Store the highlighted device index and destroy list box
    iHighLightedItemIndex = 0;
    if( iDeviceListBox )
        {
        iHighLightedItemIndex = iDeviceListBox->CurrentItemIndex();
        delete iDeviceListBox;
        iDeviceListBox = NULL;
        }
    
    // user selected a device during active inquiry
    if (keypress)
        {
        AllowDialerAndAppKeyPress( ETrue );
        HandleDeviceSelectionL( iLastSeenDevicesArray->At( iHighLightedItemIndex ) );
        return;
        }

    // Error encountered in inquiry
    if (iInquiryStatus)
        {
        AllowDialerAndAppKeyPress( ETrue );
        iDevSearchObserver->NotifyDeviceSearchCompleted(iInquiryStatus);
        return;
        }
    
    // Inquiry completed, show final device list 
    FLOG(_L("[BTNOTIF]\t CBTInqUI::DeviceSearchUiL Displaying final devicelist"));          
    CreatePopupListL( R_AVKON_SOFTKEYS_SELECT_CANCEL__SELECT, R_BT_FOUND_DEVS_POPUP_TITLE );
   
    //cancel iPeriodicTimer after the final list is shown
    iPeriodicTimer->Cancel();
    User::ResetInactivityTime();
    // Set highlight on top of same device as it was in previous popuplist
    if( iHighLightedItemIndex >= 0 )
        {
        iDeviceListBox->HandleItemAdditionL();
        iDeviceListBox->SetCurrentItemIndex(iHighLightedItemIndex);
        }

    // Launch popup list and wait for user input
    keypress = iDevicePopupList->ExecuteLD();
    iDevicePopupList = NULL;
    AllowDialerAndAppKeyPress( ETrue );
    // If dialog is cancelled by system:
    if( !keypress)
        {
        iDevSearchObserver->NotifyDeviceSearchCompleted(KErrCancel);
        return;
        }

    // Get user selection
    TInt index = iDeviceListBox->CurrentItemIndex();
    delete iDeviceListBox;
    iDeviceListBox = NULL;

    HandleDeviceSelectionL( iLastSeenDevicesArray->At( index ) );

    FLOG(_L("[BTNOTIF]\t CBTInqUI::DeviceSearchUiL() complete"));    
    }

TSearchFlowState CBTInqUI::InitInquiryL(TInt& aReason)
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::InitInquiryL"));
    if (IsActiveVoIPOverWLAN())
        {
        iUiUtil->ShowInfoNoteL( R_BT_NO_DEVICE_DISCOVERY_VOIP, ECmdBTnotifUnavailable );
        aReason = KErrInUse;
        return ESearchCompleted;
        }

    // starting  the timer
    iPeriodicTimer->Cancel();
    iPeriodicTimer->Start(KPeriodicTimerInterval4Sec, KPeriodicTimerInterval4Sec, 
                        TCallBack(PeriodicTimerCallBack, this));
    
    // Prepare for using hostresolver to search devices 
    CreateScannerL();
    
    iInquiryStatus = KErrCancel; // Give a initial value (for end key handling)
    iDevicesFound = EFalse;
    iInquiryComplete = EFalse;
    iSystemCancel = EFalse;   
    
    // Show wait note until dialog is dismissed by inquirey result callbacks or user cancel
    TInt ret = iUiUtil->ShowWaitDlgL( R_BT_SEARCHING_DEVICES_NOTE );
    
    // If user cancels searching, ret will be KerrNone.  
    // According to Description of Eikdialog, ShowWaitDlgL() returns zero 
    // if it was the cancel button (@c EEikBidCancel). 
    if( ret == KErrNone || iSystemCancel )
        {
        FLOG(_L("[BTNOTIF]\t CBTInqUI, User cancelled waiting note"));
        RemoveScanner();
        aReason = KErrCancel;
        return ESearchCompleted;
        }

     if( !iDevicesFound ) // No devices found at all
         {
         FLOG(_L("[BTNOTIF]\t CBTInqUI no devices found at all"));
         RemoveScanner(); // Cancel inquiry

         if( iInquiryStatus != KErrNone )
             {
             aReason = iInquiryStatus;
             return ESearchCompleted;
             }
         
         // Show try again query
         TBool keypress(0);
         keypress = iUiUtil->ShowQueryL( KErrNone, R_BT_TRY_AGAIN_QUERY, ECmdBTnotifUnavailable );
         if( !keypress ) // User has not requested new inquiry
             {
             aReason = KErrCancel;
             return ESearchCompleted;
             }
         else
             {
             FLOG(_L("[BTNOTIF]\t CBTInqUI user wanted to try again"));
             return ESearchAgain;
             }
         }
     return ESearchFirstDeviceFound;
    }

// ----------------------------------------------------------
// CBTInqUI::DeviceAvailableL
// Bluetooth device has been received.
// ----------------------------------------------------------
//
void CBTInqUI::DeviceAvailableL( const TNameRecord& aNameRecord, const TDesC& aDevName )
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::DeviceAvailableL()"));

    if( !iDevicesFound )
        {
        FLOG(_L("[BTNOTIF]\t CBTInqUI::DeviceAvailableL() first found, clear last seen devices array"));
        iLastSeenDevicesArray->ResetAndDestroy(); // Remove previous inquiry results
        iDevicesFound = ETrue; 
        iInquiryStatus = KErrNone;
        }
    
    iUiUtil->CompleteWaitDlgL();

    TInquirySockAddr& sa = TInquirySockAddr::Cast( aNameRecord.iAddr );
    
    // Create device object with received CoD information
    CBTDevice* newDevice = CBTDevice::NewLC( sa.BTAddr() );      
    TBTDeviceClass cod( sa.MajorServiceClass(), sa.MajorClassOfDevice(), sa.MinorClassOfDevice() );
    newDevice->SetDeviceClass( cod );
    FTRACE(FPrint(_L("[BTNOTIF]\t CoD %b"), cod.DeviceClass() ));
    TInt rssi = 0;
    if( sa.ResultFlags() & TInquirySockAddr::ERssiValid )
        {
        rssi = sa.Rssi() + KRssiRangeOffset;
        }

    BtNotifNameUtils::SetDeviceNameL(aDevName, *newDevice);

    // Update device popup list with newDevice
    UpdateDeviceListL( newDevice, rssi );

    // Append newDevice in the bottom of the "last seen" device array.
    iLastSeenDevicesArray->AppendL(newDevice);
    CleanupStack::Pop(); // new device is under iLastSeenDevicesArray control now

    FLOG(_L("[BTNOTIF]\t CBTInqUI::DeviceAvailableL() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::InquiryComplete
// Inquiry has been completed.
// ----------------------------------------------------------
//
void CBTInqUI::InquiryComplete( TInt aError )
    {
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::InquiryComplete() status = %d: %d devices found"), aError, iLastSeenDevicesArray->Count() ));
    iInquiryComplete = ETrue;
    // Remove popup list if any devices exists
    if( iDevicePopupList )
        {
        iDevicePopupList->CancelPopup();
        iDevicePopupList = NULL;
        }
    
    TRAP_IGNORE( iUiUtil->CompleteWaitDlgL() );
    
    if ( aError == KErrNone || aError == KErrCancel || aError == KErrEof )
        {
        iInquiryStatus = (aError == KErrEof ) ? KErrNone : aError;
        }
    else
        {
        iLastSeenDevicesArray->ResetAndDestroy(); // Remove previous inquiry results
        }

    FLOG(_L("[BTNOTIF]\t CBTInqUI::InquiryComplete() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::PeriodicTimerCallBack(TAny* aAny)
// The call back function
// ----------------------------------------------------------
//
TInt CBTInqUI::PeriodicTimerCallBack(TAny* /*aAny*/)
    {
    User::ResetInactivityTime();
    return KErrNone; // Return value ignored by CPeriodic
    }

// ----------------------------------------------------------
// CBTInqUI::RemoveScanner
// Remove scanner active object if still exists.
// ----------------------------------------------------------
//
void CBTInqUI::RemoveScanner()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::RemoveScanner()"));
    if( iScanner )
        {
        iScanner->CancelRequest();
        iHostResolver.Close();
        iSocketServer.Close(); 
        delete iScanner; 
        iScanner = NULL;
        }
    iDevsWithoutName.Close();
    FLOG(_L("[BTNOTIF]\t CBTInqUI::RemoveScanner() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::CreatePopupListL
// ----------------------------------------------------------
//
void CBTInqUI::CreatePopupListL(TInt aSoftkeysResourceId, TInt aTitleResourceId )
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::CreatePopupListL()"));

    __ASSERT_DEBUG( !iDeviceListBox,   User::Panic(_L("BTNotifInqUI - iDeviceListBox not released!"  ), KErrAlreadyExists));
    __ASSERT_DEBUG( !iDevicePopupList, User::Panic(_L("BTNotifInqUI - iDevicePopupList not released!"),KErrAlreadyExists));

    // Create listbox for devices
    iDeviceListBox = new(ELeave) CAknSingleGraphicPopupMenuStyleListBox;

    // Create popup list
    iDevicePopupList = CAknPopupList::NewL( iDeviceListBox, aSoftkeysResourceId, AknPopupLayouts::EDynMenuWindow );

    iDeviceListBox->ConstructL(iDevicePopupList, EAknListBoxMenuList );
    iDeviceListBox->CreateScrollBarFrameL(ETrue);
    iDeviceListBox->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

    // Read title from resources
    HBufC* popupListTitle = iEikonEnvRef.AllocReadResourceLC( aTitleResourceId );
    iDevicePopupList->SetTitleL(*popupListTitle);
    CleanupStack::PopAndDestroy(); // popupListTitle

    // Create list box model for devices
    CTextListBoxModel* model = iDeviceListBox->Model();
    model->SetItemTextArray(iDeviceListRows);
    model->SetOwnershipType(ELbmDoesNotOwnItemArray);  // Do not destroy items

    // Set up filename for Avkon bitmap
    TFileName avkonbmpFilename = AknIconUtils::AvkonIconFileName();

    // Setup graphic items list for BT devices listbox
    CAknIconArray* iconList = new(ELeave) CAknIconArray(KDeviceIconCount);
    CleanupStack::PushL( iconList );
	
	CreateAndAppendIconL( KAknsIIDQgnPropBtComputer, avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_computer,
						  EMbmAvkonQgn_prop_bt_computer_mask, iconList);

	CreateAndAppendIconL( KAknsIIDQgnPropBtPhone, avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_phone,
						  EMbmAvkonQgn_prop_bt_phone_mask, iconList);

	CreateAndAppendIconL( KAknsIIDQgnPropBtAudio, avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_audio,
						  EMbmAvkonQgn_prop_bt_audio_mask, iconList);

	CreateAndAppendIconL( KAknsIIDQgnPropBtMisc, avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_misc,
						  EMbmAvkonQgn_prop_bt_misc_mask, iconList);

	CreateAndAppendIconL( KAknsIIDQgnStatBtBlank, avkonbmpFilename, 
						  EMbmAvkonQgn_stat_bt_blank,
						  EMbmAvkonQgn_stat_bt_blank, iconList);
	
	CreateAndAppendIconL( KAknsIIDQgnPropBtKeyboard, avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_keyboard,
						  EMbmAvkonQgn_prop_bt_keyboard_mask, iconList);					  

	CreateAndAppendIconL( KAknsIIDQgnPropBtMouse, avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_mouse,
						  EMbmAvkonQgn_prop_bt_mouse_mask, iconList);
						  
	CreateAndAppendIconL( KAknsIIDQgnPropBtPrinter, avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_printer,
						  EMbmAvkonQgn_prop_bt_printer_mask, iconList);					  					  
	
	CreateAndAppendIconL( KAknsIIDQgnPropBtCarKit, avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_carkit,
						  EMbmAvkonQgn_prop_bt_carkit_mask, iconList);
	
	GetColorIconL( avkonbmpFilename, KAknsIIDQgnIndiBtPairedAdd, 
	                EMbmAvkonQgn_indi_bt_paired_add, 
	                EMbmAvkonQgn_indi_bt_paired_add_mask, iconList );
	
    
    TFileName bmpFilename;
    bmpFilename += KFileDrive;
    bmpFilename += KDC_BITMAP_DIR;
    bmpFilename += KBtnotifBmpFileName;	
	
	GetColorIconL( bmpFilename, KAknsIIDQgnIndiBtBlocked, 
	                EMbmBtnotifQgn_indi_bt_blocked, 
	                EMbmBtnotifQgn_indi_bt_blocked_mask, iconList );
	
	bmpFilename.Zero();
	bmpFilename.Append( KFileDrive );
	bmpFilename.Append( KDC_APP_BITMAP_DIR );
	bmpFilename.Append( KWLanBmpFileName );
	
    GetColorIconL( bmpFilename, KAknsIIDQgnIndiWlanSignalLowAdd, 
	                EMbmWlanpluginQgn_indi_wlan_signal_low_add, 
	                EMbmWlanpluginQgn_indi_wlan_signal_low_add_mask, iconList);
	
    GetColorIconL( bmpFilename, KAknsIIDQgnIndiWlanSignalMedAdd, 
                    EMbmWlanpluginQgn_indi_wlan_signal_med_add, 
                    EMbmWlanpluginQgn_indi_wlan_signal_med_add_mask, iconList );

    GetColorIconL( bmpFilename, KAknsIIDQgnIndiWlanSignalGoodAdd, 
                    EMbmWlanpluginQgn_indi_wlan_signal_good_add, 
                    EMbmWlanpluginQgn_indi_wlan_signal_good_add_mask, iconList );
	
	// Transfer iconlist ownership to the listbox
	//
	iDeviceListBox->ItemDrawer()->ColumnData()->SetIconArray(iconList);
	CleanupStack::Pop(); // iconList
	
    FLOG(_L("[BTNOTIF]\t CBTInqUI::CreatePopupListL() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::UpdateDeviceListL
// ----------------------------------------------------------
//
void CBTInqUI::UpdateDeviceListL( CBTDevice* aDevice, const TInt aSignalStrength )
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::UpdateDeviceListL()")); 
    
    TInt iconIndex (EDeviceIconDefault);
    TInt defNameIndex (EBTDeviceNameIndexDefault);
    
    HBufC* formatString = HBufC::NewLC( KBTAllPurposeBufferLength );

    // Check whether the device is already in registry.
    TInt index = LookupFromDevicesArray(iPairedDevicesArray, aDevice );

    if( index >= 0 )
        {
        // Update device's link key and friendly name 
        // with those found from registry.
        aDevice->UpdateL( *( iPairedDevicesArray->At( index ) ) );
        }

	for (TInt i = 0; i < KDeviceRowLayoutTableSize; i++)
	    {
	    if ( ( aDevice->DeviceClass().MajorDeviceClass() == KDeviceRowLayoutTable[i].iMajorDevClass ) &&
	        ( (aDevice->DeviceClass().MinorDeviceClass() == KDeviceRowLayoutTable[i].iMinorDevClass ) ||
	                KDeviceRowLayoutTable[i].iMinorDevClass == 0 ) )
            {
            iconIndex = KDeviceRowLayoutTable[i].iIconIndex;
            defNameIndex = KDeviceRowLayoutTable[i].iDefaultNameIndex;
            break;
            }
	    }
    
	if ( !aDevice->IsValidFriendlyName() && !aDevice->IsValidDeviceName() )
		{
        BtNotifNameUtils::SetDeviceNameL(iDefaultDeviceNamesArray->MdcaPoint(defNameIndex), *aDevice);
		}

	//Convert device name to Unocode for display
	if ( aDevice->IsValidFriendlyName() )
		{
        formatString->Des().Copy( aDevice->FriendlyName() );
		}
	else 
		{
		formatString->Des().Copy( BTDeviceNameConverter::ToUnicodeL(aDevice->DeviceName()));
		}

	TPtrC iconFormat (KDeviceIconFormatTable[iconIndex].iFormat);
	
    formatString->Des().Insert( 0, iconFormat );

    if( aSignalStrength > 0)
        {
        if( aSignalStrength <= KMediumStrength )
            {
            formatString->Des().Append( TPtrC(KDeviceIconFormatTable[EDeviceIconRssiLow].iFormat ) );
            }
        else if( aSignalStrength <= KHighStrength )
            {
            formatString->Des().Append( TPtrC(KDeviceIconFormatTable[EDeviceIconRssiMed].iFormat ) );
            }
        else
            {
            formatString->Des().Append( TPtrC(KDeviceIconFormatTable[EDeviceIconRssiGood].iFormat ) );
            }
        }
    
    // If the device is paired, add paired icon to format list
    // Paired device using JustWork file transfering mode is not shown as paired here. 
    if( index >= 0 && IsUserAwarePaired ( aDevice->AsNamelessDevice() ) )
        {
        formatString->Des().Append( TPtrC(KDeviceIconFormatTable[EDeviceIconPaired].iFormat ) );
        }
    else
    	{
    	// if device is blocked, add blocked icon to format list
    	
    	TInt indexB = LookupFromDevicesArray(iLastUsedDevicesArray, aDevice );
    	
    	if ( indexB>=0 && iLastUsedDevicesArray->At( indexB )->GlobalSecurity().Banned())
    		{
        	formatString->Des().Append( TPtrC(KDeviceIconFormatTable[EDeviceIconBlocked].iFormat ) );
    		}
    	}

    // Add device format string into device items
    //
    TInt deviceCount = iDeviceListRows->Count();
	iDeviceListRows->InsertL( deviceCount, *formatString );      
    CleanupStack::PopAndDestroy();  // formatString
	
    TInt currentItemIndex = 0;
	if(deviceCount != 0 && iDeviceListBox)
		{
	 	// Store temporarily the highlighted device and destroy list box
	    //
	    currentItemIndex = iDeviceListBox->CurrentItemIndex();		
		}

    if( iDeviceListBox )
        {
        iDeviceListBox->HandleItemAdditionL();
        
		if(deviceCount != 0 )	
			{        
			//set highligh back to user selected one.       
			iDeviceListBox->SetCurrentItemIndex(currentItemIndex); 
			}

		// Adjust top item index in order to "scroll up" the list. 
		// If current highlight device is not the top one and the listbox's visible slots are full(at least 5 devices are listed), 
		// to make the newly discovered device visible, the list needs to be scrolled up. 
		//
		TInt topIndex = iDeviceListBox->TopItemIndex();
		deviceCount = iDeviceListRows->Count();
		
		if( (currentItemIndex != topIndex  && deviceCount > 5 ) && (currentItemIndex+1 < deviceCount ) )
			{
			iDeviceListBox->SetTopItemIndex( topIndex+1 ); //scroll up
			}	     
        }
		
    FLOG(_L("[BTNOTIF]\t CBTInqUI::UpdateDeviceListL() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::LookupFromDevicesArrayL
// Check if the BT device is paired devices array
// ----------------------------------------------------------
//
TInt CBTInqUI::LookupFromDevicesArray(const CBTDeviceArray* aDeviceArray, const CBTDevice* aDevice ) const
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::LookupFromDevicesArray()"));

    __ASSERT_DEBUG( aDeviceArray, User::Panic(_L("BTNotifInqUI - aDeviceArray not created"), KErrNotFound ));

    for( TInt index = 0; index < aDeviceArray->Count(); index++ )
        {
        // Try to find device by its address.
        if ( aDeviceArray->At( index )->BDAddr() == aDevice->BDAddr() )
            {
            FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::LookupFromDevicesArrayL()idx %d"), index));
            return index; // Return index to the device at array
            }
        }
    return KErrNotFound;
    }
    
// ----------------------------------------------------
// CBTInqUI::CreateAndAppendIconL
// ----------------------------------------------------
void CBTInqUI::CreateAndAppendIconL( const TAknsItemID& aID,
									 const TDesC& aFileName,
									 const TInt aBitmapId,
									 const TInt aMaskId,
									 CAknIconArray* aIconList)
    {
	CGulIcon* icon = AknsUtils::CreateGulIconL(AknsUtils::SkinInstance(), aID, aFileName, aBitmapId, aMaskId);
    
    CleanupStack::PushL(icon);    
	aIconList->AppendL( icon );
    CleanupStack::Pop(); // icon
	}

// ---------------------------------------------------------
// CBTInqUI::AdjustDeviceArrayL
// Applend max 5 lately used non-paird devices plus all the 
// paired devices to the input parameter by the order of 
// the last used time stamp.
// ---------------------------------------------------------
//
void CBTInqUI::AdjustDeviceArrayL(CBTDeviceArray* aDeviceArray)
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::AdjustDeviceArrayL()"));

    TInt lSize = iLastUsedDevicesArray->Count(); 
    
    if (lSize <= 0)
        {
        return;
        }
    
    TInt pSize = iPairedDevicesArray->Count();   
	TInt nonPairedUsedDevicesNumber = lSize - pSize;	
    	
    // Pickup 5 latest used devices (paired devices are excluded)
    //   		    
    TInt count(0);
 
    if (nonPairedUsedDevicesNumber <= KBTNotifNonPairedUsedDevicesMaxNumber)
    	count = nonPairedUsedDevicesNumber;
    else 
    	count = KBTNotifNonPairedUsedDevicesMaxNumber;
    
	for(TInt i=0; i < count; i++ )
		{
	    TInt targetDeviceIndex = 0;
	    	   	    
	    //go through iLastUsedDevicesArray, compare each device's time stamp and, find the latest used one.
	    for(TInt k=0; k < lSize; k++)
	    	{
		    if(iLastUsedDevicesArray->At(targetDeviceIndex)->Used() < iLastUsedDevicesArray->At(k)->Used())
		    	{
		    	targetDeviceIndex = k;
		    	}
	    	}
	    	
		//if result is not a paired device, add it to the adjuested device array.	    	
	   	if( LookupFromDevicesArray(iPairedDevicesArray, iLastUsedDevicesArray->At(targetDeviceIndex)) == KErrNotFound )
	   		{
		    aDeviceArray->AppendL(iLastUsedDevicesArray->At(targetDeviceIndex)->CopyL()); 	  	   			   		
	   		}
	    //if result is a paired device, ignore this search round and step-back counter i to make sure this 
	    //search round won't be count.
		else
	   		{
	   		i--; 
	   		}
   		    
	    //set the time stamp to -1, in order to ignore this device during next search round.
	    iLastUsedDevicesArray->At(targetDeviceIndex)->SetUsed(-1);	    
		}

	// Add paired devices to the adjusted list(if paired device exist),
	// by the order of last used time.
	//
    if(pSize > 0)
    	{
	    for( TInt i = 0; i < pSize; i++ )
	        {
	    	//count size in the beginning of each loop, since the size could grow,
	    	//if new item has been appended in last loop round. 
			TInt sSize = aDeviceArray->Count();
	        
	        TInt j = 0;
	        for (; j < sSize ; j++)
	            {
	            //check if this paired device alreay on the list jump out this loop
	            if (iPairedDevicesArray->At(i)->BDAddr() == aDeviceArray->At(j)->BDAddr())
	            	{
	                break;
	                }
	            
	            // We do not add device still valid paired into list if iExcludePairedDevices is set.  
	            if ( IsUserAwarePaired( iPairedDevicesArray->At(i)->AsNamelessDevice() ) &&
	                    iExcludePairedDevices )
	                {
	                break;
	                }
	                
	            //if this device is not on the list and it is used later then current comparing
	            //device, insert the paired device to aDeviceArray at suitable position.
				TTime pTime = iPairedDevicesArray->At(i)->Used();
				TTime sTime = aDeviceArray->At(j)->Used();
				
	            if ( pTime > sTime)
	            	{
	                aDeviceArray->InsertL(j, iPairedDevicesArray->At(i)->CopyL());
	                break;
	                }
	            }	            
	            
			//if this paired device is not on the sorted list(it is older then all the devices on the list),
			//append it to the end of the list.
	        if (j == sSize) 
	            {
							if ( iExcludePairedDevices )
								 {                
			            // Add device to list if device is not paired and iExcludePairedDevices is not set. 
			            if ( !IsUserAwarePaired( iPairedDevicesArray->At(i)->AsNamelessDevice() ) )
			                {                                      
			                aDeviceArray->AppendL(iPairedDevicesArray->At(i)->CopyL());
			                }			           
	               }
	            else
	            	{
	            	aDeviceArray->AppendL(iPairedDevicesArray->At(i)->CopyL());
	            	}
	            }	            
	        }    	
    	}
 
    FLOG(_L("[BTNOTIF]\t CBTInqUI::AdjustDeviceArrayL()  Complete."));
    }

//----------------------------------------------------------
// CBTInqUI::QueryUnblockDeviceL
//----------------------------------------------------------
TInt CBTInqUI::QueryUnblockDeviceL(CBTDevice* aDevice)
	{
	FLOG(_L("[BTNOTIF]\t CBTInqUI::QueryUnblockDeviceL()"));
	RBuf stringholder;
	stringholder.CleanupClosePushL();
    TBTDeviceName name;
    BtNotifNameUtils::GetDeviceDisplayName( name, aDevice );
    BluetoothUiUtil::LoadResourceAndSubstringL( 
            stringholder, R_BT_UNBLOCK_DEVICE, name, 0 );
    TInt keypress(0);
    keypress = iUiUtil->ShowQueryL( stringholder, R_BT_UNBLOCK_QUERY, 
            ECmdBTnotifUnavailable, name, CAknQueryDialog::EConfirmationTone );
    CleanupStack::PopAndDestroy();  // stringholder
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::QueryUnblockDeviceL()  keypress= %d"),keypress));    

	return keypress;
	}

// ----------------------------------------------------------
// CBTInqUI::UnblockDevice
// Unblock device by modifying registry
//-----------------------------------------------------------
//
TInt CBTInqUI::UnblockDevice(TInt aIndex)
    {
    TInt returnValue = KErrNone;
    TBTDeviceSecurity deviceSecuritySettings = iLastUsedDevicesArray->At(aIndex)->GlobalSecurity();
    deviceSecuritySettings.SetBanned(EFalse);
    iLastUsedDevicesArray->At(aIndex)->DeleteLinkKey();
    //DeleteLinkKey() invalidates the LinkKey and IsPaired entries in Lisbon, so...       
    iLastUsedDevicesArray->At(aIndex)->SetGlobalSecurity(deviceSecuritySettings);
    returnValue = iDevMan->ModifyDevice(*iLastUsedDevicesArray->At(aIndex));
    if (!returnValue)
        {
        iBTRegistryQueryState = ESetDeviceUnblocked;
        }
    return returnValue;
    }

// ---------------------------------------------------------
// CBTInqUI::HandleDevManComplete
// CBTEngDevMan callback
// ---------------------------------------------------------
//
void CBTInqUI::HandleDevManComplete(TInt aErr)
	{
	FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleDevManComplete()  aErr= %d"),aErr));
	
	switch(iBTRegistryQueryState)
		{	
		case ESetDeviceUnblocked:
		    FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleDevManComplete() EModifyDevice")));  
		    iDevSearchObserver->NotifyDeviceSearchCompleted(aErr, iDevParams);
		    break;
		}

	FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleDevManComplete()  complete")));
	}
		
//----------------------------------------------------------------
// CBTInqUI: DeviceSearchFilter
//----------------------------------------------------------------
void CBTInqUI::SelectSearchCategoryL()
	{
	FLOG(_L("[BTNOTIF]\t CBTInqUI::SelectSearchCategoryL()"));
	
	if( !(iDesiredDeviceClass == TBTDeviceClass(0)) )
	    {
	    // Notifier caller passed specified COD to search device, not need to show filter query.
        DeviceSearchUiL();
        FLOG(_L("[BTNOTIF]\t CBTInqUI::SelectSearchCategoryL() complete: not show filter query"));
        return;
	    }
	// Create listbox for devices
    //
    CEikTextListBox* searchFilterListBox = new(ELeave) CAknSinglePopupMenuStyleListBox;
	CleanupStack::PushL(searchFilterListBox);

    // Create popup list    
    //
	iSearchFilterPopupList = CAknPopupList::NewL(
        searchFilterListBox,
        R_AVKON_SOFTKEYS_SELECT_CANCEL__SELECT,
        AknPopupLayouts::EDynMenuWindow );

    searchFilterListBox->ConstructL(iSearchFilterPopupList, EAknListBoxMenuList );
    searchFilterListBox->CreateScrollBarFrameL(ETrue);
    searchFilterListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
        CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

    // Read title from resources
    //
    HBufC* popupListTitle = iEikonEnvRef.AllocReadResourceLC( R_BT_SEARCH_FILTER_POPUP_TITLE );
    iSearchFilterPopupList->SetTitleL(*popupListTitle);
    CleanupStack::PopAndDestroy(); // popupListTitle

	// reuse iDeviceListRows for serch filter items
	iDeviceListRows->Reset();

    // Create list box model for devices
    //
    
    CTextListBoxModel* model = searchFilterListBox->Model();
    model->SetItemTextArray(iDeviceListRows);
    model->SetOwnershipType(ELbmDoesNotOwnItemArray);  // Do not destroy items
    
    
	// Add itms of filter to popuplist
    // Order must be same as in enum EBTSerchFilterItem
    //
    
    AddItemToSearchFilterL(R_BT_SEARCH_FOR_ALL); 
    AddItemToSearchFilterL(R_BT_SEARCH_FOR_AUDIO);
    AddItemToSearchFilterL(R_BT_SEARCH_FOR_PHONE);
    AddItemToSearchFilterL(R_BT_SEARCH_FOR_COMPUTER);
    AddItemToSearchFilterL(R_BT_SEARCH_FOR_INPUT);

    // Launch popup list and wait for user input, disable other Ui key presses first:
    AllowDialerAndAppKeyPress( EFalse );
    // After input, destroy popup list
    //
    TInt popupRetVal = iSearchFilterPopupList->ExecuteLD(); 
    iSearchFilterPopupList = NULL;
    AllowDialerAndAppKeyPress( ETrue );

    if( popupRetVal )
        {
        TInt currentDeviceIndex = searchFilterListBox->CurrentItemIndex();
		switch(currentDeviceIndex)
			{
			case EBTSearchForAll:				
				iDesiredDeviceClass = TBTDeviceClass(0,0,0);  				
				break;
			case EBTSearchForAudio:
				iDesiredDeviceClass = TBTDeviceClass(0,EMajorDeviceAV,0);					
				break;
				 
			case EBTSearchForPhone:	
				iDesiredDeviceClass = TBTDeviceClass(0,EMajorDevicePhone,0);					
				break;	 
				
			case EBTSearchForComputer:					
				iDesiredDeviceClass = TBTDeviceClass(0,EMajorDeviceComputer,0);
				break;
				 	 
			case EBTSearchForInput:
				iDesiredDeviceClass = TBTDeviceClass(0,EMajorDevicePeripheral,0);
				break;	 
				 			
			default:                    
				break;
			}
        } 

	CleanupStack::PopAndDestroy(); //searchFilterListBox 
	iDeviceListRows->Reset();

    if( !popupRetVal )
        {
        iDevSearchObserver->NotifyDeviceSearchCompleted(KErrCancel);
        }
    else
        {
        DeviceSearchUiL();
        }
    
	FLOG(_L("[BTNOTIF]\t CBTInqUI::SelectSearchCategoryL() complete"));
	}
   
//------------------------------------------------------------------------------
// CBTInqUI: AddItemToSearchFilterL
//------------------------------------------------------------------------------
void CBTInqUI::AddItemToSearchFilterL ( TInt aResourceId )
	{
	HBufC* formatString = HBufC::NewLC( KBTAllPurposeBufferLength );
    HBufC* string = iEikonEnvRef.AllocReadResourceLC( aResourceId );
    formatString->Des().Append( *string );
    CleanupStack::PopAndDestroy(); // string
    iDeviceListRows->AppendL( *formatString );
    CleanupStack::Pop(); // formatString
	}

void CBTInqUI::GetColorIconL( TFileName& aFilename, TAknsItemID aItemID, 
                        TInt aPic, TInt aPicmask, CAknIconArray* aIconList )
    {
    CFbsBitmap* bitmap = NULL;
    CFbsBitmap* mask = NULL;
    
    AknsUtils::CreateColorIconLC(AknsUtils::SkinInstance(), aItemID,
                                KAknsIIDQsnIconColors,
                                EAknsCIQsnIconColorsCG13,
                                bitmap, mask, aFilename, aPic, aPicmask, KRgbBlack);
    
    CGulIcon* icon = CGulIcon::NewL(bitmap, mask);
    CleanupStack::Pop(2); // bitmap, mask;
    CleanupStack::PushL(icon);    
    aIconList->AppendL( icon );
    CleanupStack::Pop(); // icon
    }

TBool CBTInqUI::IsActiveVoIPOverWLAN()
    {
    TInt error = KErrNone;
    
    // check VoIP over WLAN is on going or not:
    //
    TInt wlanStatus = 0;
    error = RProperty::Get(KPSUidWlan, KPSWlanIndicator, wlanStatus);                       
    FTRACE( FPrint( _L( "CBTInqNotifier::CheckActivationOfVoIPOverWLAN() WLAN state: %d (error %d)" ), wlanStatus, error ) );
    if (error)
        {
        return EFalse;
        }

    TInt callState = 0;
    error = RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallType, callState ); 
    FTRACE( FPrint( _L( "CBTInqNotifier::CheckActivationOfVoIPOverWLAN() call state: %d (error %d)" ), callState, error ) );
    if( error == KErrNone &&
      (wlanStatus == EPSWlanIndicatorActive || wlanStatus == EPSWlanIndicatorActiveSecure)&&
      callState == EPSCTsyCallTypeVoIP )
        {
        FTRACE( FPrint( _L( " VoIP call ongoing, do not allow inquiry!" ) ) );
        return ETrue;
        }
    return EFalse;
    }

void CBTInqUI::DoDeviceFrontListSelection()
    {
    TRAPD(err, DisplayDevicesFrontListL());
    if (err)
        {
        iDevSearchObserver->NotifyDeviceSearchCompleted(err);
        }
    }

void CBTInqUI::HandleDeviceSelectionL(CBTDevice* aDev)
    {
    iDevParams = TBTDeviceResponseParams();
    iDevParams.SetDeviceAddress( aDev->BDAddr());
    iDevParams.SetDeviceClass( aDev->DeviceClass());
    if( aDev->IsValidFriendlyName())
        {
        iDevParams.SetDeviceName( aDev->FriendlyName());
        }
    else if( aDev->IsValidDeviceName())
        {
        iDevParams.SetDeviceName( BTDeviceNameConverter::ToUnicodeL( aDev->DeviceName() ) );
        }
    //check if this device is blocked
    TInt index = LookupFromDevicesArray(iLastUsedDevicesArray, aDev);
    TBool blocked = ( index >= 0 && iLastUsedDevicesArray->At( index )->GlobalSecurity().Banned() );
    if (!blocked)
        {
        iDevSearchObserver->NotifyDeviceSearchCompleted(KErrNone, iDevParams);
        return;
        }

    TInt toUnblk = QueryUnblockDeviceL(aDev);
    if (!toUnblk)
        {
        iDevSearchObserver->NotifyDeviceSearchCompleted(KErrCancel);
        return;
        }
    TInt err = UnblockDevice( index );
    if (err)
        {
        iDevSearchObserver->NotifyDeviceSearchCompleted(err);
        }  
    // otherwise NotifyDeviceSearchCompleted will be called after unblock is really done.
    }

void CBTInqUI::AllowDialerAndAppKeyPress( TBool aAllow )
    {
    CEikAppUi* const eikAppUi = CEikonEnv::Static()->EikAppUi();
    if ( aAllow)
        {
        // Enable dialer
        static_cast<CAknAppUi*>( eikAppUi )->SetKeyEventFlags( 0x00 );     
        // Activate apps key.
        // Ignore the return error code as we can do nothing if this operation fails
        (void) static_cast<CAknNotifierAppServerAppUi*>(eikAppUi)->SuppressAppSwitching(EFalse);         
        }
    else
        {
        // Disable dialer
        static_cast<CAknAppUi*>( eikAppUi )->SetKeyEventFlags( CAknAppUiBase::EDisableSendKeyShort | 
                CAknAppUiBase::EDisableSendKeyLong);
        // Deactivate apps key
        (void) static_cast<CAknNotifierAppServerAppUi*>(eikAppUi)->SuppressAppSwitching(ETrue); 
        }
    FTRACE( FPrint( _L( "CBTInqUI::AllowDialerAndAppKeyPress : %d" ), aAllow ) );
    }

// End of File
