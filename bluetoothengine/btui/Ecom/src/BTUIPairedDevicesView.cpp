/*
* Copyright (c) 2002,2007 Nokia Corporation and/or its subsidiary(-ies).
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
#include <BtuiViewResources.rsg>        // Compiled resource ids
#include <featmgr.h>                    // Feature Manager

#include <AknGlobalNote.h>
#include <aknnotewrappers.h>
#include <StringLoader.h>   			// Series 60 localisation stringloader
#include <AknMediatorFacade.h>
#include <aknmessagequerydialog.h>
#include <aknnotewrappers.h>
#include <aknnavi.h>
#include <hlplch.h>         			// Help launcher
#include <btengdiscovery.h> 			// for device search

#include <ctsydomainpskeys.h>
#include <bt_subscribe.h>
#include <btengsettings.h>
#include <centralrepository.h>
#include <CoreApplicationUIsSDKCRKeys.h>
#include <bamdesca.h>

#include "debug.h"             
#include "btui.h" 						// Global declarations
#include "BTUIPairedDevicesView.h"
#include "BTUIDeviceContainer.h"
#include <SecondaryDisplay/BtuiSecondaryDisplayAPI.h>
#include <csxhelp/bt.hlp.hrh>   // Help IDs

#include <gstabhelper.h>
#include <btui.mbg>						// Tab icon for General Setting
#include <bluetoothuiutil.h>
#include "BTUIMainView.h" 				// base View.


const TInt KAutoConnect2ndTry = 1;
const TUint32 SetExcludePairedDeviceMask = 0x800000;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBTUIPairedDevicesView::NewL
// Two phased constructor called by BT AppUI when serve as part of btui.exe
// ---------------------------------------------------------
//
CBTUIPairedDevicesView* CBTUIPairedDevicesView::NewL(MBtuiPluginViewActivationObserver* aObserver)
    {
    CBTUIPairedDevicesView* self = new (ELeave) CBTUIPairedDevicesView(aObserver);
	CleanupStack::PushL(self);
	
	self->iConstructAsGsPlugin = EFalse;	
 	self->ConstructL();
	CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------
// CBTUIPairedDevicesView::NewLC
// Two phased constructor called by mainView when serve as GS Plugin.
// ---------------------------------------------------------
//
CBTUIPairedDevicesView* CBTUIPairedDevicesView::NewLC(CBTUIMainView* aBaseView, CArrayPtrFlat<MGSTabbedView>* aTabViewArray)
    {
    CBTUIPairedDevicesView* self = new (ELeave) CBTUIPairedDevicesView();    
	CleanupStack::PushL( self );
		
	self->iConstructAsGsPlugin = ETrue;
 	self->ConstructL( aBaseView, aTabViewArray );

    return self;
    }
    

// ---------------------------------------------------------
// CBTUIPairedDevicesView::ConstructL
// Symbian 2nd phase constructor can leave.
// ---------------------------------------------------------
//
void CBTUIPairedDevicesView::ConstructL()
    {
	TRACE_FUNC_ENTRY

    BaseConstructL(R_BTUI_OTHER_DEVICES_VIEW);

    iCoverDisplayEnabled = FeatureManager::FeatureSupported(KFeatureIdCoverDisplay); 
    iHelpEnabled = FeatureManager::FeatureSupported(KFeatureIdHelp);
    
   	iDeviceSearcher = CBTEngDiscovery::NewL(this);   	
	iBtEngSettings = CBTEngSettings::NewL(this);
	iBTPluginMan = CBTUIPluginMan::NewL( AppUi() );
	iIsAlreadyConnected = EFalse;
	iExpectedCallBack = 0;
	TRACE_INFO(_L("Initial iIsAlreadyConnected -> False"))
    TRACE_FUNC_EXIT
    }

void CBTUIPairedDevicesView::ConstructL( CBTUIMainView* aBaseView, CArrayPtrFlat<MGSTabbedView>* aTabViewArray )
    {
    TRACE_FUNC_ENTRY
    
	iBaseView = aBaseView,
    iTabHelper = CGSTabHelper::NewL();
    aTabViewArray->AppendL(this); 
    
    BaseConstructL(R_BTUI_OTHER_DEVICES_VIEW_GS);
    iCoverDisplayEnabled = FeatureManager::FeatureSupported(KFeatureIdCoverDisplay); 
    iHelpEnabled = FeatureManager::FeatureSupported(KFeatureIdHelp);
    
   	iDeviceSearcher = CBTEngDiscovery::NewL(this);
    iBtEngSettings=CBTEngSettings::NewL(this);	
    iBTPluginMan = CBTUIPluginMan::NewL( AppUi() );
	iIsAlreadyConnected = EFalse;
	TRACE_INFO(_L("Initial iIsAlreadyConnected = False"))
    iExpectedCallBack = 0;
	
	TRACE_FUNC_EXIT
    }
  
// ---------------------------------------------------------
// CBTUIBlockedDevicesView::CBTUIBlockedDevicesView
// ---------------------------------------------------------
    
CBTUIPairedDevicesView::CBTUIPairedDevicesView( MBtuiPluginViewActivationObserver* aObserver )
	: iActivationObserver(aObserver)
	{
	}        

// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CBTUIPairedDevicesView::~CBTUIPairedDevicesView()
    {
	TRACE_FUNC_ENTRY

	//close dialogs if any
	TRAP_IGNORE(
		DialogDismissedL(EAknSoftkeyCancel);
	);

	delete iActive;
	
    if (iContainer)
        {
        AppUi()->RemoveFromViewStack(*this, iContainer);
		delete iContainer;           
		iContainer = NULL;
        }
            
    if(iDeviceSearcher)
        {
    	// Canceling devicesearches is done by DialogDismissedL(),
    	// so it is not done here    	
    	delete iDeviceSearcher;
    	iDeviceSearcher = NULL ;
        }
	delete iConnectDevice;
	iConnectDevice=NULL;
	
	delete iPairingDevice;
	delete iBtEngSettings;
	delete iBTPluginMan;
	
	if(iConstructAsGsPlugin)
	    	delete iTabHelper;	
	
	CancelDisconnectQueryDlg();
	
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// From CAknView
// CBTUIPairedDevicesView::Id
// ---------------------------------------------------------
//
TUid CBTUIPairedDevicesView::Id() const
    {
	TRACE_FUNC_ENTRY    
    return KBtuiPluginUidPairedDevicesView;//KBTOtherDevicesViewId;
    }




// ---------------------------------------------------------
// From CAknView
// CBTUIPairedDevicesView::HandleCommandL 
// Redirects commands to the appui class.
// ---------------------------------------------------------
//
void CBTUIPairedDevicesView::HandleCommandL(TInt aCommand)
    {
	TRACE_FUNC_ENTRY

    switch (aCommand)
        {
        
        case EAknSoftkeyBack:      
            // do not exit if devicechange is in progress.
            if(iModel->DeviceChangeInProgress() )
            	return;                
	        AppUi()->ActivateLocalViewL(iBaseView->GetParentView().iViewUid );
            break;
            
        case EAknSoftkeyExit:
            {
			TRACE_INFO(_L("EAknSoftkeyExit"))
            // do not exit if devicechange or bt discovery is in progress.				
            if(iModel->DeviceChangeInProgress() 
                    || iSearchOngoing )   
            	{
            	TRACE_INFO(_L("exit denied, due to operation"))
            	return;
	           	}
	           	
			TRACE_INFO(_L("exit in progress"))
			AppUi()->ProcessCommandL(EEikCmdExit);            
            break;
            }

        case EAknCmdHelp:
            {
            #ifdef __SERIES60_HELP
                HlpLauncher::LaunchHelpApplicationL(iEikonEnv->WsSession(),AppUi()->AppHelpContextL());
            #endif
            break;
            }          
        	
        case EBTUICmdNewPairedDevice:
            {
            iSearchOngoing=ETrue;
            NewPairedDeviceDlgL();
            break;
            }

        case EBTUICmdDelete:
            {
            DeleteDeviceDlgL();
            break;
            }

        case EBTUICmdDeleteAll:
            {
            DeleteAllDevicesDlgL();
            break;
            }

        case EBTUICmdGiveNickname:
            {
            RenameDeviceDlgL();
            break;
            }

        case EBTUICmdSetTrusted:
            {
            ChangeDeviceSecurityL(ETrue);
            break;
            }

        case EBTUICmdSetUnTrusted:
            {
            ChangeDeviceSecurityL(EFalse);
            break;
            }

        case EBTUICmdConnect:
        case EBTUICmdConnectAudio: 
            {
            TBTDevice device;
            device.iIndex = iContainer->CurrentItemIndex();
            iModel->GetDevice(device);
            if ( device.iStatus & EStatusBtuiConnected )
                {
                // If device is already connected, we do nothing here. 
                break;
                }
            device.iOperation = EOpConnect;
            ConnectL( device, ETrue );
	        break;            	
            }

        case EBTUICmdDisconnect:
            {
            DisconnectL();
            break;
            }
  
        case EBTUICmdSetBlocked:
        	{
        	BlockDeviceDlgL();
        	break;
        	}
        	
        case EBTUICmdSettings:
        	{
        	LaunchSettingViewL();
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
// CBTUIPairedDevicesView::DoActivateL
// ---------------------------------------------------------
//
void CBTUIPairedDevicesView::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
                                               TUid /*aCustomMessageId*/,
                                         const TDesC8& /*aCustomMessage*/)
    {
	TRACE_FUNC_ENTRY

    iModel = CBTDevModel::NewL(EGroupPaired, this);
    iContainer = CBTUiDeviceContainer::NewL(ClientRect(), this, EGroupPaired, this);
    iContainer->SetMopParent(this); // MObjectProvider

    // Setup title
    HBufC* btTitlePairedDevices = 
    	iCoeEnv->AllocReadResourceLC(R_BT_TITLE_PAIRED_DEV);
    CEikStatusPane* statusPane = StatusPane();
    CAknTitlePane* titlePane;
    titlePane = (CAknTitlePane*) statusPane->ControlL(
    	TUid::Uid(EEikStatusPaneUidTitle));
    titlePane->SetTextL(btTitlePairedDevices->Des());
    CleanupStack::PopAndDestroy(btTitlePairedDevices); // btTitlePairedDevices
    btTitlePairedDevices = NULL;

    AppUi()->AddToStackL(*this, iContainer);

    if( iBaseView && iConstructAsGsPlugin)
        {
        iTabHelper->CreateTabGroupL( Id(), 
            static_cast<CArrayPtrFlat<MGSTabbedView>*>
            (iBaseView->TabbedViews()) ); 
        } 
        
	// MSK menu is added here.
	// the left key menu is added by resources.
    MenuBar()->SetContextMenuTitleResourceId(
    	R_BTUI_MENUBAR_OTHER_DEVICES_VIEW_CONTEXT);		
	// notify observer   
    if( iActivationObserver  && iConstructAsGsPlugin == EFalse)
    {    	    
        iActivationObserver->PluginViewActivated(EPairedDevicesViewId);        
    }

	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// From CAknView
// CBTUIPairedDevicesView::DoDeactivate
// ---------------------------------------------------------
//
void CBTUIPairedDevicesView::DoDeactivate()
    {
	TRACE_FUNC_ENTRY

    if (iContainer)
        {
        AppUi()->RemoveFromStack(iContainer);
        delete iContainer;
        iContainer = NULL;
        }
	if ( iDeviceSearcher )
	    {
	    iDeviceSearcher->CancelSearchRemoteDevice();
	    }
    delete iModel;
    iModel=NULL;
    if(iConstructAsGsPlugin)
	    iTabHelper->RemoveTabGroup();  
    delete iActive;
    iActive = NULL;
    iIsPairConnect = EFalse;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// From CAknView
// CBTUIPairedDevicesView::DynInitMenuPaneL
// Shows only options supported for selected device, if any.
// If not show only options supported regardless of selected
// device
// ---------------------------------------------------------
//
void CBTUIPairedDevicesView::DynInitMenuPaneL(TInt aResId, CEikMenuPane* aMenu)
    {
	TRACE_FUNC_ENTRY
	iActiveMenuPane = aMenu;
	// dim help if it exists and is not supported	
	TInt tmp;	
	if (  aMenu->MenuItemExists(EAknCmdHelp,tmp ) )			
		{			
		aMenu->SetItemDimmed(EAknCmdHelp,
		  !FeatureManager::FeatureSupported( KFeatureIdHelp ) );
		}
	
	if ( aResId == R_BTUI_OTHER_DEVICES_VIEW_MENU  )
	    {
        TInt index = iContainer->CurrentItemIndex();

        // we only show "new pair device", "help" , "Exit" and "Delete All" if view contains device(s).  
        aMenu->SetItemDimmed(EBTUICmdSetTrusted, ETrue);   
        aMenu->SetItemDimmed(EBTUICmdDelete,ETrue);
        aMenu->SetItemDimmed(EBTUICmdSetTrusted,ETrue);
        aMenu->SetItemDimmed(EBTUICmdSetUnTrusted,ETrue);
        aMenu->SetItemDimmed(EBTUICmdConnect,ETrue);
        aMenu->SetItemDimmed(EBTUICmdConnectAudio,ETrue); 
        aMenu->SetItemDimmed(EBTUICmdDisconnect,ETrue);
        aMenu->SetItemDimmed(EBTUICmdSetBlocked,ETrue);
        aMenu->SetItemDimmed(EBTUICmdSettings,ETrue);
        aMenu->SetItemDimmed(EBTUICmdGiveNickname, ETrue);         
        
        if ( index < 0 ) // No devices at list. Hide all device related commands.
            {
            aMenu->SetItemDimmed(EBTUICmdDeleteAll, ETrue); 
            }
        
	    }
	
    if ( aResId == R_BTUI_OTHER_DEVICES_VIEW_MENU_CONTEXT )
        {
        TInt index = iContainer->CurrentItemIndex();
          
        // If user is fast enough to click the RSK option between 
        // bt discovery view and device found view,  bt ui will crash.
        // Therefore, we disable commands if bt discovery is onging.
        //          
        if(iSearchOngoing)
            {
            aMenu->SetItemDimmed(EBTUICmdGiveNickname, ETrue);
            aMenu->SetItemDimmed(EBTUICmdSetTrusted, ETrue);
            aMenu->SetItemDimmed(EBTUICmdSetUnTrusted, ETrue);
            aMenu->SetItemDimmed(EBTUICmdSetBlocked, ETrue);
            aMenu->SetItemDimmed(EBTUICmdDelete, ETrue);
            aMenu->SetItemDimmed(EBTUICmdConnect, ETrue);
            aMenu->SetItemDimmed(EBTUICmdConnectAudio, ETrue);
            aMenu->SetItemDimmed(EBTUICmdDisconnect, ETrue);            
            aMenu->SetItemDimmed(EBTUICmdNewPairedDevice, ETrue);
            return;
            }
        
        if(index < 0)  // No devices at list. Hide all device related commands.
            {            
            TRACE_INFO(_L("no devices in list"))
            
            aMenu->SetItemDimmed(EBTUICmdGiveNickname, ETrue);
            aMenu->SetItemDimmed(EBTUICmdSetTrusted, ETrue);
            aMenu->SetItemDimmed(EBTUICmdSetUnTrusted, ETrue);
            aMenu->SetItemDimmed(EBTUICmdSetBlocked, ETrue);
            aMenu->SetItemDimmed(EBTUICmdDelete, ETrue);
            aMenu->SetItemDimmed(EBTUICmdConnect, ETrue);
            aMenu->SetItemDimmed(EBTUICmdConnectAudio, ETrue);
            aMenu->SetItemDimmed(EBTUICmdDisconnect, ETrue);
            aMenu->SetItemDimmed(EBTUICmdSettings, ETrue);
            }
        else // There are devices at list
            {
	        TBTDevice device;
            device.iIndex = index;
    	    iModel->GetDevice(device);
            iMenuDevice = device;    	    
            
            TRACE_INFO(_L("there are device in list"))
          
            TBool dim = (device.iStatus & EStatusTrusted) == EStatusTrusted;
            
            aMenu->SetItemDimmed(EBTUICmdSetTrusted, dim);
            aMenu->SetItemDimmed(EBTUICmdSetUnTrusted, !dim);
             
            // Check if device can be connected or disconnected
			dim = (! (device.iStatus & EStatusConnectable) 
			|| device.iStatus & EStatusBtuiConnected);
			
			if (dim) 
				{
            	aMenu->SetItemDimmed(EBTUICmdConnect, dim);
            	aMenu->SetItemDimmed(EBTUICmdConnectAudio, dim);
				}
			else
				{
				if ( device.iDeviceClass.MajorDeviceClass() == EMajorDeviceComputer) 
            		{
            		aMenu->SetItemDimmed(EBTUICmdConnect, ETrue);
            		}
            	else 
            		{
            		aMenu->SetItemDimmed(EBTUICmdConnectAudio, ETrue);
            		}
				}
			dim = (device.iStatus & EStatusBtuiConnected) != EStatusBtuiConnected;
            aMenu->SetItemDimmed(EBTUICmdDisconnect, dim);

            if ( !iBTPluginMan->IsPluginAvaiable(device.iDeviceClass) ) 
                {
                aMenu->SetItemDimmed(EBTUICmdSettings, ETrue);
                }
            else
                {                
                dim = (device.iStatus & EStatusBtuiConnected) != EStatusBtuiConnected;
                aMenu->SetItemDimmed(EBTUICmdSettings, dim);
                }
            //Hide New pair device
            aMenu->SetItemDimmed(EBTUICmdNewPairedDevice, ETrue);
            }
        }
	TRACE_FUNC_EXIT        
    }
    

// ----------------------------------------------------------
// CBTUIPairedDevicesView::RenameDeviceDlgL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::RenameDeviceDlgL()
    {
	TRACE_FUNC_ENTRY

	TInt index = iContainer->CurrentItemIndex();
	if(index == KErrNotFound) return;
    
	TBTDevice device;
    device.iIndex = index;
	iModel->GetDevice(device);
	TBTDeviceName name = device.iName;
	
    CAknTextQueryDialog* dlg = CAknTextQueryDialog::NewL(device.iName);
    TInt keypress = dlg->ExecuteLD(R_BT_ENTER_NICKNAME_QUERY);

    if(keypress)  // User has accepted the dialog
        {
        if(device.iName != name) // Check if the user really changed the name
        	{
    		device.iOperation = EOpChangeName;        
    		// Remove trouble making control chars
		    AknTextUtils::StripCharacters(device.iName, KAknStripListControlChars);    
	        device.iName.TrimAll();  // Remove extra spaces    		
        	iModel->ChangeDevice(device);        	
        	}
        }
	TRACE_FUNC_EXIT        
    }
    

// ----------------------------------------------------------
// CBTUIPairedDevicesView::NewPairedDeviceDlgL
//
// Initiate bonding (pairing) by opening the search dialog.
// DeviceSearchComplete is called after that.
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::NewPairedDeviceDlgL()
	{
	TRACE_FUNC_ENTRY	
	// we must not have other pairing in progress.
	// we maight issue an error, but none is defined by 
	// ui spec. Anyhow this should be possible
	// only by pressing everything roo rapidly.
	if(iPairingDevice!=NULL)
		return;
	iPairingDevice = CBTDevice::NewL();
	
	// Here we use last bit in device class to inform bt inquiry notifier 
	// not to display the device which is already paired and in the paired 
	// device list.  
	    
	// We use DeviceClass to tell btnotif not to show paired device in 
	// bt discovery notifier through bteng.
	// Set bit 24 to 1 ( bit 24 is not used for any other purpose currently )	
	iPairingDevice->SetDeviceClass(iPairingDevice->DeviceClass().DeviceClass() | SetExcludePairedDeviceMask );
	TInt err = iDeviceSearcher->SearchRemoteDevice(iPairingDevice,
                                                   &iPairingDevNameEntry, 
                                                   iPairingDevice->DeviceClass().DeviceClass());


	if(err != KErrNone)
		{
		delete iPairingDevice;
		iPairingDevice=NULL;
        TBTUIViewsCommonUtils::ShowGeneralErrorNoteL();        
    	}
	TRACE_FUNC_EXIT    	
	}

// ----------------------------------------------------------
// CBTUIPairedDevicesView::DeleteDeviceDlgL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::DeleteDeviceDlgL()
    {
	TRACE_FUNC_ENTRY
	
    iExpectedCallBack = 0;
	TInt index = iContainer->CurrentItemIndex();
	if(index == KErrNotFound) return;

    TBTDevice device;
    device.iIndex = index;
    iModel->GetDevice(device);

    RBuf stringholder;
    CleanupClosePushL( stringholder );

	TInt tmp=EStatusBtuiConnected|EStatusPhysicallyConnected;
    // Choose confirmation phrase
    TInt resourceId = ( device.iStatus & tmp ) ? R_BT_WARN_ERASE_CONNECTED :
        R_BT_WARN_ERASE;
    BluetoothUiUtil::LoadResourceAndSubstringL( 
            stringholder, resourceId, device.iName, 0 );

    // Create confirmation query
    //
    CAknQueryDialog* dlg = CAknQueryDialog::NewL();

	if(iCoverDisplayEnabled)
		{
        CleanupStack::PushL(dlg); 
	    dlg->PublishDialogL(ECmdShowBtWarnEraseNote, KUidCoverUiCategoryBtui); 
	    // initializes cover support    
        CleanupStack::Pop(dlg); 

		CAknMediatorFacade* covercl = AknMediatorFacade(dlg); // uses MOP, so control provided 
		if (covercl) // returns null if __COVER_DISPLAY is not defined
		    {	    
		    covercl->BufStream() << BTDeviceNameConverter::ToUTF8L(device.iName) ;// takes copy so consts are ok too
		    covercl->BufStream().CommitL(); // no more data to send so commit buf
		    }  
		}
    TInt keypress = dlg->ExecuteLD(R_BT_WARN_ERASE_QUERY, stringholder);

    CleanupStack::PopAndDestroy(&stringholder);  // stringholder

    if(keypress)  // User has accepted the dialog
        {
        // If device is connected we need to first to disconnect 
        // then unpair, otherwise unpairing does not succeed.
        if((device.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected)
        {
            iExpectedCallBack++;
            DisconnectWithoutQuery();            
            return;
        }
        device.iOperation = EOpUnpair;
        iModel->ChangeDevice(device);       
        }

	TRACE_FUNC_EXIT
    }

// ----------------------------------------------------------
// CBTUIPairedDevicesView::DeleteAllDevicesDlgL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::DeleteAllDevicesDlgL()
    {
	TRACE_FUNC_ENTRY
	
    iExpectedCallBack = 0;
    CAknQueryDialog* dlg = CAknQueryDialog::NewL();
    TInt keypress = KErrNone;
    
    CleanupStack::PushL(dlg); 

    // Choose and execute confirmation query
    //
    if (iModel->IsAnyDeviceConnected())
        {
		if(iCoverDisplayEnabled)
			{
            dlg->PublishDialogL(ECmdShowBtWarnEraseAllConnectedQuery, KUidCoverUiCategoryBtui); // initializes cover support    
			}

        keypress = dlg->ExecuteLD(R_BT_WARN_ERASE_ALL_CONNECTED_QUERY);
        }
    else
        {        
		if(iCoverDisplayEnabled)
			{
	        dlg->PublishDialogL(ECmdShowBtWarnEraseAllQuery, KUidCoverUiCategoryBtui); // initializes cover support    
			}

        keypress = dlg->ExecuteLD(R_BT_WARN_ERASE_ALL_QUERY);
        }
    CleanupStack::Pop(dlg); 

    if(keypress)  // User has accepted the dialog
        {
        TInt count = iContainer->CountItems();   
        for (TInt i = 0; i < count; i++)
           {
           // If device is connected we need to first to disconnect 
           // then unpair, otherwise unpairing does not succeed.                      
           TBTDevice device;
           device.iIndex = i;
           iModel->GetDevice(device);
           if((device.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected)
               {
               iExpectedCallBack++;
               }
           }
        TRACE_INFO((_L("iExpectedCallBack %d"), iExpectedCallBack ))
        
        for (TInt i = 0; i < count; i++)
            {
            // If device is connected we need to first to disconnect 
            // then unpair, otherwise unpairing does not succeed.                      
            TBTDevice device;
            device.iIndex = i;
            iModel->GetDevice(device);           
            
            if((device.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected)
                {
                //disconnect it first, removal will be done in 
                //callback NotifyChangeDeviceCompleteL for ChangeDevice() below
                TRACE_INFO((_L("EOpDisconnect issued for %d"), i ))
                device.iOperation = EOpDisconnect;
                iModel->ChangeDevice(device);  
                }
            else
                {
                //otherwise, just remove it from the paired view
                device.iOperation = EOpUnpair;
                iModel->ChangeDevice(device);
                }
            }
        }

	TRACE_FUNC_EXIT
    }
// ----------------------------------------------------------
// CBTUIPairedDevicesView::BlockDeviceDlgL
// ----------------------------------------------------------
void CBTUIPairedDevicesView::BlockDeviceDlgL()
	{
	TRACE_FUNC_ENTRY

	TInt index = iContainer->CurrentItemIndex();
	if(index == KErrNotFound) return;
			
    TBTDevice device;
    device.iIndex = index;
    iModel->GetDevice(device);

	//check if this is connected device
		
	HBufC* header= iCoeEnv->AllocReadResourceLC(R_BT_OPTION_BLOCK_DEVICE_HEADER); 
	
	RBuf stringHolder;
	CleanupClosePushL( stringHolder );
	TInt resouseId;
	if (device.iStatus & (EStatusBtuiConnected|EStatusPhysicallyConnected) )
	    {
	    resouseId = iHelpEnabled? R_BT_OPTION_BLOCK_PAIRED_DEVICE_HELP : 
            R_BT_OPTION_BLOCK_PAIRED_DEVICE_NOHELP;
	    }
	else
	    {
	    resouseId = iHelpEnabled ? R_BT_OPTION_BLOCK_PAIRED_DEVICE_HELP :
            R_BT_OPTION_BLOCK_PAIRED_DEVICE_NOHELP;
	    }
	BluetoothUiUtil::LoadResourceAndSubstringL( 
	        stringHolder, resouseId, device.iName, 0 );
	
	CAknMessageQueryDialog *queryDialog = CAknMessageQueryDialog::NewL (stringHolder, 
	  CAknQueryDialog::EConfirmationTone);		
	queryDialog->PrepareLC(R_BT_MESSAGE_QUERY);
	
	(queryDialog)->Heading()->SetTextL(*header);
	queryDialog->SetFocus(ETrue);

    TInt keypress=EFalse;
#ifdef __SERIES60_HELP    
	if (iHelpEnabled)
        {
		HBufC* queryLink = StringLoader::LoadL(R_BT_BLOCK_DEVICE_LINK);
		TCoeHelpContext hc(KBtuiUid3,KBT_HLP_BLOCKED);
		TCallBack helpCallBack(CBTUIPairedDevicesView::LaunchHelp , &hc);
		
		TRAPD(err,
			static_cast<CAknMessageQueryDialog*>(queryDialog)->SetLinkTextL(*queryLink);
			static_cast<CAknMessageQueryDialog*>(queryDialog)->SetLink(helpCallBack););
		if(err==KErrNone)
			{
			keypress = queryDialog->RunLD();
			}
        	
		delete queryLink; 
        queryLink = NULL;
        }
    else
#endif    
        {
        keypress = queryDialog->RunLD();
        }
	
	CleanupStack::PopAndDestroy(&stringHolder);  				 
	CleanupStack::PopAndDestroy(header);
	        
	if(keypress)
		{
		device.iOperation = EOpBlock;
		iModel->ChangeDevice(device);
		}
	TRACE_FUNC_EXIT		
	}
// ----------------------------------------------------------
// CBTUIPairedDevicesView::ChangeDeviceSecurityL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::ChangeDeviceSecurityL(TBool aTrusted)
    {
   	TRACE_FUNC_ENTRY

	TInt index = iContainer->CurrentItemIndex();
	if(index == KErrNotFound) return;
			
    TBTDevice device;
    device.iIndex = index;
    iModel->GetDevice(device);
    
    if(aTrusted)
        {
        // set to trusted
        CAknQueryDialog* dlg = CAknQueryDialog::NewL();
        
		if(iCoverDisplayEnabled)
			{
			CleanupStack::PushL(dlg); 
	        dlg->PublishDialogL(ECmdShowBtAuthConfirmQuery, KUidCoverUiCategoryBtui); // initializes cover support    
		    CleanupStack::Pop(dlg); 
			}
        TInt keypress = dlg->ExecuteLD(R_BT_SET_TRUSTED_QUERY);

        if(keypress)  // User has accepted the dialog
            {
            device.iOperation = EOpTrust;
            iModel->ChangeDevice(device);
            }
        }
    else
        {
        device.iOperation = EOpUntrust;
        iModel->ChangeDevice(device);        
        }
	TRACE_FUNC_EXIT        
    }
    
// ----------------------------------------------------
// CBTUIMainView::VisibilityModeChanged
// ----------------------------------------------------
//    
void CBTUIPairedDevicesView::VisibilityModeChanged( TBTVisibilityMode  )
    {
	TRACE_FUNC_ENTRY
	
	
	TRACE_FUNC_EXIT	
    }
    
        
// ----------------------------------------------------
// CBTUIMainView::PowerStateChanged
// ----------------------------------------------------
//
void CBTUIPairedDevicesView::PowerStateChanged( TBTPowerStateValue  )
    {
	TRACE_FUNC_ENTRY
	
	if ( iIsConnectDevice )
		{
		iModel->ChangeDevice(*iConnectDevice);
		iIsConnectDevice = EFalse;   // reset flag
		}
	
	TRACE_FUNC_EXIT	
    }
    
// ----------------------------------------------------------
// CBTUIPairedDevicesView::ConnectL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::ConnectL(
        const TBTDevice& aDevice, TBool aNewWaitNote )
	{
	TRACE_FUNC_ENTRY
	
	iIsAlreadyConnected = EFalse;
	TRACE_INFO(_L("Set iIsAlreadyConnected -> False"))
	//Check that there is not more than max amount of connections allready.		
	TInt connNum = 0;
    RProperty::Get(KPropertyUidBluetoothCategory, KPropertyKeyBluetoothPHYCount, connNum);
	if(connNum > KBtUiMaxConnections) 
  		{
        ConnectCompleteL(KErrServerBusy, aDevice);
  		}
  	else if(CallOnGoing()&& connNum > 1)
	  	{
    	CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog(CAknNoteDialog::ENoTone,
                                                       CAknNoteDialog::ELongTimeout);
		if(iCoverDisplayEnabled)
			{
			CleanupStack::PushL(dlg); 
    		dlg->PublishDialogL(ECmdShowBtImpossibleIncallNote, KUidCoverUiCategoryBtui); // initializes cover support    
			CleanupStack::Pop(dlg); 
			}
	
    	dlg->ExecuteLD(R_BT_NOT_INCALL_NOTE);    	
  		}
    else
        {
		if(TBTUIViewsCommonUtils::TurnBTOnIfNeededL(iBtEngSettings,iCoverDisplayEnabled)!= KErrNone)
			return;
		
		TInt errorCode = KErrNone;  
		TBTPowerStateValue currentPowerMode;		
		errorCode=iBtEngSettings->GetPowerState(currentPowerMode); 
		
	    if(errorCode)    
	    	{
	    	// handle the error code
	    	return;
	    	}
	    if ( aNewWaitNote )
	        {
            // Launch wait note              
            OpenWaitNoteL(iWaitForConnectDlg,R_BT_CONNECTING_TO_WAIT_NOTE,R_BT_CONNECTING_TO ,
                ECmdShowBtWaitConnectingToNote,aDevice.iName); 
	        }
		iConnectDevice=new(ELeave) TBTDevice(aDevice);            
		iConnectDevice->iOperation=EOpConnect;
		     
		if( currentPowerMode == EBTPowerOff )
			{
				// turn on the flag IsConnectDevice
				//
				iIsConnectDevice = ETrue;
			}
        else
        	{	
        		// connect device
        		//
        		iModel->ChangeDevice(*iConnectDevice);
        		
        		// reset IsConnectDevice flag
        		iIsConnectDevice = EFalse;
        	}
        }
    TRACE_FUNC_EXIT
	}
	
// ----------------------------------------------------------
// CBTUIPairedDevicesView::OpenWaitNoteL
// ----------------------------------------------------------
void CBTUIPairedDevicesView::OpenWaitNoteL(CAknWaitDialog*& aWaitDialog,TUint aNoteResource,TUint aNoteTextResource,
		TSecondaryDisplayBtuiDialogs aSec,const TDesC& aDevName)
	{
	TRAPD(err,
	        RBuf stringholder;
	        CleanupClosePushL( stringholder );
	        BluetoothUiUtil::LoadResourceAndSubstringL( 
	                stringholder, aNoteTextResource, aDevName, 0 );
	    aWaitDialog = new (ELeave) CAknWaitDialog(
                    reinterpret_cast<CEikDialog**>(&aWaitDialog), ETrue);
	    aWaitDialog->PrepareLC(aNoteResource);
	    aWaitDialog->SetTextL(stringholder);

	    if(iCoverDisplayEnabled)
	        {
	        aWaitDialog->PublishDialogL(
	          aSec, KUidCoverUiCategoryBtui); // initializes cover support    
	        CAknMediatorFacade* covercl = AknMediatorFacade(aWaitDialog); // uses MOP, so control provided 
	        if (covercl) // returns null if __COVER_DISPLAY is not defined
	            {	    
	            covercl->BufStream() << BTDeviceNameConverter::ToUTF8L(aDevName);// takes copy so consts are ok too
	            covercl->BufStream().CommitL(); // no more data to send so commit buf
	            }  
	        }	        
	    aWaitDialog->SetCallback(this); // for capturing Cancel keypress	            
	    aWaitDialog->RunLD();
	    
    	CleanupStack::PopAndDestroy(&stringholder); // stringholder
    );
    // if the above functions leaves the iWaitNote is deleted, but
    // not set to NULL. This will cause a problem, when
    // the items may be deleted again, since it has to
    // be a member variable, in order to be closed
    // programmatically.
    if(err!=KErrNone)
    	{
    	// that is why we set it to NULL here and
    	// continue leaving further.
        aWaitDialog=NULL;
		User::Leave(err);	
    	}
	}

// ----------------------------------------------------------
// CBTUIPairedDevicesView::LaunchSettingViewL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::LaunchSettingViewL()
	{
	TRACE_FUNC_ENTRY
	
	TInt index = iContainer->CurrentItemIndex();
	if(index == KErrNotFound) return;			
	TBTDevice device;
	device.iIndex = index;
	iModel->GetDevice(device);
	
	CAknView* settingView = iBTPluginMan->GetSettingViewL(device);
	if(settingView != NULL)
		{
		TUid viewId = settingView->Id();		
        if(( device.iStatus & EStatusBtuiConnected) == EStatusBtuiConnected )
            {
            TBuf8<KMaxBCBluetoothNameLen> DeviceName;
            DeviceName.Copy(device.iName);
            AppUi()->ActivateLocalViewL( viewId, KNullUid, DeviceName); 
            }
        else
            {
            AppUi()->ActivateLocalViewL( viewId ); 
            }
		}
	else 
		{
		User::Leave(KErrNotFound);
		}	
		
	TRACE_FUNC_EXIT
	}

// ----------------------------------------------------------
// CBTUIPairedDevicesView::DisconnectL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::DisconnectL()
    {
	TRACE_FUNC_ENTRY
    
	TInt index = iContainer->CurrentItemIndex();
	if(index == KErrNotFound) return;
			
    TBTDevice device;
    device.iIndex = index;
    iModel->GetDevice(device);
    
    if ( !( device.iStatus & EStatusBtuiConnected) )
        {
        // If device is already disconnected, return immediately.
        return;
        }
    
	iDisconnectQueryDevice = device;//remember device related with query dialog

    // Create confirmation query
    RBuf stringholder;
    CleanupClosePushL( stringholder );
    BluetoothUiUtil::LoadResourceAndSubstringL( 
            stringholder, R_BT_DISCONNECT_FROM, device.iName, 0 );
    if ( !iDisconnectQueryDlg )
        {
        iDisconnectQueryDlg = CAknQueryDialog::NewL();
        }

	if(iCoverDisplayEnabled)
		{
	    iDisconnectQueryDlg->PublishDialogL(ECmdShowDisconnectQuery, KUidCoverUiCategoryBtui); // initializes cover support    
	
		CAknMediatorFacade* covercl = AknMediatorFacade(iDisconnectQueryDlg); // uses MOP, so control provided 
		if (covercl) // returns null if __COVER_DISPLAY is not defined
		    {	    
		    covercl->BufStream() << BTDeviceNameConverter::ToUTF8L(device.iName);// takes copy so consts are ok too
		    covercl->BufStream().CommitL(); // no more data to send so commit buf
		    }  
		}
			
    TInt keypress = iDisconnectQueryDlg->ExecuteLD(R_BT_DISCONNECT_FROM_QUERY, stringholder);

    CleanupStack::PopAndDestroy(&stringholder);  // stringholder

	iDisconnectQueryDlg = NULL;
    if(keypress)  // User has accepted the dialog
        {
        device.iOperation = EOpDisconnect;
	    iModel->ChangeDevice(device);
		}
	TRACE_FUNC_EXIT		
	}
	
// ----------------------------------------------------------
// CBTUIPairedDevicesView::CancelDisconnectQueryDlgL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::CancelDisconnectQueryDlg()
{
	TRACE_FUNC_ENTRY
	
	if(iDisconnectQueryDlg)
	{
		delete iDisconnectQueryDlg;
		iDisconnectQueryDlg = NULL;
	}
	
	TRACE_FUNC_EXIT	
}

// ----------------------------------------------------------
// CBTUIPairedDevicesView::DisconnectWithoutQuery
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::DisconnectWithoutQuery()
{
	TRACE_FUNC_ENTRY
    
	TInt index = iContainer->CurrentItemIndex();
	if(index == KErrNotFound) return;
			
    TBTDevice device;
    device.iIndex = index;
    iModel->GetDevice(device);  
    device.iOperation = EOpDisconnect;
    iModel->ChangeDevice(device);      
}
// ----------------------------------------------------------
// CBTUIPairedDevicesView::ConnectCompleteL
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::ConnectCompleteL(TInt aError,const TBTDevice& aDevice,
	const RBTDevNameArray* aConflictingDeviceNames)
    {
	TRACE_FUNC_ENTRY

    // Dismiss wait note
    //   
    if(iConnectDevice)
        {
        delete iConnectDevice;
        iConnectDevice=NULL;
        }
    
	if ( iIsPairConnect )
	    {
	    iIsPairConnect = EFalse;
	    if ( aError != KErrNone && 
	         aError != KErrCancel &&
	         aError != KErrNotSupported &&
	         aError != KErrAlreadyExists && 
	         aError != KErrServerBusy )
	        {
	        // It is possible that we started connecting too soon before the
	        // device moves to conntable mode from pairing mode. Let's try
	        // once more:
	        if ( !iActive )
	            {
	            iActive = CBTUIActive::NewL( 
	                        this, KAutoConnect2ndTry, CActive::EPriorityStandard );
	            }
	        iActive->RequestStatus() = KRequestPending;
	        iActive->GoActive();	
	        TRequestStatus* stptr = &( iActive->RequestStatus() );
	        User::RequestComplete( stptr, KErrNone );
	        TRACE_INFO( _L("CBTUIPairedDevicesView, sceadule auto connect 2nd try") )
            return;
	        }
	    }

    if(iWaitForConnectDlg)
        {
        TRACE_INFO(_L("waiting dialog is active, dismiss it"))
        iWaitForConnectDlg->ProcessFinishedL();
        }
	
    switch (aError)
        {
        case KErrCancel:
        	TRACE_INFO(_L("aError = KErrCancel"))
            break;
        case KErrAlreadyExists:	// connection exists allready
        	{
          	RBuf stringholder;
          	CleanupClosePushL( stringholder );
          	if(aConflictingDeviceNames && aConflictingDeviceNames->Count()> 0)
        		{        		
        		if(aConflictingDeviceNames->Count() == 1)
        			{
        			BluetoothUiUtil::LoadResourceAndSubstringL( stringholder, 
        			        R_BT_DISCONNECT_FIRST_PROMPT, 
        			        *(*aConflictingDeviceNames)[0], 0 );
        			}
        		else
        			{
        			BluetoothUiUtil::LoadResourceAndSubstringL( stringholder, 
                        R_BT_DISCONNECT_FIRST_STEREO_PROMPT, 
                        *(*aConflictingDeviceNames)[0], 0 );
        			BluetoothUiUtil::AddSubstringL( stringholder, *(*aConflictingDeviceNames)[1], 1 );       					
        			}
        		TRACE_INFO(_L("KErrAllreadyExists"))      	  		  
          		}
          	else
          		{
       			// Showing a note in this case would always show a note with "%U" in it
       			// so the note is not shown at all
       			TRACE_INFO(_L("KErrAlreadyExists, but no device given, not showing a notification"))
	          	break;	
          		}

           	CAknInformationNote* note = new (ELeave) CAknInformationNote();
            
			if(iCoverDisplayEnabled)
				{
				CleanupStack::PushL(note); 			
		    	note->PublishDialogL(ECmdShowBtDisconnectFirstNote, KUidCoverUiCategoryBtui); // initializes cover support    
				CleanupStack::Pop(note); 

				CAknMediatorFacade* covercl = AknMediatorFacade(note); // uses MOP, so control provided 
				if (covercl) // returns null if __COVER_DISPLAY is not defined
			    	{	
					if(aConflictingDeviceNames && aConflictingDeviceNames->Count()> 0)			    	    
						{
							covercl->BufStream() << BTDeviceNameConverter::ToUTF8L(*(*aConflictingDeviceNames)[0]);	
						}			    	
			    	covercl->BufStream().CommitL(); // no more data to send so commit buf
			    	}  
				}
		
        	note->ExecuteLD(stringholder);
        	CleanupStack::PopAndDestroy(&stringholder);
        	}
        	break;	
		case KErrServerBusy:
			{
        	CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog(
        		CAknNoteDialog::ENoTone,CAknNoteDialog::ELongTimeout);                
			if(iCoverDisplayEnabled)
				{
				CleanupStack::PushL(dlg); 			
				dlg->PublishDialogL(ECmdBtuiShowBtBusyNote, KUidCoverUiCategoryBtui); // initializes cover support    
				CleanupStack::Pop(dlg); 			
				}
			
        	dlg->ExecuteLD(R_BT_BUSY);
			}
			break;        		
        case KErrNone:
            {           
            
            TRACE_INFO(_L("aError = KErrNone"))
            // Create note 
            //          
			CAknInformationNote* notePtr = new (ELeave) CAknInformationNote();

            RBuf stringHolder;
            CleanupClosePushL( stringHolder );
            
            BluetoothUiUtil::LoadResourceAndSubstringL( stringHolder,
                    R_BT_CONF_CONNECTED_PROMPT, aDevice.iName, 0 );
            
            if(iCoverDisplayEnabled)
                    {
                    CleanupStack::PushL(notePtr); 			
                    notePtr->PublishDialogL(ECmdBtuiShowBtConfConnected, KUidCoverUiCategoryBtui); // initializes cover support    
                    CleanupStack::Pop(notePtr); 

                    CAknMediatorFacade* covercl = AknMediatorFacade(notePtr); // uses MOP, so control provided 
                    if (covercl) // returns null if __COVER_DISPLAY is not defined
                        {	
                            covercl->BufStream() << BTDeviceNameConverter::ToUTF8L(aDevice.iName);	
                          covercl->BufStream().CommitL(); // no more data to send so commit buf
                        }  
                    }
                
            notePtr->ExecuteLD(stringHolder);        
            CleanupStack::PopAndDestroy(&stringHolder);
            }
            
            TRAP_IGNORE(LaunchSettingViewL();)
            break;

        case KErrNotSupported:
            {
            TRACE_INFO(_L("error = KErrNotSupported"))
            
            RBuf stringHolder;
            CleanupClosePushL( stringHolder );
            
            BluetoothUiUtil::LoadResourceAndSubstringL( stringHolder,
                    R_BT_DEVICE_NOT_SUPPORTED, aDevice.iName, 0 );
			
			// create note
			//
			CAknInformationNote* note = new (ELeave) CAknInformationNote();
            
			if(iCoverDisplayEnabled)
				{
				CleanupStack::PushL(note); 			
		    	note->PublishDialogL(ECmdBtuiShowBtDeviceNotSupported, KUidCoverUiCategoryBtui); // initializes cover support    
				CleanupStack::Pop(note); 

				CAknMediatorFacade* covercl = AknMediatorFacade(note); // uses MOP, so control provided 
				if (covercl) // returns null if __COVER_DISPLAY is not defined
			    	{	
						covercl->BufStream() << BTDeviceNameConverter::ToUTF8L(aDevice.iName);	
						covercl->BufStream().CommitL(); // no more data to send so commit buf
			    	}  
				}
			note->ExecuteLD(stringHolder);
        	CleanupStack::PopAndDestroy(&stringHolder);
            }
            break;

        default:
            {
            // Create note
           	CAknInformationNote* note = new (ELeave) CAknInformationNote();
            HBufC* stringHolder=StringLoader::LoadLC(R_BT_DEVICE_NOT_AVAIL_NOTE );
	      
			if(iCoverDisplayEnabled)
				{
				CleanupStack::PushL(note); 			
		    	note->PublishDialogL(ECmdBtuiShowBtDeviceNotAvail, KUidCoverUiCategoryBtui); // initializes cover support    
				CleanupStack::Pop(note); 
			
				}
		
        	note->ExecuteLD(*stringHolder);
        	CleanupStack::PopAndDestroy(stringHolder);
        	TRACE_INFO(_L("default error note BT-not-available"))
            }
            break;
        }    
	TRACE_FUNC_EXIT
    }

// ------------------------------------------------------
// CBTUIPairedDevicesView::LaunchHelp
// static help launching function for TCallback
// ------------------------------------------------------
TInt CBTUIPairedDevicesView::LaunchHelp(TAny* aCoeHelpContext)
	{	
#ifdef __SERIES60_HELP      
	//TRACE_FUNC_ENTRY
	TCoeHelpContext hc;
	
	if(aCoeHelpContext==NULL)
		hc = TCoeHelpContext(KBtuiUid3,KBT_HLP_BLOCKED);
	else
		hc = *static_cast<TCoeHelpContext*>(aCoeHelpContext);
	TRAPD(err,		 
		CArrayFix< TCoeHelpContext >* array=new (ELeave) CArrayFixFlat< TCoeHelpContext >(1);
		array->AppendL(hc);
		CleanupStack::PushL(array);
		// codescan will complain about CCoeEnv::Static(), however it is not reasonable
		// avoidable, since this is static funtion.
        HlpLauncher::LaunchHelpApplicationL(CCoeEnv::Static()->WsSession() , array);
        CleanupStack::Pop(array);
   );
    
    // the array is not deleted, since deleting it will result
    // to E32User-CBase 3. It is assumed that HlpLancher is taking care of it.
    // however this has not been documented.
    //TRACE_FUNC_EXIT
    return err;
#else
	return KErrNone;    
#endif    
	}

// ------------------------------------------------------
// CBTUIPairedDevicesView::CallOnGoing
// ------------------------------------------------------
TBool CBTUIPairedDevicesView::CallOnGoing()
	{
	TRACE_FUNC_ENTRY	
	TInt callState;
	// Try to get the call state property
    
	TInt err = RProperty::Get(KPSUidCtsyCallInformation,
	 KCTsyCallState, callState); //KPSUidTelephonyCallInformation should be used   // Check also,if retrieving the property value succeeded
	return (err == KErrNone) ?
           (callState != EPSCTsyCallStateNone && 
           callState != EPSCTsyCallStateUninitialized) :
           EFalse;
	}
// ----------------------------------------------------------
// From MProgressDialogCallback
// CBTUIPairedDevicesView::DialogDismissedL
//
// Cancels pairing/connection operation if user has pressed Cancel softkey.
// Does nothing if the dialog has been dismissed by this program.
// ----------------------------------------------------------
//
void CBTUIPairedDevicesView::DialogDismissedL(TInt aButtonId )
    {    
	TRACE_FUNC_ENTRY

	// it was not canceled, but dialog was closed
	// because it ended normally, so there is nothing to clean up.
   if( aButtonId != EAknSoftkeyCancel )
   		return;
   
    // Cancel ongoing operations if any
    //
    if ( iWaitForConnectDlg)
    	{
        if( iConnectDevice )
        	{
	        iModel->CancelChange(*iConnectDevice);
	        delete iConnectDevice;        	
			iConnectDevice=NULL;	  
			iIsPairConnect = EFalse;
        	}            	
    	iWaitForConnectDlg = NULL;
    	}    	
    if ( iWaitForPairingDlg )
        {
        if( iPairingDevice )
        	{
	        TBTDevice device;
	        device.iAddr = iPairingDevice->BDAddr();
	        device.iOperation = EOpPair;

        	iDeviceSearcher->CancelSearchRemoteDevice();
        	
	        iModel->CancelChange(device);
        	        	
	        delete iPairingDevice;        	
			iPairingDevice=NULL;	        			
        	}        
        // The dialog has already been deleted by UI framework and
        // must not be deleted manually.        	
        iWaitForPairingDlg = NULL;
        }
	
	TRACE_FUNC_EXIT
    }    
// ------------------------------------------------------
// From MBTEngSdpResultReceiver
// CBTUIPairedDevicesView::ServiceSearchComplete
// not used and implemented as empty
// ------------------------------------------------------
void CBTUIPairedDevicesView::ServiceSearchComplete(const RSdpRecHandleArray& , TUint , TInt)
	{
	TRACE_FUNC_ENTRY	
	}
// ------------------------------------------------------
// From MBTEngSdpResultReceiver
// CBTUIPairedDevicesView::AttributeSearchComplete
// not used and implemented as empty
// ------------------------------------------------------
void CBTUIPairedDevicesView::AttributeSearchComplete(TSdpServRecordHandle, 
	const RSdpResultArray& ,TInt)
	{
	TRACE_FUNC_ENTRY	
	}
// ------------------------------------------------------
// From MBTEngSdpResultReceiver
// CBTUIPairedDevicesView::AttributeSearchComplete
// not used and implemented as empty
// ------------------------------------------------------
void CBTUIPairedDevicesView::ServiceAttributeSearchComplete(TSdpServRecordHandle ,
	const RSdpResultArray& ,TInt )
	{
	TRACE_FUNC_ENTRY	
	}
// ------------------------------------------------------
// From MBTEngSdpResultReceiver
// CBTUIPairedDevicesView::DeviceSearchComplete
// ------------------------------------------------------	
void CBTUIPairedDevicesView::DeviceSearchComplete(CBTDevice* aDevice, TInt aErr)
	{
	TRACE_FUNC_ENTRY	

	TRAPD(err,	DeviceSearchCompleteL(aDevice, NULL, aErr);
		);
		// something went wrong.
	if(err != KErrNone)
		{
		TRAP_IGNORE( TBTUIViewsCommonUtils::ShowGeneralErrorNoteL(); );
		}			
    iSearchOngoing=EFalse;
	TRACE_FUNC_EXIT		
	}

// ------------------------------------------------------
// From MBTEngSdpResultReceiver
// CBTUIPairedDevicesView::DeviceSearchComplete
// ------------------------------------------------------   
void CBTUIPairedDevicesView::DeviceSearchComplete( CBTDevice* aDevice, 
        TNameEntry* aNameEntry, TInt aErr )
    {
	TRACE_FUNC_ENTRY	

	TRAPD(err,	DeviceSearchCompleteL(aDevice, aNameEntry, aErr);
		);
		// something went wrong.
	if(err != KErrNone)
		{
		TRAP_IGNORE( TBTUIViewsCommonUtils::ShowGeneralErrorNoteL(); );
		}
    iSearchOngoing=EFalse;
	TRACE_FUNC_EXIT		    
    }

// ------------------------------------------------------
// CBTUIPairedDevicesView::DeviceSearchCompleteL
// Leaving version of DeviceSearchComplete
// ------------------------------------------------------

void CBTUIPairedDevicesView::DeviceSearchCompleteL(CBTDevice* aDevice, 
        TNameEntry* aNameEntry, TInt aErr)
	{
	TRACE_FUNC_ENTRY	
    iSearchOngoing=EFalse;
	if ( !iPairingDevice )
	    {
	    delete iModel;
	    iModel = NULL;
	    return;
	    }
		// we must  not have multiple pairings at the same time
	__ASSERT_DEBUG(aDevice->BDAddr() == iPairingDevice->BDAddr(),
		PANIC(EBTPanicTwoPairingAttemptsAtTheSameTime) );

	// other error condition, most likely dialog cancel.
	if(aErr != KErrNone && (aErr != KErrAlreadyExists))
		{
		delete iPairingDevice;
		iPairingDevice=NULL;
		return;
		}

    // In case user switches the view fast enough before searching starts, 
	// It crashes the whole btui by iModel is null value.
	// We add an extra check here
	//
	if ( !iModel )
	    {
	    delete iPairingDevice;
	    iPairingDevice=NULL;
	    return;
	    }
		
	TBTDevice *tmpDevice=iModel->CreateDeviceL(aDevice, aNameEntry);
	CleanupStack::PushL(tmpDevice);	
    if(tmpDevice->iStatus & (EStatusPhysicallyConnected|EStatusBtuiConnected|EStatusPaired))	
        {
        ShowDisconnecNoteL(tmpDevice);        
        CleanupStack::PopAndDestroy(tmpDevice);            
	    delete iPairingDevice;
	    iPairingDevice=NULL;                                  
        return;
        }
    else        
	    {
    	// Launch wait note
    	OpenWaitNoteL(iWaitForPairingDlg,R_BT_WAITING,R_BT_WAITING_PROMPT,
           		ECmdShowBtWaitingNote,tmpDevice->iName);     

    	CleanupStack::PopAndDestroy(tmpDevice); 
    	tmpDevice=NULL;
    	// and start the pairing	
    	iModel->ChangeDevice(aDevice, aNameEntry, EOpPair);
    	// aDevice is not deleted, since it points to iPairingDevice and it is still used
    	// after this
        }
	TRACE_FUNC_EXIT		
	}


// ------------------------------------------------------
// CBTUIPairedDevicesView::ShowDisconnecNote
// ------------------------------------------------------	
void CBTUIPairedDevicesView::ShowDisconnecNoteL(TBTDevice *aDevice)
    {
    RBuf stringholder;
    CleanupClosePushL( stringholder );
    BluetoothUiUtil::LoadResourceAndSubstringL( 
            stringholder, R_BT_DISCONNECT_FIRST_PROMPT, aDevice->iName, 0 );
    // Launch a waiting confirmation note
    //
    CAknConfirmationNote* note = new (ELeave) CAknConfirmationNote(ETrue);

    if(iCoverDisplayEnabled)
    	{
        CleanupStack::PushL(note); 				
        note->PublishDialogL(ECmdShowBtSuccesfullyPairedNote, 
          KUidCoverUiCategoryBtui); // initializes cover support    
    	CAknMediatorFacade* covercl = AknMediatorFacade(note); // uses MOP, so control provided 
    	if (covercl) // returns null if __COVER_DISPLAY is not defined
    	    {	    
    	    covercl->BufStream() << BTDeviceNameConverter::ToUTF8L(aDevice->iName);// takes copy so consts are ok too
    	    covercl->BufStream().CommitL(); // no more data to send so commit buf
    	    }  
        CleanupStack::Pop(note); 				
    	}

    note->ExecuteLD(stringholder);

    CleanupStack::PopAndDestroy(&stringholder); // stringholder    
    
    }

void CBTUIPairedDevicesView::ConnectIfNeededAfterPairL( TInt aCount )
    {
    TRACE_FUNC_ENTRY
    TBTDevice device;
    device.iAddr = iPairConnectAddr;
    iModel->GetDevice(device);
    device.iOperation = EOpConnect;
    ConnectL( device, ( aCount ) ? EFalse : ETrue );
    TRACE_FUNC_EXIT
    }

// ------------------------------------------------------
// CBTUIPairedDevicesView::BondingCompleted
// called by NotifyChangeDeviceComplete
// ------------------------------------------------------
void CBTUIPairedDevicesView::BondingCompleteL(TInt aError, const TBTDevice& aDevice)
	{
    TRACE_INFO((_L("aError=%d"), aError))
	// We need to make a copy of the device, since it may be cleaned up
	// by device refresh.
	TBTDevice bDevice= aDevice;
	
	// this may look useless, however if the device has a given shortName it is listed with it
	// all the time. If you omnit this line it will be shown with its original deviceName,
	// dispite it will be listed with the given name
	iModel->GetDevice(bDevice);
	 	
	delete iPairingDevice;
	iPairingDevice=NULL;
	           
    // Dismiss wait note
    if(iWaitForPairingDlg)
        {
        iWaitForPairingDlg->ProcessFinishedL();
        iWaitForPairingDlg = NULL;
        }
	
    // Error handling
    switch(aError)
        {
        case KErrCancel:
            break;
        case KErrAlreadyExists: // This is a legal return value
        case KErrNone:
        case KHCIErrorBase:
            {
	        if( (bDevice.iStatus & EStatusConnectable) 
	                && (bDevice.iDeviceClass.MajorDeviceClass() != EMajorDeviceComputer)) 
	            // Only automatically query for a connectable device but not a computer
	        	{
	            iPairConnectAddr = aDevice.iAddr;
	        	iIsPairConnect = ETrue;
	        	ConnectIfNeededAfterPairL( 0 );
	        	}
            break;
			}
        default:
            {
            break;
          }
        }  
	TRACE_FUNC_EXIT
	}

// -----------------------------------------------
// From MBTDeviceObserver
// CBTUIPairedDevicesView::RefreshDeviceList
// calls the container to refresh the deviceList
// -----------------------------------------------
void CBTUIPairedDevicesView::RefreshDeviceList(const RDeviceArray* aDevices,
       TInt aSelectedItemIndex)
	{
	TRACE_FUNC_ENTRY	
	TRAPD(err,
		if(iContainer)
			{			
				iContainer->RefreshDeviceListL(aDevices,aSelectedItemIndex);			
			}
		);			
	if (iActiveMenuPane && iActiveMenuPane->NumberOfItemsInPane()>0 ) 
	    {
		TInt index = iContainer->CurrentItemIndex();
	    if ( index >=0 )
		    {
		    TBTDevice device;
		    device.iIndex = index;
		    iModel->GetDevice(device);
			if ( iMenuDevice.iAddr == device.iAddr && 
			    ( ( iMenuDevice.iStatus & EStatusBtuiConnected ) != ( device.iStatus & EStatusBtuiConnected ) ) )
			    {
			    TRACE_INFO(_L("Device status changed, update menupane"));
			    iMenuDevice = device;
			    iActiveMenuPane->Close();
			    TRAP_IGNORE( this->ProcessCommandL(EAknSoftkeyOptions) );
			    }
		    }
	    }
	if(err!= KErrNone)	    
        {
		TRAP_IGNORE( TBTUIViewsCommonUtils::ShowGeneralErrorNoteL(); );
        }
	TRACE_FUNC_EXIT        
	}


// -----------------------------------------------
// 
// -----------------------------------------------
void CBTUIPairedDevicesView::RequestCompletedL( CBTUIActive* aActive, TInt aId, 
                        TInt aStatus )
    {
    TRACE_INFO( ( _L("CBTUIPairedDevicesView::RequestCompletedL err %d"), aStatus ) )
    (void) aActive;
    (void) aId;
    (void) aStatus;
    TRACE_INFO( _L("CBTUIPairedDevicesView, auto connect 2nd try ... ") )
    ConnectIfNeededAfterPairL( 1 );
    }

// -----------------------------------------------
// 
// -----------------------------------------------
void CBTUIPairedDevicesView::HandleError( CBTUIActive* aActive, TInt aId, 
                  TInt aError )
    {
    (void) aActive;
    (void) aId;
    (void) aError;
    iIsPairConnect = EFalse;
    }

// ---------------------------------------------------------
// From MBTDeviceObserver
// CBTUIPairedDevicesView::NotifyChangeDeviceComplete
// This function implement the functionality of command complete.
// The lists of shown devices are refreshed by RefreshDeviceList, 
// not by this function
// ---------------------------------------------------------
//
void CBTUIPairedDevicesView::NotifyChangeDeviceComplete(const TInt aErr, 
		const TBTDevice& aDevice, const RBTDevNameArray* aDevNameArray)
	{
	TRACE_FUNC_ENTRY
	TBTDevice tmpDevice=aDevice;
	TRAPD(err,NotifyChangeDeviceCompleteL(aErr,tmpDevice,aDevNameArray
		););
	if(err!= KErrNone)	    
        {
		TRAP_IGNORE( TBTUIViewsCommonUtils::ShowGeneralErrorNoteL(); );
        }
	TRACE_FUNC_EXIT		
	}
// ---------------------------------------------------------
// CBTUIPairedDevicesView::NotifyChangeDeviceCompleteL
// Leaving version of notifyChangeDevice
// ---------------------------------------------------------

void CBTUIPairedDevicesView::NotifyChangeDeviceCompleteL(const TInt aErr, 
		const TBTDevice& aDevice, const RBTDevNameArray* aDevNameArray)
	{
	TRACE_FUNC_ENTRY
	if(iContainer == NULL) return;

	TRACE_INFO((_L("aErr=%d"), aErr))
	TRACE_INFO((_L("aDevice.iOperation=%d"), aDevice.iOperation))
	TRACE_INFO((_L("aDevice.iStatus=%d"), aDevice.iStatus))
	TRACE_INFO((_L("iIsAlreadyConnected=%d"), iIsAlreadyConnected))
	
    if (aDevice.iOperation == EOpDisconnect && iExpectedCallBack>0 )
        {
        TBTDevice device;
        device.iIndex = aDevice.iIndex;
        iModel->GetDevice(device);    
        device.iOperation = EOpUnpair;
        iModel->ChangeDevice(device); 
        iExpectedCallBack--;
        }
	
	if( aDevice.iOperation == EOpConnect )
		{
		if( aErr == KErrNone && iIsAlreadyConnected )
		    {
		    TRACE_INFO(_L("Don't show notes several times for the same device with different profile connections"))
		    return; 
		    }
		ConnectCompleteL(aErr, aDevice, aDevNameArray );
		iIsAlreadyConnected = ETrue;
		TRACE_INFO(_L("iIsAlreadyConnected False -> True"))
		return;
		}
	if(aDevice.iOperation == EOpPair)		
		{		
		BondingCompleteL(aErr, aDevice);
		}
	if(aErr != KErrNone)
		{
        // The user tried to rename to a name that allready exists.
        if(aDevice.iOperation == EOpChangeName && aErr == KErrAlreadyExists)
            {
            CAknNoteDialog* dlg2 = new (ELeave) CAknNoteDialog(CAknNoteDialog::EErrorTone,
                                                                CAknNoteDialog::ELongTimeout);
            dlg2->ExecuteLD(R_BT_NAME_EXISTS_NOTE);

            RenameDeviceDlgL();
            return;
            }
        // Need to properly handle situation when "Disconnect from xxx" query 
        // has been shown and paired device has been turned off simultaneously.
        // CancelDisconnectQuery dialog need to be closed. 
        if(aErr == KErrDisconnected)
            {
            if(iDisconnectQueryDlg && aDevice.iAddr == iDisconnectQueryDevice.iAddr)
                CancelDisconnectQueryDlg();
            
            TRACE_FUNC_EXIT
            return;
            }
        // show general error if any of these failed
        if(	aDevice.iOperation== EOpTrust ||
            aDevice.iOperation== EOpUntrust ||
            aDevice.iOperation== EOpBlock ||
            aDevice.iOperation== EOpChangeName)		
            {
            TBTUIViewsCommonUtils::ShowGeneralErrorNoteL();
            }
		return;
		}		
	
        if(aDevice.iOperation == EOpUntrust) 	        
            {		
            // Unauthorise ok note,
            CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog(CAknNoteDialog::EConfirmationTone,
                                                                   CAknNoteDialog::ELongTimeout);
            
            if(iCoverDisplayEnabled)
                {
                CleanupStack::PushL(dlg); 
                dlg->PublishDialogL(ECmdShowBtDevSetAsUntrustNote, KUidCoverUiCategoryBtui); 
                // initializes cover support    
                CleanupStack::Pop(dlg); 
                }
            dlg->ExecuteLD( R_BT_DEVICE_UNTRUSTED_NOTE );				
            }
	TRACE_FUNC_EXIT		
	}

// ---------------------------------------------------------------------------
// CBTUIPairedDevicesView::CreateTabIconL()
// Creates icon for tab.
// ---------------------------------------------------------------------------
//    
CGulIcon* CBTUIPairedDevicesView::CreateTabIconL()
    {
    TFileName bmpFilename;
    bmpFilename += KBitmapFileDrive;
    bmpFilename += KDC_APP_BITMAP_DIR;
    bmpFilename += KBtuiBmpFileName;
        
    CGulIcon* icon = AknsUtils::CreateGulIconL(
        AknsUtils::SkinInstance(), 
        KAknsIIDQgnPropBtDevicesTab3,  
        bmpFilename, 
        EMbmBtuiQgn_prop_bt_devices_tab3, 
        EMbmBtuiQgn_prop_bt_devices_tab3_mask );
          
    return icon;
    }   
    
// End of File

