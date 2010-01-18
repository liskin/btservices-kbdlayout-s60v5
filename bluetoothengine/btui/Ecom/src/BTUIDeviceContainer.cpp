/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the implementation of device list container. It contains 
*				 all necessary AVKON components and handles user inputs
*				 for paired and blocked devices view.
*
*/


#include <btui.mbg>				// Compiled resource ids
#include <BtuiViewResources.rsg> 
#include <AknsUtils.h>          // For Extended skinning icons
#include <gulicon.h>
#include <AknUtils.h>			// For char filter
#include <aknlists.h> 
#include <avkon.mbg> 			// AVKON icons
#include <csxhelp/bt.hlp.hrh>   // Help IDs

#include "BTUIDeviceContainer.h"
#include "BTUIListedDevicesView.h"
#include "debug.h"               
#include "btui.h" // Global declarations

const TInt KBTNumberOfIcons = 13;    // Number of class of device icons
	
_LIT( DEV_TYPE_COMPUTER,"0\t");
_LIT( DEV_TYPE_PHONE,"1\t");
_LIT( DEV_TYPE_AUDIO,"2\t");
_LIT( DEV_TYPE_DEFAULT,"3\t");	
_LIT( DEV_TYPE_KEYBOARD,"4\t");
_LIT( DEV_TYPE_MOUSE,"5\t");
_LIT( DEV_TYPE_PRINTER,"6\t");
_LIT( DEV_TYPE_CARKIT,"7\t");
_LIT( DEV_TYPE_AUDIO_CONNECT,"8\t");
_LIT( DEV_TYPE_COMPUTER_CONNECT,"9\t");
_LIT( DEV_TYPE_KEYBOARD_CONNECT,"10\t");
_LIT( DEV_TYPE_CARKIT_CONNECT,"11\t");
_LIT( DEV_TYPE_MOUSE_CONNECT,"12\t");

_LIT(DEV_STATUS_NORMAL, "\t" ); 		// blocked in blocked devices view, paired in pairred devices view
_LIT(DEV_STATUS_AUTHORIZED, "\t13" ); 	// autorized in paired devices view

    

// ================= MEMBER FUNCTIONS =======================
  
// ---------------------------------------------------------
// CBTUiDeviceContainer::ConstructL
// ---------------------------------------------------------
void CBTUiDeviceContainer::ConstructL(const TRect& aRect, 
  CBTUIListedDevicesView* aView, TBTDeviceGroup aGroup)
    {
	TRACE_FUNC_ENTRY
	
    CreateWindowL();
    
	iView = aView;
    iGroup = aGroup;
    // Create new list for other paired bt-devices
    //
    iDeviceList = new (ELeave) CAknSingleGraphicStyleListBox;
    iDeviceList->SetContainerWindowL(*this);
    iDeviceList->ConstructL(this, EAknListBoxSelectionList);
    
	iDeviceList->SetListBoxObserver( this );    

    // Set the empty text at basis of the list
    HBufC* devEmptyText = NULL ;
    if(iGroup ==EGroupPaired)
    	{
    	devEmptyText=iCoeEnv->AllocReadResourceLC( R_BT_NO_PAIRED_DEVICES);
    	}    	
    else
    	{    
    	devEmptyText=iCoeEnv->AllocReadResourceLC( R_BT_NO_BLOCKED_DEVICES);
    	}
    iDeviceList->View()->SetListEmptyTextL(*devEmptyText );
    CleanupStack::PopAndDestroy(devEmptyText);

    // Set up filenames for Btui and Avkon bitmaps
    TFileName bmpFilename;
    bmpFilename += KBitmapFileDrive;
    bmpFilename += KDC_APP_BITMAP_DIR;
    bmpFilename += KBtuiBmpFileName;
        
    // The avkon bitmap we will get from AknIconUtils::AvkonIconFileName()
    TFileName avkonbmpFilename = AknIconUtils::AvkonIconFileName();
    
    // Setup graphic items list for BT devices listbox
    CAknIconArray* iconList = new(ELeave) CAknIconArray(KBTNumberOfIcons);
    CleanupStack::PushL( iconList );
    

	CreateAndAppendIconL( KAknsIIDQgnPropBtComputer, 
						  avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_computer,
						  EMbmAvkonQgn_prop_bt_computer_mask,
						  iconList);

	CreateAndAppendIconL( KAknsIIDQgnPropBtPhone,
						  avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_phone,
						  EMbmAvkonQgn_prop_bt_phone_mask,
						  iconList);

	CreateAndAppendIconL( KAknsIIDQgnPropBtAudio,
						  avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_audio,
						  EMbmAvkonQgn_prop_bt_audio_mask,
						  iconList);

	CreateAndAppendIconL( KAknsIIDQgnPropBtMisc,
						  avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_misc,
						  EMbmAvkonQgn_prop_bt_misc_mask,
						  iconList);
						  
	CreateAndAppendIconL( KAknsIIDQgnPropBtKeyboard,
						  avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_keyboard,
						  EMbmAvkonQgn_prop_bt_keyboard_mask,
						  iconList);					  

	CreateAndAppendIconL( KAknsIIDQgnPropBtMouse,
						  avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_mouse,
						  EMbmAvkonQgn_prop_bt_mouse_mask,
						  iconList);
						  
	CreateAndAppendIconL( KAknsIIDQgnPropBtPrinter,
						  avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_printer,
						  EMbmAvkonQgn_prop_bt_printer_mask,
						  iconList);					  					  
	
	CreateAndAppendIconL( KAknsIIDQgnPropBtCarKit,
						  avkonbmpFilename, 
						  EMbmAvkonQgn_prop_bt_carkit,
						  EMbmAvkonQgn_prop_bt_carkit_mask,
						  iconList);
						  
	CreateAndAppendIconL( KAknsIIDQgnPropBtAudioConnect,
						  bmpFilename, 
						  EMbmBtuiQgn_prop_bt_audio_connect,
						  EMbmBtuiQgn_prop_bt_audio_connect_mask,
						  iconList);					  
						  
	CreateAndAppendIconL( KAknsIIDQgnPropBtComputerConnect,
						  bmpFilename, 
						  EMbmBtuiQgn_prop_bt_computer_connect,
						  EMbmBtuiQgn_prop_bt_computer_connect_mask,
						  iconList);
						  
    CreateAndAppendIconL( KAknsIIDQgnPropBtKeyboardConnect,
						  bmpFilename, 
						  EMbmBtuiQgn_prop_bt_keyboard_connect,
						  EMbmBtuiQgn_prop_bt_keyboard_connect_mask,
						  iconList);						  		
						  
	CreateAndAppendIconL( KAknsIIDQgnPropBtCarKitConnect,
						  bmpFilename, 
						  EMbmBtuiQgn_prop_bt_car_kit_connect,
						  EMbmBtuiQgn_prop_bt_car_kit_connect_mask,
						  iconList);	
	
	CreateAndAppendIconL( KAknsIIDQgnPropBtMouseConnect,
                          bmpFilename, 
                          EMbmBtuiQgn_prop_bt_mouse_connect,
                          EMbmBtuiQgn_prop_bt_mouse_connect_mask,
                          iconList);
	
	CFbsBitmap* bitmap = NULL;
    CFbsBitmap* mask = NULL;
    AknsUtils::CreateColorIconLC(AknsUtils::SkinInstance(), 
                                 KAknsIIDQgnIndiBtTrustedAdd,
                                 KAknsIIDQsnIconColors,
                                 EAknsCIQsnIconColorsCG13,
                                 bitmap, mask,
                                 avkonbmpFilename,
                                 EMbmAvkonQgn_indi_bt_trusted_add,
                                 EMbmAvkonQgn_indi_bt_trusted_add_mask,
                                 KRgbBlack);
    CGulIcon* icon = CGulIcon::NewL(bitmap, mask);
        
    CleanupStack::Pop(mask);
    CleanupStack::Pop(bitmap); 
    
    CleanupStack::PushL(icon);    
	iconList->AppendL( icon );
    CleanupStack::Pop(icon);
    
    	
    // Transfer iconlist ownership to the listbox
    iDeviceList->ItemDrawer()->ColumnData()->SetIconArray(iconList);
    CleanupStack::Pop(iconList);

    // Create scrollbars
    iDeviceList->CreateScrollBarFrameL(ETrue);
    iDeviceList->ScrollBarFrame()->SetScrollBarVisibilityL(
    	CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto);

    SetRect(aRect);
    ActivateL();

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CBTUiDeviceContainer::NewL
// ---------------------------------------------------------
CBTUiDeviceContainer* CBTUiDeviceContainer::NewL(const TRect& aRect, 
  CBTUIListedDevicesView* aView, TBTDeviceGroup aGroup, MObjectProvider* aParent)
    {
	 CBTUiDeviceContainer* self = new (ELeave) CBTUiDeviceContainer();
	 self->SetMopParent(aParent);
	 CleanupStack::PushL(self);
	 self->ConstructL(aRect, aView, aGroup);
	 CleanupStack::Pop(self);
	 return self;
    }
// ---------------------------------------------------------
// destructor
// ---------------------------------------------------------

CBTUiDeviceContainer::~CBTUiDeviceContainer()
    {
	TRACE_FUNC_ENTRY
    delete iDeviceList;
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CBTUiDeviceContainer::RefreshDeviceListL
// This function will refresh only the changed parts of the
// screen, if any.
// ---------------------------------------------------------   
void CBTUiDeviceContainer::RefreshDeviceListL(const RDeviceArray* aDeviceArray,TInt selectedItem)
    {
	TRACE_FUNC_ENTRY
	
    MDesCArray* itemTextArray = iDeviceList->Model()->ItemTextArray();
    CDesCArray* itemArray = (CDesCArray*) itemTextArray;
    TBool removal=EFalse;
    TBool addition=EFalse;
	TInt previouslySelected=CurrentItemIndex();
    TBTDeviceName listItem;
    TInt deviceCount = aDeviceArray->Count();
    TInt i = 0;
    for( ; i < deviceCount; i++)
        {
        TBTDevice device = *(*aDeviceArray)[i];
        CreateListItemL(device, listItem);
        if(itemArray->Count()<=i)
        	{
        	itemArray->AppendL(listItem);
        	addition=ETrue;
        	}
        else
        	if(listItem != (*itemArray)[i] )
				{
				itemArray->Delete(i);
				itemArray->InsertL(i,listItem);
				
				removal=ETrue;
				addition=ETrue;
				}
        
        }
	if(itemArray->Count()-i>=0)
		{
		itemArray->Delete(i,itemArray->Count()-i);        
		removal=ETrue;
		}
		
        
	if(removal)        
		iDeviceList->HandleItemRemovalL();
	if(addition)
    	iDeviceList->HandleItemAdditionL();

	if(deviceCount >0 )
		iDeviceList->SetCurrentItemIndex( Max(selectedItem,0 )) ;
	
	if(addition | removal | previouslySelected!=selectedItem)
    	iDeviceList->DrawDeferred();
	TRACE_FUNC_EXIT
    } 

// ---------------------------------------------------------
// CBTUiDeviceContainer::CountItems
// ---------------------------------------------------------    
TInt CBTUiDeviceContainer::CountItems() 
	{   
	return iDeviceList->Model()->ItemTextArray()->MdcaCount();    
	}

// ---------------------------------------------------------
// CBTUiDeviceContainer::CreateListItemL
// ---------------------------------------------------------
void CBTUiDeviceContainer::CreateListItemL(const TBTDevice& aDevice, TBTDeviceName& aListItem)
    {
	TRACE_FUNC_ENTRY
    aListItem.Zero(); // may not needed
    
    TInt iconIndex = (TInt)(aDevice.iType);
    
    switch( iconIndex)
    	{
    	case EDeviceComputer:
    		if(( aDevice.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected)
    			{
    			aListItem.Append(DEV_TYPE_COMPUTER_CONNECT);   	
    	    	}
			else
				{
				aListItem.Append(DEV_TYPE_COMPUTER); 
				}
			break;
    	
    	case EDevicePhone:   	
    		aListItem.Append(DEV_TYPE_PHONE); 
    		break;
    		
    	case EDeviceAudio: 
    		if(( aDevice.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected)
    			{
    			aListItem.Append(DEV_TYPE_AUDIO_CONNECT);   	
    	    	}
			else
				{
    			aListItem.Append(DEV_TYPE_AUDIO);
				}
			break ;
    	case EDeviceCarkit: 	  
    	  	if(( aDevice.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected)
    			{
    			aListItem.Append(DEV_TYPE_CARKIT_CONNECT);   	
    	    	}
			else
				{
    			aListItem.Append(DEV_TYPE_CARKIT);
				}
			break ;
				
		case EDevicePrinter:
			aListItem.Append(DEV_TYPE_PRINTER);
			break;
			
		case EDeviceKeyboard:
		
			if(( aDevice.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected)
    			{
    			aListItem.Append(DEV_TYPE_KEYBOARD_CONNECT);   	
    	    	}
			else
				{
    			aListItem.Append(DEV_TYPE_KEYBOARD);
				}
			break ;
				
		case EDeviceMice:
		    
		    if(( aDevice.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected)
                {
                aListItem.Append(DEV_TYPE_MOUSE_CONNECT);    
                }
            else
                {
                aListItem.Append(DEV_TYPE_MOUSE);
                }
			break;			
			
		case EDeviceDefault:	
    		aListItem.Append(DEV_TYPE_DEFAULT); 
    		break;
    			
		default:
			aListItem.Append('\t');
			break;
    	}
    	    
    aListItem.Append(aDevice.iName);

    if((aDevice.iStatus & EStatusTrusted) == EStatusTrusted && iGroup ==EGroupPaired )                            
        {   	
    	aListItem.Append(DEV_STATUS_AUTHORIZED);    
        }    	
    else 
    	{	
  		aListItem.Append( DEV_STATUS_NORMAL );
    	}
	TRACE_FUNC_EXIT    	
    }


// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::OfferKeyEventL
// ---------------------------------------------------------
TKeyResponse CBTUiDeviceContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                                               TEventCode aType)
    {
	TRACE_FUNC_ENTRY    
    // Inform model where we are now
    TInt bItemIndex = CurrentItemIndex();

    // Shortcut for deleting a device
    if ((aKeyEvent.iCode == EKeyBackspace) && (bItemIndex >= 0))
        {
        iView->HandleCommandL( EBTUICmdDelete );
        return EKeyWasConsumed;
        }

    // Show popup commands window if selection or enter key is pressed.
	// It should not go to here, unless | EAknEnableMSK is not set
    if ((aKeyEvent.iCode == EKeyDevice3) || (aKeyEvent.iCode == EKeyEnter))
        {
        iView->ProcessCommandL(EAknSoftkeyContextOptions);        
        return EKeyWasConsumed;
        }

    if ((aKeyEvent.iCode != EKeyLeftArrow) && 
        (aKeyEvent.iCode != EKeyRightArrow))
        {
		TRACE_FUNC_EXIT        
        return iDeviceList->OfferKeyEventL(aKeyEvent, aType);
        }
    else
        {
        // BTRegistry operations are not done in atomic way
        // so we cannot allow them to be interrupted
        if(iView->DeviceChangeInProgress())
        	return EKeyWasConsumed;
        
		TRACE_FUNC_EXIT        
        return EKeyWasNotConsumed;
        }
    }

// ----------------------------------------------------
// CBTUiDeviceContainer::CreateAndAppendIconL
// ----------------------------------------------------
void CBTUiDeviceContainer::CreateAndAppendIconL( 
  const TAknsItemID& aID,
  const TDesC& aFileName,
  const TInt aBitmapId,
  const TInt aMaskId,
  CAknIconArray* aIconList)
    {    
	TRACE_FUNC_ENTRY    
	
    CGulIcon* icon = AknsUtils::CreateGulIconL(AknsUtils::SkinInstance(),
     aID, aFileName, aBitmapId, aMaskId);
    
    CleanupStack::PushL(icon);    
	aIconList->AppendL(icon);
    CleanupStack::Pop(icon);
	}

// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::GetHelpContext
// ---------------------------------------------------------
void CBTUiDeviceContainer::GetHelpContext(TCoeHelpContext& aContext) const
    {
	TRACE_FUNC_ENTRY
#ifdef __SERIES60_HELP

    aContext.iMajor = KBtuiUid3;
    if( iGroup ==EGroupPaired)
    	aContext.iContext = KBT_HLP_PAIRED;
    else
    	aContext.iContext = KBT_HLP_BLOCKED ;
#else    
#endif
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::SizeChanged
// Called by framework when the view size is changed
// ---------------------------------------------------------
void CBTUiDeviceContainer::SizeChanged()
    {
	TRACE_FUNC_ENTRY    
    iDeviceList->SetRect(Rect());  // Mandatory, otherwise not drawn
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::CountComponentControls
// ---------------------------------------------------------
TInt CBTUiDeviceContainer::CountComponentControls() const
    {
	TRACE_FUNC_ENTRY    
    // return number of controls inside this container
    return 1;
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::ComponentControl
// ---------------------------------------------------------
CCoeControl* CBTUiDeviceContainer::ComponentControl(TInt aIndex) const
    {
	TRACE_FUNC_ENTRY    
	if(aIndex == 0)
		{
		return iDeviceList ;
		}    	
	else
		{
		return NULL;	
		}		
    }

// ----------------------------------------------------
// CBTUiDeviceContainer::CurrentItemIndex
// ----------------------------------------------------
TInt CBTUiDeviceContainer::CurrentItemIndex()
    {
	TRACE_FUNC_ENTRY    
	
    return iDeviceList->CurrentItemIndex();
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::Draw
// ---------------------------------------------------------
void CBTUiDeviceContainer::Draw( const TRect& /* aRect */ ) const
    {
    }

// ---------------------------------------------------------
// From MCoeControlObserver
// CBTUiDeviceContainer::HandleControlEventL
// ---------------------------------------------------------
void CBTUiDeviceContainer::HandleControlEventL( CCoeControl* /* aControl */, TCoeEvent /* aEventType */ )
    {
	TRACE_FUNC_ENTRY    
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::HandleResourceChange
// ---------------------------------------------------------
void CBTUiDeviceContainer::HandleResourceChange( TInt aType )
    {
	TRACE_FUNC_ENTRY    
    CCoeControl::HandleResourceChange(aType);
    TRAP_IGNORE(HandleResourceChangeL(aType));
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// Internal Leave function
// CBTUiDeviceContainer::HandleResourceChangeL
// ---------------------------------------------------------
void CBTUiDeviceContainer::HandleResourceChangeL( TInt aType )
    {
	TRACE_FUNC_ENTRY    

    //Handle change in layout orientation
    if (aType == KEikDynamicLayoutVariantSwitch || aType == KAknsMessageSkinChange )
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        
        // Because the trailing icons are not changed properly after theme is changed.
        // we refresh those icons in the iconlist from iDeviceList manually.
        
        // Get iconlist from listbox
        CAknIconArray* iconList = static_cast<CAknIconArray*>(iDeviceList->ItemDrawer()->ColumnData()->IconArray());
        
        CleanupStack::PushL( iconList );
        // remove the last two icons which will not be updated properly by theme changing.
		iconList->Delete(KBTNumberOfIcons-1);
	        
	    // The avkon bitmap we will get from AknIconUtils::AvkonIconFileName()
	    TFileName avkonbmpFilename = AknIconUtils::AvkonIconFileName();
	    
	  
	    
		CFbsBitmap* bitmap = NULL;
	    CFbsBitmap* mask = NULL;
	    AknsUtils::CreateColorIconLC(AknsUtils::SkinInstance(), 
	                                 KAknsIIDQgnIndiBtTrustedAdd,
	                                 KAknsIIDQsnIconColors,
	                                 EAknsCIQsnIconColorsCG13,
	                                 bitmap, mask,
	                                 avkonbmpFilename,
	                                 EMbmAvkonQgn_indi_bt_trusted_add,
	                                 EMbmAvkonQgn_indi_bt_trusted_add_mask,
	                                 KRgbBlack);
	    
	    
	    CGulIcon* icon = CGulIcon::NewL(bitmap, mask);
	     
	    CleanupStack::Pop(2); // bitmap, mask;
	    
	    CleanupStack::PushL(icon);    
		
		iconList->AppendL( icon );
	    CleanupStack::Pop(icon);
	    
	    	
	    // Transfer iconlist ownership back to the listbox
	    iDeviceList->ItemDrawer()->ColumnData()->SetIconArray(iconList);
	    CleanupStack::Pop(iconList);
		
        SetRect(mainPaneRect);
		DrawDeferred();
		}
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::FocusChanged
// ---------------------------------------------------------
void CBTUiDeviceContainer::FocusChanged(TDrawNow /*aDrawNow*/)
    {
	TRACE_FUNC_ENTRY    
	iDeviceList->SetFocus(IsFocused());
	TRACE_FUNC_EXIT        
    }
// ---------------------------------------------------------
// From CCoeControl
// CBTUiDeviceContainer::HandleListBoxEventL
//
// Handle touch screen singleclick as MSK devent to paired view or 
// as delete key event to blocked devices view.
// ---------------------------------------------------------
//
void CBTUiDeviceContainer::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
    {
    TRACE_FUNC_ENTRY
	// this is no longer called, unless MSK is disabled.
	// if msk is not disabled, then  
	// BTMainView::HandleCommand(EBTUICmdMSK) is called instead
    if( aEventType == EEventEnterKeyPressed 
    || aEventType == EEventItemSingleClicked) // Selection key pressed, or item clicked
        {        
    	// msk open context options in paired devices view.
    	// it will unblock the selected blocked device in blocked devices view.
    	if(iGroup==EGroupPaired) 
    		{
    		iView->ProcessCommandL( EAknSoftkeyContextOptions);
    		}				
		else
			{
			iView->ProcessCommandL( EBTUICmdUnblock);				
			}
			 
        }
	TRACE_FUNC_EXIT
    }
