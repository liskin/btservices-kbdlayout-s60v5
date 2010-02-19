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
* Description:   This is view that handles listing and 
*  unblocking blocked devices view.
*
*/

#include <avkon.hrh>                    // AVKON components
#include <akntitle.h>
#include <aknViewAppUi.h>
#include <BtuiViewResources.rsg>                     // Compiled resource ids
#include <featmgr.h>                    // Feature Manager
#include <AknGlobalNote.h>
#include <aknmessagequerydialog.h> 
#include <aknnotewrappers.h>
#include <StringLoader.h>   // Series 60 localisation stringloader
#include <aknnavi.h>
#include <hlplch.h>         // Help launcher
#include <SecondaryDisplay/BtuiSecondaryDisplayAPI.h>

#include "BTUIBlockedDevicesView.h"
#include "BTUIDeviceContainer.h"
#include "debug.h"
#include "btui.h"

#include <gstabhelper.h>
#include <btui.mbg>						// Tab icon for General Setting
#include <bluetoothuiutil.h>
#include "BTUIMainView.h" 				// base View.

// ---------------------------------------------------------
// CBTUIBlockedDevicesView::ConstructL
// Symbian 2nd phase constructor can leave.
// ---------------------------------------------------------
//
void CBTUIBlockedDevicesView::ConstructL()
    {
	TRACE_FUNC_ENTRY

    BaseConstructL(R_BTUI_BLOCK_DEVICES_VIEW);    

	TRACE_FUNC_EXIT
    }
    
// ---------------------------------------------------------
// CBTUIBlockedDevicesView::ConstructL
// Symbian 2nd phase constructor when served as GS plugin.
// ---------------------------------------------------------
//  
void CBTUIBlockedDevicesView::ConstructL( CBTUIMainView* aBaseView, CArrayPtrFlat<MGSTabbedView>* aTabViewArray )
    {
	iBaseView = aBaseView,
    iTabHelper = CGSTabHelper::NewL();
    aTabViewArray->AppendL(this); 
    
    BaseConstructL(R_BTUI_BLOCK_DEVICES_VIEW_GS);  
    	
    TRACE_INFO(_L("CBtuiGSPlugin* aBaseView, CArrayPtrFlat<MGSTabbedView>* aTabViewArray"))
    }
        
// ---------------------------------------------------------
// CBTUIBlockedDevicesView::NewL
// Two phased constructor
// ---------------------------------------------------------
//
CBTUIBlockedDevicesView* CBTUIBlockedDevicesView::NewL(MBtuiPluginViewActivationObserver* aObserver)
    {
	//TRACE_FUNC_ENTRY
    CBTUIBlockedDevicesView* self = new (ELeave) CBTUIBlockedDevicesView(aObserver);
	CleanupStack::PushL(self);
	self->iConstructAsGsPlugin = EFalse;	
 	self->ConstructL();	
	CleanupStack::Pop(self);

    return self;
    }
    
// ---------------------------------------------------------
// CBTUIBlockedDevicesView::NewL
// Two phased constructor
// ---------------------------------------------------------
// 
CBTUIBlockedDevicesView* CBTUIBlockedDevicesView::NewLC(CBTUIMainView* aBaseView, CArrayPtrFlat<MGSTabbedView>* aTabViewArray)
    {
    CBTUIBlockedDevicesView* self = new (ELeave) CBTUIBlockedDevicesView();    
	CleanupStack::PushL( self );
		
	self->iConstructAsGsPlugin = ETrue;
 	self->ConstructL( aBaseView, aTabViewArray );

    return self;
    }

// ---------------------------------------------------------
// CBTUIBlockedDevicesView::CBTUIBlockedDevicesView
// ---------------------------------------------------------
    
CBTUIBlockedDevicesView::CBTUIBlockedDevicesView(MBtuiPluginViewActivationObserver* aObserver )
	:iActivationObserver(aObserver)
	{
	}        
        
// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CBTUIBlockedDevicesView::~CBTUIBlockedDevicesView()
    {
	TRACE_FUNC_ENTRY   
    if (iContainer)
        {
        AppUi()->RemoveFromViewStack(*this, iContainer);
        }

    delete iContainer;   
    delete iModel;
    
	if(iConstructAsGsPlugin)
	    	delete iTabHelper;	
	
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// From CAknView
// CBTUIBlockedDevicesView::Id
// ---------------------------------------------------------
//
TUid CBTUIBlockedDevicesView::Id() const
    {
	TRACE_FUNC_ENTRY
    return KBtuiPluginUidBlockedDevicesView;//KBTBlockDevicesViewId;
    }

// ---------------------------------------------------------
// From CAknView
// CBTUIBlockedDevicesView::HandleCommandL 
// ---------------------------------------------------------
//
void CBTUIBlockedDevicesView::HandleCommandL(TInt aCommand)
    {
    TRACE_FUNC_ENTRY
	TInt bItemIndex;
    switch (aCommand)
        {
        case EAknSoftkeyBack:
            // do not exit if devicechange is in progress.
            if(iModel->DeviceChangeInProgress() )
            	return;                
	        AppUi()->ActivateLocalViewL(iBaseView->GetParentView().iViewUid );
            break;

        case EAknSoftkeyExit:
            // do not exit if devicechange is in progress.
            if(iModel->DeviceChangeInProgress() )
            	return;        
			AppUi()->ProcessCommandL(EEikCmdExit);        
			break;
		
		case  EBTUICmdDelete:
        case  EBTUICmdUnblock:
        	{
        	
        	bItemIndex = iContainer->CurrentItemIndex(); 
			if(bItemIndex < 0) 
        		break;
        	        	
        	TBTDevice device;
            device.iIndex = bItemIndex;
        	iModel->GetDevice(device);
        	
        	RBuf s;
        	CleanupClosePushL( s );
        	BluetoothUiUtil::LoadResourceAndSubstringL(
        	        s, R_BT_WARN_ERASE_BLOCKED, device.iName, 0);
        	
        	CAknQueryDialog* dialog = CAknQueryDialog::NewL(CAknQueryDialog::EConfirmationTone);
			TInt keypress = dialog->ExecuteLD(R_BT_GENERIC_QUERY, s);

			CleanupStack::PopAndDestroy(&s);

			if(keypress)  // User has accepted the dialog
			    {
        		device.iOperation = EOpUnblock;
               	iModel->ChangeDevice(device);
			    }        		
        	
        	break; 
        	
        	}
        case EBTUICmdDeleteAll:        	
        case EBTUICmdUnblockAll:
            {
        	CAknQueryDialog* dialog = CAknQueryDialog::NewL(CAknQueryDialog::EConfirmationTone);
        	HBufC* s=iCoeEnv->AllocReadResourceLC(R_BT_WARN_ERASE_ALL_BLOCKED);
        	TInt keypress=dialog->ExecuteLD(R_BT_GENERIC_QUERY, *s);
        	CleanupStack::PopAndDestroy(s);
        	
        	if(keypress)
            	{
        		iModel->ChangeAllDevices(EOpUnblock);
            	}
        	
        	break; 
            }

        case EAknCmdHelp:
            {            
            #ifdef __SERIES60_HELP
            HlpLauncher::LaunchHelpApplicationL(iEikonEnv->WsSession(), AppUi()->AppHelpContextL());
            #endif
            break;
            }          
                	
        default:
            {
            AppUi()->HandleCommandL(aCommand);
            break;
            }
        }
	TRACE_FUNC_EXIT        
    }


// ---------------------------------------------------------
// From CAknView
// CBTUIBlockedDevicesView::DoActivateL
// ---------------------------------------------------------
//
void CBTUIBlockedDevicesView::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
                                               TUid /*aCustomMessageId*/,
                                         const TDesC8& /*aCustomMessage*/)
    {
	TRACE_FUNC_ENTRY
    // Setup title
    //
    iModel = CBTDevModel::NewL(EGroupBlocked, this);     
    iContainer = CBTUiDeviceContainer::NewL(ClientRect(), this, EGroupBlocked, this);

    HBufC* btBlockDevices = iCoeEnv->AllocReadResourceLC(R_BT_TITLE_BLOCKED_DEV);
    CEikStatusPane* statusPane = StatusPane();
    CAknTitlePane* titlePane;
    titlePane = (CAknTitlePane*) statusPane->ControlL(TUid::Uid(EEikStatusPaneUidTitle));
    titlePane->SetTextL(*btBlockDevices);
    CleanupStack::PopAndDestroy(btBlockDevices); 
    AppUi()->AddToStackL(*this, iContainer);
	DimMskL(ETrue ); 
	// the MSK will be undimmed, if there are blocked devices when
	// the deviceList will be refreshed

    if( iBaseView && iConstructAsGsPlugin)
        {
        iTabHelper->CreateTabGroupL( Id(), 
            static_cast<CArrayPtrFlat<MGSTabbedView>*>
            (iBaseView->TabbedViews()) ); 
        }
        
	// notify observer   
    if( iActivationObserver  && iConstructAsGsPlugin == EFalse)
    {    	    
        iActivationObserver->PluginViewActivated(EBlockedDevicesViewId);        
    }
        
	TRACE_FUNC_EXIT
	}

// ---------------------------------------------------------
// From CAknView
// CBTUIBlockedDevicesView::DoDeactivate
// ---------------------------------------------------------
//
void CBTUIBlockedDevicesView::DoDeactivate()
    {
	TRACE_FUNC_ENTRY
    if (iContainer)
        {
        AppUi()->RemoveFromStack(iContainer);
        delete iContainer;
        iContainer = NULL;
        }

    delete iModel;
    iModel=NULL;
    if(iConstructAsGsPlugin)
	    iTabHelper->RemoveTabGroup();  
 
	TRACE_FUNC_EXIT
    }
// ---------------------------------------------------------
// CBTUIBlockedDevicesView::DimMskL
// This will dim/Undim MSK.
// However the command is sent when the key is pressed 
// regardless wether the MSK command is visible and 
// the view must be able to receive it anyway.
// ---------------------------------------------------------
void CBTUIBlockedDevicesView::DimMskL(TBool aDimmed)
    {
	TRACE_FUNC_ENTRY			
	if(aDimmed)
		{
		Cba()->SetCommandL (EBTUICmdUnblock,KNullDesC);
		}
	else
		{
		if(iConstructAsGsPlugin)
			{
			Cba()->SetCommandSetL(R_BTUI_SOFTKEYS_OPTIONS_BACK__UNBLOCK); 					
			}
		else
			{
			Cba()->SetCommandSetL(R_BTUI_SOFTKEYS_OPTIONS_EXIT__UNBLOCK); 								
			}
		}
		
	Cba()->DrawDeferred();
	
	// NOTE: these will only hide the MSK label.
	// The command is still being sent if button is pressed.
	// HandleCommanL will take care
	// of the situation that there may be no devices to unblock
	TRACE_FUNC_EXIT	
    }	

// ---------------------------------------------------------
// From CAknView
// CBTUIBlockedDevicesView::DynInitMenuPaneL
// Shows selected item's context sensitive options menu.
// ---------------------------------------------------------
//
void CBTUIBlockedDevicesView::DynInitMenuPaneL(TInt aResId, CEikMenuPane* aMenu)
    {
	TRACE_FUNC_ENTRY    
	// dim help if it exists and is not supported	
	TInt tmp;	
	if (  aMenu->MenuItemExists(EAknCmdHelp,tmp ) )			
		{			
		aMenu->SetItemDimmed(EAknCmdHelp,
		  !FeatureManager::FeatureSupported( KFeatureIdHelp ) );
		}
    
    // do not show "unblock" and "unblock all" commands 
    // if there is nothing to unblock
	if (aResId == R_BTUI_BLOCK_DEVICES_VIEW_MENU)
	    {
        if( iContainer->CountItems() == 0) 
            {        
            aMenu->SetItemDimmed(EBTUICmdUnblock, ETrue);
            aMenu->SetItemDimmed(EBTUICmdUnblockAll, ETrue);            
            }
        else
            {
            aMenu->SetItemDimmed(EBTUICmdUnblockAll, EFalse);
            aMenu->SetItemDimmed(EBTUICmdUnblock, ETrue);
            }
	    }
	TRACE_FUNC_EXIT        
    }

// ---------------------------------------------------------
// From MBTDeviceObserver
// CBTUIBlockedDevicesView::NotifyChangeDeviceComplete
// ---------------------------------------------------------
//
void CBTUIBlockedDevicesView::NotifyChangeDeviceComplete(const TInt aErr, 
		const TBTDevice&, const RBTDevNameArray* )
	{
	TRACE_FUNC_ENTRY	
	// we do not show any note, when blocked devices are unblocked,
	// and that is the only command available.

	if(aErr!= KErrNone)
		{
		TRAP_IGNORE(TBTUIViewsCommonUtils::ShowGeneralErrorNoteL(););
		}
	TRACE_FUNC_EXIT		
	}


// -----------------------------------------------
// From MBTDeviceObserver
// CBTUIBlockedDevicesView::RefreshDeviceList
// -----------------------------------------------
void CBTUIBlockedDevicesView::RefreshDeviceList(const RDeviceArray* aDevices,
	  TInt aSelectedItemIndex)
	{		
	TRACE_FUNC_ENTRY	
	
	TRAPD(err,	
		if(iContainer)
			{
			iContainer->RefreshDeviceListL(aDevices,aSelectedItemIndex );			
			}
		// dim the MSK if there are no devices to unblock
		DimMskL(aDevices->Count()==0);
			
	);
	if(err!= KErrNone)	    
        {
        TRAP_IGNORE( TBTUIViewsCommonUtils::ShowGeneralErrorNoteL(); );
        }
		
	TRACE_FUNC_EXIT		
	}

// ---------------------------------------------------------------------------
// CBTUIBlockedDevicesView::CreateTabIconL()
// Creates icon for tab.
// ---------------------------------------------------------------------------
//    
CGulIcon* CBTUIBlockedDevicesView::CreateTabIconL()
    {
    TFileName bmpFilename; 
    bmpFilename += KBitmapFileDrive;
    bmpFilename += KDC_APP_BITMAP_DIR;
    bmpFilename += KBtuiBmpFileName;
    
    CGulIcon* icon = AknsUtils::CreateGulIconL(
        AknsUtils::SkinInstance(), 
        KAknsIIDQgnPropBtBlockedTab3,  
        bmpFilename, 
        EMbmBtuiQgn_prop_bt_blocked_tab3, 
        EMbmBtuiQgn_prop_bt_blocked_tab3_mask );
          
    return icon;
    }   
    
