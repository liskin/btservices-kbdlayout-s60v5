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
* Description:   This is the settings view of btui.
*
*/

#include <aknViewAppUi.h>       		// AVKON components
#include <avkon.hrh>
#include <akntitle.h>
#include <featmgr.h>            		// Feature Manager API

#include "btui.hrh"             		// Menu commands
#include "btui.h"
#include "BTUIMainView.h"
#include "BTUIMainContainer.h"
#include "debug.h"
#include "BTUIKeyWatcher.h"
#include "BTUIPairedDevicesView.h"
#include "BTUIBlockedDevicesView.h"

#include <BtuiViewResources.rsg>     	// Compiled resource ids
#include <AknGlobalNote.h>
#include <aknnotewrappers.h>
#include <aknradiobuttonsettingpage.h>
#include <akntextsettingpage.h>
#include <StringLoader.h>   			// Series 60 localisation stringloader
#include <aknslidersettingpage.h> 		//visibility time setting page
#include <data_caging_path_literals.hrh> 
#include <bautils.h>         			// BAFL utils (for language file)
#include <btui.mbg>						// application icon for General Setting
#include <AknMediatorFacade.h>
#include <AknUtils.h>					// For char filter
#include <CoreApplicationUIsSDKCRKeys.h>
#include <hlplch.h>         			// Help launcher
#include <secondarydisplay/BtuiSecondaryDisplayAPI.h>
#include <btengprivatecrkeys.h>
#include <gstabhelper.h>
#include <gsfwviewuids.h>
#include <gsparentplugin.h>
#include <utf.h>        				// CnvUtfConverter
#include <btfeaturescfg.h>					// For EnterpriseEnablementL()
#include <btnotif.h>

// ---------------------------------------------------------
// CBTUIMainView::NewL
// Two phased constructor 
// ---------------------------------------------------------
//
CBTUIMainView* CBTUIMainView::NewL(MBtuiPluginViewActivationObserver* aObserver)
    {
    //TRACE_FUNC_ENTRY

    CBTUIMainView* self = new (ELeave) CBTUIMainView(aObserver);
    
	// Set construction type, before ConstructL().	
    self->iConstructAsGsPlugin = EFalse;                  

	CleanupStack::PushL( self );	
 	self->ConstructL();
	CleanupStack::Pop(self);

	//TRACE_FUNC_EXIT
    return self;
    }


// ---------------------------------------------------------
// CBTUIMainView::NewL
// Two phased constructor
// ---------------------------------------------------------
//
CBTUIMainView* CBTUIMainView::NewGsPluginL()
    {
    //TRACE_FUNC_ENTRY

    CBTUIMainView* self = new (ELeave) CBTUIMainView(NULL);    

	// Set construction type, before ConstructL(). 
    self->iConstructAsGsPlugin = ETrue;
    
	CleanupStack::PushL( self );	
 	self->ConstructL();

	CleanupStack::Pop( self );

	//TRACE_FUNC_EXIT
    return self;
    }


// ---------------------------------------------------------
// CBTUIMainView::CBTUIMainView
// ---------------------------------------------------------
    
CBTUIMainView::CBTUIMainView(MBtuiPluginViewActivationObserver* aObserver )
	:iConstructAsGsPlugin(EFalse), iActivationObserver(aObserver)
	{
	}
// ---------------------------------------------------------
// CBTUIMainView::ConstructL
// Symbian 2nd phase constructor can leave.
// ---------------------------------------------------------
//
void CBTUIMainView::ConstructL()
    {
	TRACE_FUNC_ENTRY
	    
	// Plugin load resource manually.
	// These resources are needed by BaseConstructL
	
    iEikEnv = CEikonEnv::Static();

    TFileName filename;
    filename += KFileDrive;
    filename += KDC_RESOURCE_FILES_DIR; 
    filename += KResourceFileName;

    BaflUtils::NearestLanguageFile(iEikEnv->FsSession(),filename);
    iResourceFileFlag=iEikEnv->AddResourceFileL(filename);
        
	// According to Construction type, different softkey resource will be used.
	//
	if(iConstructAsGsPlugin)
		{
	    iViewArray = new CArrayPtrFlat<MGSTabbedView>( 3 );   
	    iTabHelper = CGSTabHelper::NewL();
	    iViewArray->AppendL(this); 
		iParentViewNotSetYet = ETrue;
		
		iKeyWatcher = CBTUIKeyWatcher::NewL(this, KBTUIPHYCountWatcher);

		// Construct OtherDeviceView
		//
	    CAknView* pairedDeviceView = CBTUIPairedDevicesView::NewLC( this, iViewArray );	
	    iAppUi->AddViewL( pairedDeviceView ); // Transfer ownership to CAknViewAppUi
	    CleanupStack::Pop(pairedDeviceView );

		// Construct BlockDeviceView
		//
	    CAknView* blockedDeviceView = CBTUIBlockedDevicesView::NewLC( this, iViewArray );
	    iAppUi->AddViewL( blockedDeviceView ); // Transfer ownership to CAknViewAppUi
	    CleanupStack::Pop(blockedDeviceView );        
		
		BaseConstructL( R_BTUI_MAIN_VIEW_GS_PLUGIN );     	
		}
	else
		{
		BaseConstructL( R_BTUI_MAIN_VIEW );
		}

    iBtEngSettings = CBTEngSettings::NewL(this);         
        
	// Sets up TLS, must be done before FeatureManager is used.
	FeatureManager::InitializeLibL();
	iCoverDisplayEnabled = FeatureManager::FeatureSupported(KFeatureIdCoverDisplay);
    	

	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// ---------------------------------------------------------
// Destructor.
// ---------------------------------------------------------
//
CBTUIMainView::~CBTUIMainView()
    {
	TRACE_FUNC_ENTRY
    
	iEikEnv->DeleteResourceFile(iResourceFileFlag);
	
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
	
	delete iBtEngSettings;
    delete iContainer;	
	delete iIdle;

    if(iConstructAsGsPlugin)
    	{
	    if ( iViewArray )
	        {
	        iViewArray->Reset();
	        delete iViewArray;
	        }    	
	        
	    delete iTabHelper;
	    delete iKeyWatcher;
    	}
    
	// Frees the TLS. Must be done after FeatureManager is used.
    FeatureManager::UnInitializeLib();  
    	

	TRACE_FUNC_EXIT
	}

// ----------------------------------------------------
// CBTUIMainView::PowerStateChanged
// ----------------------------------------------------
//
void CBTUIMainView::PowerStateChanged( TBTPowerStateValue  )
    {
	TRACE_FUNC_ENTRY
	if(iContainer)
    	{    
        iContainer->SettingChanged(EBTMainListItemPowerMode);
    	}
	
	if( iConstructAsGsPlugin )
		{
		UpdateParentView();
		}
	TRACE_FUNC_EXIT	
    }
// ----------------------------------------------------
// CBTUIMainView::VisibilityModeChanged
// ----------------------------------------------------
//    
void CBTUIMainView::VisibilityModeChanged( TBTVisibilityMode  )
    {
	TRACE_FUNC_ENTRY
	if(iContainer)
    	{
    	iContainer->SettingChanged(EBTMainListItemVisibilityMode);
    	}
    
    if( iConstructAsGsPlugin )
		{
		UpdateParentView();
		}	
    }
    
// ---------------------------------------------------------
// From CAknView
// CBTUIMainView::Id
// ---------------------------------------------------------
//
TUid CBTUIMainView::Id() const
    {
	TRACE_FUNC_ENTRY    
    if(iConstructAsGsPlugin)
    	{
	    return KGsBtuiPluginUid; //same as ecom impl UID, required by GS interface.
    	}
    else
    	{
    	return KBtuiPluginUidMainView;    //view id
    	}
    }
// ---------------------------------------------------------
// CBTUIMainView::HandleMiddleSoftkey
// Handles middle soft key
// ---------------------------------------------------------
//    
void CBTUIMainView::HandleMiddleSoftkey()
    {
		TRACE_FUNC_ENTRY 	
       	if( iContainer == NULL ) return;
    	
        TInt currentHighLightPosition= iContainer->CurrentItemIndex();
        switch(currentHighLightPosition)
            {
            case EBTMainListItemPowerMode:
                HandleCommand( EBTUICmdChangePowerStateSelect );
                break;
            case EBTMainListItemVisibilityMode:
                HandleCommand( EBTUICmdChangeVisibilitySelect );
                break;
            case EBTMainListItemNameSetting:
                HandleCommand( EBTUICmdChangeBTNameSelect );
                break;
                 
			// For Bluetooth SIM Access Profile feature
			//
			case EBTMainListItemSapMode:
				HandleCommand( EBTUICmdChangeSapStateSelect );
				break;
            default:
                break;
            }
		TRACE_FUNC_EXIT	        	    
    }    
// ---------------------------------------------------------
// TUid CBTUIMainView::HandleCommand
// ---------------------------------------------------------
//   
void CBTUIMainView::HandleCommand(TInt aCommand)    
{
    TRAP_IGNORE(HandleCommandL(aCommand));    
}
// ---------------------------------------------------------
// CBTUIMainView::HandleCommandL
// Redirects commands to the appui class.
// ---------------------------------------------------------
//
void CBTUIMainView::HandleCommandL(TInt aCommand)
    {
	TRACE_INFO((_L(">> CBTUIMainView::HandleCommandL: %d"), aCommand))        
    switch ( aCommand )
        {
        //middle soft key is handled here
        case EBTUICmdMSK:
	 		 HandleMiddleSoftkey();
	 		 break;
         case EAknSoftkeyBack:
	         AppUi()->ActivateLocalViewL(iParentView.iViewUid);
             break;

        case EAknSoftkeyExit:
            {
			AppUi()->ProcessCommandL(EEikCmdExit);
            break;
            }
            
#ifdef __SERIES60_HELP
        case EAknCmdHelp:
            {
            HlpLauncher::LaunchHelpApplicationL(
            	iEikonEnv->WsSession(), AppUi()->AppHelpContextL());
            break;
            }          
#endif
        case EBTUICmdChangePowerStateMenu:
            {
            AskPowerModeDlgL();
            break;
            }

        case EBTUICmdChangePowerStateSelect:
            {            
            TBTPowerStateValue powerState;
            iBtEngSettings->GetPowerState(powerState);                        
            if(powerState==EBTPowerOn)
                {
                SetPowerStatusL( EFalse );
                }
            else
                {
                SetPowerStatusL( ETrue );
                }            
            break;
            }

        case EBTUICmdChangeVisibilitySelect: // now both select key and menu command will triger the setting page.
        case EBTUICmdChangeVisibilityMenu:
            {
            AskVisibilityDlgL();
            break;
            }

        case EBTUICmdChangeBTNameMenu:
        case EBTUICmdChangeBTNameSelect:
            {
            AskNewBtNameDlgL();
            iContainer->SettingChanged(TBTMainListItemIndexes(iContainer->CurrentItemIndex())); 
            break;
            }
		
		// For Bluetooth SIM Access Profile feature
		case EBTUICmdChangeSapStateSelect:
            {
	        
	        if ( BluetoothFeatures::EnterpriseEnablementL() == BluetoothFeatures::EDataProfilesDisabled )
	        	{
		        RNotifier notifier;
		        User::LeaveIfError(notifier.Connect());
		        CleanupClosePushL(notifier);
				User::LeaveIfError(notifier.StartNotifier(KBTEnterpriseItSecurityInfoNotifierUid, KNullDesC8 ));
				CleanupStack::PopAndDestroy(&notifier);
	        	}
	        else
	        	{
	        	TInt sapStatus;
	        	CRepository* repository = CRepository::NewL(KCRUidBTEngPrivateSettings);
	        	CleanupStack::PushL(repository);            	
				repository->Get(KBTSapEnabled, sapStatus);
				CleanupStack::PopAndDestroy(repository);			    
				
	           if(sapStatus==EBTSapEnabled)
				    {
	                 SetSapStatusL( EBTSapDisabled );
	                }
	           else
	                {
	                 SetSapStatusL( EBTSapEnabled );
					}     
	            iContainer->SettingChanged(
	              TBTMainListItemIndexes(iContainer->CurrentItemIndex())); 				  
              	}     
            break;
            }

		// For Bluetooth SIM Access Profile feature
        case EBTUICmdChangeSapStateMenu:
            {
            AskSapModeDlgL();
            iContainer->SettingChanged(
              TBTMainListItemIndexes(iContainer->CurrentItemIndex())); 				
            break;
            }				

        default:
            {
            AppUi()->HandleCommandL( aCommand );
			iContainer->SettingChanged(
			  TBTMainListItemIndexes(iContainer->CurrentItemIndex())); 
            break;
            }
        }
	TRACE_FUNC_EXIT        
    }


// ---------------------------------------------------------
// From CAknView
// CBTUIMainView::DoActivateL
// ---------------------------------------------------------
//
void CBTUIMainView::DoActivateL( const TVwsViewId&  aPrevViewId,
                                 TUid /*aCustomMessageId*/,
                                 const TDesC8& /*aCustomMessage*/)
    {            
    TRACE_FUNC_ENTRY
    
    
	if ( BluetoothFeatures::EnterpriseEnablementL() == BluetoothFeatures::EDisabled ) 
		{
		RNotifier notifier;
		User::LeaveIfError(notifier.Connect());
		CleanupClosePushL(notifier);
		User::LeaveIfError(notifier.StartNotifier(KBTEnterpriseItSecurityInfoNotifierUid, KNullDesC8 ));
		CleanupStack::PopAndDestroy(&notifier);
		
		AppUi()->ActivateLocalViewL(aPrevViewId.iViewUid); //go back straight away
		return;
		}
    
    
    if( iConstructAsGsPlugin )
        {
	 	// Only set parent view when entering BT GS setting.
		// since tab change chould make aPrevViewId assign to non-parent views. e.g. paired or blocked view.
		if(iParentViewNotSetYet)
			{
	        iParentView = aPrevViewId; 
	        iParentViewNotSetYet = EFalse;
			}

		// Setup tab group
        iTabHelper->CreateTabGroupL( Id(), iViewArray ); 
        } 
        
    iContainer = new (ELeave) CBTUIMainContainer;
    iContainer->SetMopParent(this); // MObjectProvider
    iContainer->ConstructL( ClientRect(), this );

    // Setup the title
    //
    HBufC* btTitleMain = iCoeEnv->AllocReadResourceLC(R_BT_TITLE_BLUETOOTH);
    CEikStatusPane* statusPane = StatusPane();
    CAknTitlePane* titlePane;
    titlePane = (CAknTitlePane*) statusPane->ControlL(TUid::Uid(EEikStatusPaneUidTitle));
    titlePane->SetTextL(btTitleMain->Des());
    CleanupStack::PopAndDestroy(btTitleMain); // btTitleMain
    btTitleMain = NULL;

    AppUi()->AddToStackL( *this, iContainer );
	
	//Check if the local name has been set. If not ask user to set name
	
	TBool nameStatus = EFalse;
    TInt err = IsLocalNameModifiedL( nameStatus );
    
    if( err || nameStatus == EBTLocalNameDefault )
    	{
    	InitiateBTLocalNameQueryL();    // Ask local Bluetooth name if not given yet.
    	}
	
	// notify observer   
    if( iActivationObserver  && iConstructAsGsPlugin == EFalse)
    {    	    
        iActivationObserver->PluginViewActivated(EMainViewId);        
    }
    
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// From CAknView
// CBTUIMainView::DoDeactivate
// ---------------------------------------------------------
//
void CBTUIMainView::DoDeactivate()
    {
	TRACE_FUNC_ENTRY

    if ( iContainer )
        {
        AppUi()->RemoveFromStack( iContainer );
        }

    delete iContainer;
    iContainer = NULL;    
     
    if(iConstructAsGsPlugin)
    	{
    	iTabHelper->RemoveTabGroup();
	    CGSBaseView::DoDeactivate();  	
    	}
	TRACE_FUNC_EXIT
    
    }

// ---------------------------------------------------------
// From CAknView
// CBTUIMainView::DynInitMenuPaneL
// Shows selected item's context sensitive options menu.
// ---------------------------------------------------------
//
void CBTUIMainView::DynInitMenuPaneL( TInt aResId, CEikMenuPane* aMenu )
    {
	TRACE_FUNC_ENTRY
    
	// dim help if it exists and is not supported	
	TInt tmp;	
	if (  aMenu->MenuItemExists(EAknCmdHelp,tmp ) )			
		{			
		aMenu->SetItemDimmed(EAknCmdHelp,
		  !FeatureManager::FeatureSupported( KFeatureIdHelp ) );
		}
			
    if ( aResId == R_BTUI_MAIN_VIEW_MENU )
        {
        // Set all menus dimmed at first(hide them all)
        //
        aMenu->SetItemDimmed(EBTUICmdChangePowerStateMenu, ETrue);
        aMenu->SetItemDimmed(EBTUICmdChangeVisibilityMenu, ETrue);
        aMenu->SetItemDimmed(EBTUICmdChangeBTNameMenu, ETrue);
        if ( FeatureManager::FeatureSupported( KFeatureIdBtSap ) )
            aMenu->SetItemDimmed(EBTUICmdChangeSapStateMenu, ETrue);

        }
	TRACE_FUNC_EXIT        
    }

// ---------------------------------------------------------
// CBTUIMainView::SettingChangedL
// Redraws the BT Name field.
// ---------------------------------------------------------
//
void CBTUIMainView::SettingChangedL( TBTMainListItemIndexes aListItem )
	{
	TRACE_FUNC_ENTRY
	if(iContainer)
		{
		iContainer->SettingChanged(aListItem);	
		}
	
	}

// ----------------------------------------------------
// CBTUIMainView::SetPowerStatusL
// ----------------------------------------------------
//
TInt CBTUIMainView::SetPowerStatusL( TBool aValue )
    {
	TRACE_FUNC_ENTRY

    TInt errorCode = KErrNone;    
    TBTPowerStateValue previousPowerMode;
    errorCode=iBtEngSettings->GetPowerState(previousPowerMode); 
    if(errorCode)    
    	return errorCode;    
                   
    TBool legalToChange = ETrue;

    if ( ( previousPowerMode==EBTPowerOn ) && ( !aValue )) // power ON -> OFF
        {
        TRACE_INFO(_L("power ON -> OFF"))

        // Check active connections before query                        
        if( TBTUIViewsCommonUtils::IsAnyDeviceConnectedL())
        	{
            CAknQueryDialog* dlg = CAknQueryDialog::NewL();

			if(iCoverDisplayEnabled)
				{
				CleanupStack::PushL(dlg);
    			dlg->PublishDialogL(ECmdShowBtSwitchOffAnywayQuery, KUidCoverUiCategoryBtui); // initializes cover support				
				CleanupStack::Pop(dlg);
				}

            TInt keypress = dlg->ExecuteLD( R_BT_SWITCH_OFF_ANYWAY_QUERY );
            if( !keypress )  // User has not accepted the dialog
                {
                legalToChange = EFalse;
                }
            }
        TRACE_INFO((_L("power ON -> OFF, legalToChange = %d"), legalToChange))

        }
    else if ( ( !previousPowerMode ) && ( aValue ) ) // power OFF -> ON
        {
        TRACE_INFO(_L("power OFF -> ON"))

        TInt offlineModeOff = 0;    // possible values are 0 and 1
        TInt activationEnabled = 0; // possible values are 0 and 1
               
        CRepository* repository = CRepository::NewL(KCRUidCoreApplicationUIs);
        CleanupStack::PushL(repository);
        repository->Get(KCoreAppUIsNetworkConnectionAllowed, offlineModeOff);
		// Closing connection:
		CleanupStack::PopAndDestroy(repository);        
        repository =NULL;                        
        repository = CRepository::NewL(KCRUidBluetoothEngine);
		// Check activation enabled
		CleanupStack::PushL(repository);
		repository->Get(KBTEnabledInOffline, activationEnabled);
		// Closing connection:
		CleanupStack::PopAndDestroy(repository);
        
                
		TRACE_INFO((_L("power OFF -> ON, offlineModeOff = %d"), offlineModeOff ))
		TRACE_INFO((_L("power OFF -> ON, activationEnabled = %d"),  activationEnabled ))

        // Is offline mode on
        //
        if ( !offlineModeOff )
            {
            // Choose user interaction
            //
            if ( activationEnabled )
                {
                // Create and show query
                //
                CAknQueryDialog* dlg = CAknQueryDialog::NewL();
                
				if(iCoverDisplayEnabled)
					{  
					CleanupStack::PushL(dlg);
    	   	    	dlg->PublishDialogL(ECmdShowBtActivateInOfflineQuery, KUidCoverUiCategoryBtui); // initializes cover support
					CleanupStack::Pop(dlg);
					}

                TInt keypress = dlg->ExecuteLD( R_BT_ACTIVATE_IN_OFFLINE_QUERY );
                if( !keypress ) // User has not accepted the dialog
                    {
                    legalToChange = EFalse;
                    }
                }
            else
                {
                legalToChange = EFalse;

                // Load a string, create and show a note
                //
                HBufC* stringHolder = StringLoader::LoadLC( R_BT_OFFLINE_DISABLED );
                CAknInformationNote* note = new (ELeave) CAknInformationNote();
                
				if(iCoverDisplayEnabled)
					{  
					CleanupStack::PushL(note);
 	    	   	    note->PublishDialogL(ECmdShowBtOfflineDisabledNote, KUidCoverUiCategoryBtui); // initializes cover support
					CleanupStack::Pop(note);
					}
                
                note->ExecuteLD( *stringHolder );
                CleanupStack::PopAndDestroy(stringHolder); //stringHolder
                }
            }
        }

    TRACE_INFO((_L("legalToChange = %d"), legalToChange))

    // Switch powermode
    //
    if ( legalToChange )
        {
        
        TInt errorCode = KErrNone;    
        TBTPowerStateValue powerMode;
        if(aValue)
    	    {
    		powerMode=EBTPowerOn;	    	
    	    }    	
        else
        	{
        	powerMode=EBTPowerOff;
        	}
        	
        errorCode=iBtEngSettings->SetPowerState(powerMode); 
                
        if( (errorCode == KErrNone ) && (aValue))
        
        	{
        	//Check the visibility mode
        	TBTVisibilityMode visiMode;
        	errorCode=iBtEngSettings->GetVisibilityMode(visiMode);
        	if(errorCode==KErrNone)
	        	{        		            		      	        		        	
	        	if (visiMode == EBTVisibilityModeHidden)
	        		{
	        		//Show note
	        		CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog( CAknNoteDialog::EConfirmationTone,
	                                                                 CAknNoteDialog::ELongTimeout );
					if(iCoverDisplayEnabled)
						{  
						CleanupStack::PushL(dlg);
		    	   	    dlg->PublishDialogL(ECmdBtuiShowBtHiddenFromOthersNote, KUidCoverUiCategoryBtui); // initializes cover support
						CleanupStack::Pop(dlg);
						}
											
	                dlg->ExecuteLD( R_BT_HIDDEN_FROM_OTHERS );
	        		}
	        	} 
        	}
        if ( errorCode != KErrNone && errorCode != KErrAlreadyExists ) 
            {
            TRAP_IGNORE(TBTUIViewsCommonUtils::ShowGeneralErrorNoteL());
            }
           
        }
        
    TRACE_INFO((_L("SetPowerStatusL() complete. errorCode = %d"), errorCode))
    return errorCode;
    }

// ----------------------------------------------------------
// CBTUIMainView::SetVisibilityStatusL
// ----------------------------------------------------------
//
void CBTUIMainView::SetVisibilityStatusL( TBTVisibilityMode aMode )
    {
	TRACE_FUNC_ENTRY

    TInt errorCode = KErrNone;
    
    switch( aMode )
        {
            case EBTVisibilityModeGeneral:
            {            
            errorCode = iBtEngSettings->SetVisibilityMode(aMode);
            
			if(errorCode == KErrNone)
				{
	            CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog( CAknNoteDialog::EConfirmationTone,
                                                               CAknNoteDialog::ELongTimeout );

				if(iCoverDisplayEnabled)
					{  
					CleanupStack::PushL(dlg);
	    	   	    dlg->PublishDialogL(ECmdShowBtPhoneVisibToAllNote, KUidCoverUiCategoryBtui); // initializes cover support
					CleanupStack::Pop(dlg);
					}
    	        dlg->ExecuteLD( R_BT_PHONE_VISIB_TO_OTH_NOTE );
				}
            }
            break;

            case EBTVisibilityModeHidden:
            {            
            errorCode = iBtEngSettings->SetVisibilityMode(aMode);
            
			if(errorCode == KErrNone)
				{
	            CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog( CAknNoteDialog::EConfirmationTone,
                                                               CAknNoteDialog::ELongTimeout );

				if(iCoverDisplayEnabled)
					{  
					CleanupStack::PushL(dlg);
	    	   	    dlg->PublishDialogL(ECmdBtuiShowBtHiddenFromOthersNote, KUidCoverUiCategoryBtui); // initializes cover support
					CleanupStack::Pop(dlg);
					}
    	        dlg->ExecuteLD( R_BT_HIDDEN_FROM_OTHERS );		
				}
            }
            break;
                    
            case EBTVisibilityModeTemporary:
            {
            // Asks user to turn on Bluetooth, if temporary visibility is chosen.            
            if( TBTUIViewsCommonUtils::TurnBTOnIfNeededL(iBtEngSettings,iCoverDisplayEnabled) != KErrNone)
            	{
            	return;
            	}            
		    TInt sliderValue = 1; 			    
		    CAknSettingPage* dlg = 
		        new(ELeave) CAknSliderSettingPage(R_VISIBILITY_TIME_SETTING_PAGE, 
		                                          sliderValue);			 
		    
		    if (dlg->ExecuteLD(CAknSettingPage::EUpdateWhenChanged))
		        {			        		        
		        errorCode = iBtEngSettings->SetVisibilityMode( aMode, sliderValue );
		        }
            }
            break;

            default:
                break;
        }

    if((errorCode != KErrNone) && (errorCode != KErrAlreadyExists))
        {
	    TRAP_IGNORE(TBTUIViewsCommonUtils::ShowGeneralErrorNoteL());
        }
	TRACE_FUNC_EXIT        
    }
    
// ----------------------------------------------------------
// CBTUIMainView::GetBTLocalName
// ----------------------------------------------------------
//
TDesC* CBTUIMainView::GetBTLocalName()
    {
	TRACE_FUNC_ENTRY

    iTempNameBuffer.Zero();                 // Empty Local BT device name   
    TInt err=iBtEngSettings->GetLocalName( iTempNameBuffer );// Store name into member variable
    if(err!=KErrNone)
    	TRAP_IGNORE(TBTUIViewsCommonUtils::ShowGeneralErrorNoteL());

    // Cut the excess part of the name away. The query can handle only 
    // KBTUIMaxNameLength characters.
    //
    if( iTempNameBuffer.Length() > KBTUIMaxNameLength )
        {
        iTempNameBuffer.SetLength( KBTUIMaxNameLength );
        }
	TRACE_FUNC_EXIT
    return &iTempNameBuffer;                // And return reference for it
    }

// ----------------------------------------------------------
// CBTUIMainView::AskNewBtNameDlgL
// ----------------------------------------------------------
//
void CBTUIMainView::AskNewBtNameDlgL()
    {
    TRACE_FUNC_ENTRY

    // Cut the excess part of the name away. The query can handle only 
    // KBTUIMaxNameLength characters.
    //
    if( iTempNameBuffer.Length() > KBTUIMaxNameLength )
        {
        iTempNameBuffer.SetLength( KBTUIMaxNameLength );
        }

    FOREVER
        {
        CAknTextSettingPage* dlg = new( ELeave )CAknTextSettingPage( R_BT_NAME_SETTING_PAGE,
                                                                     iTempNameBuffer );
		if( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ))
            {
            // Remove trouble making control chars
	        AknTextUtils::StripCharacters(iTempNameBuffer,  KAknStripListControlChars);    
	                 
            iTempNameBuffer.TrimAll();                  // Remove spaces and check what is left..
            if( iTempNameBuffer.Length() > 0 )
                {
                
                if(iBtEngSettings->SetLocalName(iTempNameBuffer) != KErrNone)
                    {
                    TBTUIViewsCommonUtils::ShowGeneralErrorNoteL();
                    }
				TRACE_FUNC_EXIT       
                return;
                }
            }
        else
            {
            TRACE_FUNC_EXIT       
            return; // User has cancelled this operation
            }
        }
	 
    }

// ----------------------------------------------------------
// CBTUIMainView::AskPowerModeDlgL
// ----------------------------------------------------------
//
void CBTUIMainView::AskPowerModeDlgL()
    {
    TRACE_FUNC_ENTRY

    CDesCArrayFlat* items = iCoeEnv->ReadDesCArrayResourceL( R_BT_POWER_MODES );
    CleanupStack::PushL(items);
    TBTPowerStateValue powerState;
    iBtEngSettings->GetPowerState(powerState);          
    TInt currentItem = !powerState; // Implicit cast from Not-TBool to int (inverted)

    CAknRadioButtonSettingPage* dlg = new ( ELeave )CAknRadioButtonSettingPage( 
      R_BT_POWER_SETTING_PAGE, currentItem, items );
    if( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ))
        {
        SetPowerStatusL( static_cast<TBool>( !currentItem )); // Cast it back (invert again)
        }
    CleanupStack::PopAndDestroy(items); // items
    TRACE_FUNC_EXIT
    }

// ----------------------------------------------------------
// CBTUIMainView::AskVisibilityDlgL
// ----------------------------------------------------------
//
void CBTUIMainView::AskVisibilityDlgL()
    {
	TRACE_FUNC_ENTRY

    CDesCArrayFlat* items = iCoeEnv->ReadDesCArrayResourceL( R_BT_VISIBILITY_MODES );
    CleanupStack::PushL(items);

    TInt currentItem = 0;  // Current position at radio button setting page    
    TBTVisibilityMode currentMode;
    TInt errorCode(iBtEngSettings->GetVisibilityMode(currentMode));    
    if(errorCode==KErrNone)
	    {

	    switch( currentMode )   // Map from mode to radio button position at setting page
	        {
	        case EBTVisibilityModeGeneral:
	            currentItem = 0;
	            break;
	        case EBTVisibilityModeTemporary: // temp visible
	            currentItem = 1;
	            break;	        
	        case EBTVisibilityModeHidden:
	            currentItem = 1+1;
	            break;
	        default:
	            break;
	        }

	    // Ask new mode from user

	    CAknRadioButtonSettingPage* dlg = new ( ELeave )CAknRadioButtonSettingPage( 
	      R_BT_VISIBILITY_SETTING_PAGE, currentItem, items );
	    if( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ))
	        {
	        switch( currentItem )   // Map it back from radio button position to mode at setting page
	            {
	            case 0:	                
	                SetVisibilityStatusL( EBTVisibilityModeGeneral );	                
	                break;
	                
	            case 1:	                
	                SetVisibilityStatusL( EBTVisibilityModeTemporary );
		            break;
		            
	            case 1+1:
	                SetVisibilityStatusL( EBTVisibilityModeHidden );
	                break;
	                
	            default:
	                break;
	            }
	        }
	    }  
	else
		TBTUIViewsCommonUtils::ShowGeneralErrorNoteL();  
   	        
  	
    CleanupStack::PopAndDestroy(items); // items
    }

  
// ----------------------------------------------------
// CBTUIMainView::GetBtSettingsReference
//
// This method returns the reference to the settings class
// ----------------------------------------------------
//
CBTEngSettings* CBTUIMainView::GetBtSettingsReference()
	{
	TRACE_FUNC
	return iBtEngSettings;
	}

// ----------------------------------------------------
// CBTUIMainView::InitiateBTLocalNameQueryL
//
// Initiate BT local name query after Symbian OS has entered
// idle state.
// ----------------------------------------------------
//
void CBTUIMainView::InitiateBTLocalNameQueryL()
    {
	TRACE_FUNC
    
        	
    __ASSERT_DEBUG( !iIdle, PANIC(EBTPanicClassMemberVariableNotNull) );

    // Launch idle time regognition and execute local BT name query when ready
    iIdle = CIdle::NewL(CActive::EPriorityIdle);
    iIdle->Start(TCallBack(LaunchBTLocalNameQueryL, this));		
    }

// ----------------------------------------------------
// CBTUIMainView::LaunchBTLocalNameQueryL
//
// Execute BT local name query. Symbian OS has entered
// idle state now.
// ----------------------------------------------------
//
TInt CBTUIMainView::LaunchBTLocalNameQueryL(TAny* aMainView)
    {
	//TRACE_FUNC

    TInt errorCode = static_cast<CBTUIMainView*>(aMainView)->AskBTLocalNameQueryL();
    if( errorCode != KErrNone )
        {
        CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog( CAknNoteDialog::EErrorTone,
                                                           CAknNoteDialog::ELongTimeout );
		if(static_cast<CBTUIMainView*>(aMainView)->iCoverDisplayEnabled)
			{  
			CleanupStack::PushL(dlg);
       	    dlg->PublishDialogL(ECmdShowBtGeneralErrorNote, KUidCoverUiCategoryBtui); // initializes cover support}
			CleanupStack::Pop(dlg);
			}			
		dlg->ExecuteLD( R_BT_GENERAL_ERROR_NOTE );
        }
    return KErrNone;
    }

// ----------------------------------------------------
// CBTUIMainView::AskBTLocalNameQueryL
// ----------------------------------------------------
//
TInt CBTUIMainView::AskBTLocalNameQueryL()
    {
    TRACE_FUNC_ENTRY

    TInt errorCode = KErrNone;
    iTempNameBuffer.Zero();                     // Empty Local BT device name    
    
    //Get default name if given by e.g. phone product
    errorCode = RProperty::Get( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothSetDeviceName, iTempNameBuffer );
    
    if( errorCode || !iTempNameBuffer.Length() )
    {
        errorCode = RProperty::Get( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothGetDeviceName, iTempNameBuffer );
        
        if( errorCode || !iTempNameBuffer.Length() )
        {
            RBTRegServ btRegServ;
            RBTLocalDevice btReg;
            TBTLocalDevice localDev;

            errorCode = btRegServ.Connect();

            if( !errorCode )
            {
                errorCode = btReg.Open( btRegServ );
                
                if( !errorCode )
                {
                    // Read the BT local name from BT Registry.
                    errorCode = btReg.Get( localDev );
                    
                    if( !errorCode )
                    {
                        // The error can be > 0 if there are unconverted characters.
                        errorCode = CnvUtfConverter::ConvertToUnicodeFromUtf8( iTempNameBuffer, localDev.DeviceName() );
                    }
                }
            }

            btReg.Close();
            btRegServ.Close();
        }
    }
    
	if(errorCode==KErrNone)
		{
			
	    // Cut the excess part of the name away. The query can handle only 
	    // KBTUIMaxNameLength characters.
	    //
	    if( iTempNameBuffer.Length() > KBTUIMaxNameLength )
	        {
	        iTempNameBuffer.SetLength( KBTUIMaxNameLength );
	        }

	    CAknTextQueryDialog* dlg = CAknTextQueryDialog::NewL( iTempNameBuffer );
	    TInt keypress = dlg->ExecuteLD( R_BT_ENTER_LOCAL_NAME_QUERY );

	    if( keypress )  // User has accepted the dialog
	        {
	        // Remove trouble making control chars
		    AknTextUtils::StripCharacters(iTempNameBuffer,  KAknStripListControlChars);    
		                
	        iTempNameBuffer.TrimAll();                           // Remove extra spaces	        
	        errorCode = iBtEngSettings->SetLocalName(iTempNameBuffer);
	        
		 	this->SettingChangedL(EBTMainListItemNameSetting);	// Redraw BT name 
	        }
		}

    delete iIdle; iIdle = NULL;	
    TRACE_FUNC_EXIT
    return errorCode;
    }

//---------------------------------------------------------------------
// CBTUIMainView::GetCaption
//----------------------------------------------------------------------
//
void CBTUIMainView::GetCaptionL( TDes& aCaption ) const
	{
	TRACE_FUNC
	
    // the resource file is already opened.
    HBufC* result = StringLoader::LoadL( R_GS_BTUI_VIEW_CAPTION );
    aCaption.Copy( *result );
    delete result;	
	}


//---------------------------------------------------------------------
// CBTUIMainView::CreateIconL
//----------------------------------------------------------------------
//    
CGulIcon* CBTUIMainView::CreateIconL( const TUid aIconType )
	{
	TRACE_FUNC_ENTRY
	
	RefreshSettingsStatusL();		
    CGulIcon* icon = NULL;
    TFileName bmpFilename;
    bmpFilename += KBitmapFileDrive;
    bmpFilename += KDC_APP_BITMAP_DIR;
    bmpFilename += KBtuiBmpFileName;

    if(aIconType == KGSIconTypeLbxItem)
    	{
	
	    switch(iBtState)
			{
			case EBTOff:
				icon = AknsUtils::CreateGulIconL(
			        AknsUtils::SkinInstance(), 
			        KAknsIIDQgnPropSetAppsBtOff,  
			        bmpFilename, 
			        EMbmBtuiQgn_prop_set_apps_bt_off, 
			        EMbmBtuiQgn_prop_set_apps_bt_off_mask );
				break;
			case EBTOnShown:
				icon = AknsUtils::CreateGulIconL(
			        AknsUtils::SkinInstance(), 
			        KAknsIIDQgnPropSetAppsBtOnShown,  
			        bmpFilename, 
			        EMbmBtuiQgn_prop_set_apps_bt_on_shown, 
			        EMbmBtuiQgn_prop_set_apps_bt_on_shown_mask );
				break;
			case EBTOnHidden:
				icon = AknsUtils::CreateGulIconL(
			        AknsUtils::SkinInstance(), 
			        KAknsIIDQgnPropSetAppsBtOnHidden,  
			        bmpFilename, 
			        EMbmBtuiQgn_prop_set_apps_bt_on_hidden, 
			        EMbmBtuiQgn_prop_set_apps_bt_on_hidden_mask );
				break;
			case EBTConnectedShown:
				icon = AknsUtils::CreateGulIconL(
			        AknsUtils::SkinInstance(), 
			        KAknsIIDQgnPropSetAppsBtConnShown,  
			        bmpFilename, 
			        EMbmBtuiQgn_prop_set_apps_bt_conn_shown, 
			        EMbmBtuiQgn_prop_set_apps_bt_conn_shown_mask );
				break;
			case EBTConnectedHidden:
				icon = AknsUtils::CreateGulIconL(
			        AknsUtils::SkinInstance(), 
			        KAknsIIDQgnPropSetAppsBtConnHidden,  
			        bmpFilename, 
			        EMbmBtuiQgn_prop_set_apps_bt_conn_hidden, 
			        EMbmBtuiQgn_prop_set_apps_bt_conn_hidden_mask );
				break;
			}	        
    	}
    else
	    {
	    icon = CGSPluginInterface::CreateIconL( aIconType );	
	    }
    	            
    return icon;
	}    


// ---------------------------------------------------------------------------
// CBTUIMainView::GetValue()
// Get 2nd row text for parent Connectivity View.
// ---------------------------------------------------------------------------
//  
void CBTUIMainView::GetValue( const TGSPluginValueKeys aKey, TDes& aValue )
	{
	TRACE_FUNC_ENTRY
	
	TRAP_IGNORE( GetValueL(aKey, aValue) );
	}

// ---------------------------------------------------------------------------
// CBTUIMainView::GetValueL()
// ---------------------------------------------------------------------------
// 	
void CBTUIMainView::GetValueL( const TGSPluginValueKeys aKey, TDes& aValue )
	{
	if(aKey == EGSPluginKeySettingsItemValueString)
		{
		HBufC* secondary = NULL; 
		RefreshSettingsStatusL();
		switch(iBtState)
			{
			case EBTOff:
				secondary = StringLoader::LoadL( R_CP_DETAIL_BT_OFF );
				break;
			case EBTOnShown:
				secondary = StringLoader::LoadL( R_CP_DETAIL_BT_ON_SHOWN );
				break;
			case EBTOnHidden:
				secondary = StringLoader::LoadL( R_CP_DETAIL_BT_ON_HIDDEN );
				break;
			case EBTConnectedShown:
				secondary = StringLoader::LoadL( R_CP_DETAIL_BT_CONN_SHOWN );
				break;
			case EBTConnectedHidden:
				secondary = StringLoader::LoadL( R_CP_DETAIL_BT_CONN_HIDDEN );
				break;
			}
		aValue.Copy( *secondary );
		delete secondary;	
		}
	}

// ---------------------------------------------------------------------------
// CBTUIMainView::CreateTabIconL()
// Creates icon for tab.
// ---------------------------------------------------------------------------
//    
CGulIcon* CBTUIMainView::CreateTabIconL()
    {
    TFileName bmpFilename; 
    bmpFilename += KBitmapFileDrive;
    bmpFilename += KDC_APP_BITMAP_DIR;
    bmpFilename += KBtuiBmpFileName;
    
    CGulIcon* icon = AknsUtils::CreateGulIconL(
        AknsUtils::SkinInstance(), 
        KAknsIIDQgnPropBtSetTab3,  
        bmpFilename, 
        EMbmBtuiQgn_prop_bt_set_tab3, 
        EMbmBtuiQgn_prop_bt_set_tab3_mask );

    return icon;
    }       

// ---------------------------------------------------------------------------
// CBTUIMainView::RefreshSettingsStatusL()
// Get the current BT settings: power, visibility and connection.
// ---------------------------------------------------------------------------
//    
void CBTUIMainView::RefreshSettingsStatusL()
	{
		
	if ( BluetoothFeatures::EnterpriseEnablementL() == BluetoothFeatures::EDisabled ) 
		{
		iBtState = EBTOff;	
		}
	else
		{
		TBTPowerStateValue powerState;
		iBtEngSettings->GetPowerState(powerState);
		
		TBTVisibilityMode currentMode;
		iBtEngSettings->GetVisibilityMode(currentMode);
		
		TBool connected = TBTUIViewsCommonUtils::IsAnyDeviceConnectedL();
			
		if(powerState == EBTPowerOff)
			iBtState = EBTOff;	
		else 
			{
			if( !connected && (currentMode == EBTVisibilityModeGeneral || 
								currentMode == EBTVisibilityModeTemporary))	
				iBtState = EBTOnShown;
			
			else if(!connected && currentMode == EBTVisibilityModeHidden)	
				iBtState = EBTOnHidden;
			
			else if(connected && (currentMode == EBTVisibilityModeGeneral || 
								currentMode == EBTVisibilityModeTemporary))	
				iBtState = EBTConnectedShown;
			
			else if(connected && currentMode == EBTVisibilityModeHidden)
				iBtState = EBTConnectedHidden;
			}	
		}
	}
	
// ----------------------------------------------------
// CBTUIMainView::UpdateParentView
//
// Update Connectivity View when it's the current  
// active view and the key watched is changed.
// ----------------------------------------------------
void CBTUIMainView::UpdateParentView()
	{
	if(iConstructAsGsPlugin)
		{
		CGSParentPlugin* parent = static_cast<CGSParentPlugin*>(AppUi()->View( KGSConPluginUid ));
		if( parent )
			{
			parent->UpdateView();
			}		
		}
	}
	    
// ---------------------------------------------------------------------------
// CBTUIMainView::NewContainerL
// Creates new iContainer.
// ---------------------------------------------------------------------------
//
void CBTUIMainView::NewContainerL()
    {
	//As a transparent view, do nothing here.
    }

// ---------------------------------------------------------------------------
// CBTUIMainView::HandleListBoxSelectionL
// Handle any user actions while in the list view.
// ---------------------------------------------------------------------------
//
void CBTUIMainView::HandleListBoxSelectionL() 
    {
	//As a base-view, it's transparent, so do nothing here.
    }
    
// ---------------------------------------------------------------------------
// CBTUIMainView::GetParentView
// ---------------------------------------------------------------------------
//
TVwsViewId CBTUIMainView::GetParentView()
    {
    return iParentView; 
    } 

// ---------------------------------------------------------------------------
// CBTUIMainView::TabbedViews()
// Returns pointer to tabbed views of application.
// ---------------------------------------------------------------------------
//
CArrayPtrFlat<MGSTabbedView>* CBTUIMainView::TabbedViews()
    {
    return iViewArray;
    }
    

// ---------------------------------------------------------------------------
// CBTUIMainView::IsLocalNameModified
// Checks from central repository whether the Bluetooth friendly name 
// has been modified .
// ---------------------------------------------------------------------------
//
TInt CBTUIMainView::IsLocalNameModifiedL( TBool& aStatus )
    {
    TRACE_FUNC_ENTRY

    aStatus = (TBool) EBTLocalNameDefault;
    
    TInt val = 0;
	CRepository* cenRep = NULL;
    TInt err = KErrNone;
    TRAP_IGNORE(cenRep = CRepository::NewL( KCRUidBTEngPrivateSettings ));
    
    err = cenRep->Get( KBTLocalNameChanged, val );
    
    if ( val != 0 )
    	{
    	aStatus = (TBool) EBTLocalNameSet;
    	}
    delete cenRep;
    cenRep = NULL;
    TRACE_FUNC_EXIT
    return err;
    }

