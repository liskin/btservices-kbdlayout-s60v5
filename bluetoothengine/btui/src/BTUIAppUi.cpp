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
* Description:   This class the care of opening and closing the views.
 *
*/


// INCLUDE FILES
#include "BTUIDocument.h"
#include <aknview.h>
#include "BTUIAppUi.h"
#include <avkon.hrh>
#include <aknappui.h>
#include "BtuiPluginInterface.h"
#include "btui.h"
#include "BTUIMainView.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTUIAppUi::ConstructL
//
// Symbian 2nd phase constructor can leave. 
// Contains status pane, navi pane, tab group and view 
// creations and setting the main view active.
// ----------------------------------------------------------
//
void CBTUIAppUi::ConstructL()
    {
    TRACE_FUNC_ENTRY

    BaseConstructL( EAknEnableSkin | EAknEnableMSK | EAknSingleClickCompatible);

    // Show tabs for main views from resources
    CEikStatusPane* sp = StatusPane();

    // Fetch pointer to the default navi pane control
    iNaviPane = (CAknNavigationControlContainer*)sp->ControlL(TUid::Uid(EEikStatusPaneUidNavi));

    // Tabgroup has been read from resource and it were pushed to the navi pane.
    // Get pointer to the navigation decorator and set this to be a MAknTabObserver    
    iDecoratedTabGroup = iNaviPane->ResourceDecorator();

    if (iDecoratedTabGroup)
        {
        iTabGroup = (CAknTabGroup*) iDecoratedTabGroup->DecoratedControl();
        iTabGroup->SetObserver( this );
        }    
        
    // -----------------------------------------------------------------------    
    // Construct Views by calling ECOM interface CBtuiPluginInterface's 
    // factory function NewL(), which in turn will call instantiation funtion
    // of the concrate class that implement this interface.
    // -----------------------------------------------------------------------
        
	// 1.Construct Mainview 
	// 
    CBtuiPluginInterface* mainView = CBtuiPluginInterface::NewL(KBtuiPluginUidMainView,(MBtuiPluginViewActivationObserver*) this );
   		
    CleanupStack::PushL( mainView );
    AddViewL( mainView ); // Transfer ownership to CAknViewAppUi
    CleanupStack::Pop(mainView );    

	// 2.Construct OtherDeviceView
	//
    CBtuiPluginInterface* otherDeviceView = CBtuiPluginInterface::NewL(
    	KBtuiPluginUidPairedDevicesView, (MBtuiPluginViewActivationObserver*) this );	

    CleanupStack::PushL( otherDeviceView );
    AddViewL( otherDeviceView ); // Transfer ownership to CAknViewAppUi
    CleanupStack::Pop(otherDeviceView );

	// 3.Construct BlockDeviceView
	//
    CBtuiPluginInterface* blockDeviceView = CBtuiPluginInterface::NewL(
    	KBtuiPluginUidBlockedDevicesView , (MBtuiPluginViewActivationObserver*) this );	

    CleanupStack::PushL( blockDeviceView );
    AddViewL( blockDeviceView ); // Transfer ownership to CAknViewAppUi
    CleanupStack::Pop(blockDeviceView );

    //ActivateLocalViewL( mainView->Id() ); 
    TRACE_FUNC_EXIT
    }

// ----------------------------------------------------
// CBTUIAppUi::~CBTUIAppUi
// ----------------------------------------------------
//
CBTUIAppUi::~CBTUIAppUi() 
    {
    TRACE_FUNC_ENTRY
    if(iNaviPane != NULL && iDecoratedTabGroup != NULL)
    	{
    
  		delete iDecoratedTabGroup; // Tabgroup was read from resource.
  		iDecoratedTabGroup=NULL;
    	}
  	
	REComSession::FinalClose();
  	
    TRACE_FUNC_EXIT
    }


// ------------------------------------------------------------------------------
// CBTUIAppUi::DynInitMenuPaneL
//
// This function is called by the UIKON framework just before it displays
// a menu pane. Its default implementation is empty, and by overriding it,
// the application can set the state of menu items dynamically according
// to the state of application data.
// ------------------------------------------------------------------------------
//
void CBTUIAppUi::DynInitMenuPaneL( TInt /* aResourceId */, CEikMenuPane* /* aMenuPane */ )
    {
    }

// ----------------------------------------------------
// CBTUIAppUi::HandleKeyEventL
// ----------------------------------------------------
//

TKeyResponse CBTUIAppUi::HandleKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TRACE_FUNC

    iTabGroup = (CAknTabGroup*)iDecoratedTabGroup->DecoratedControl();
    
	//  Event will not be handled, if:
	//  1.It is not a key event.  
	//  2.No tab group be created yet.
	//  3.Device's BT name hasn't been initialized by user yet.   
	//
    if (aType != EEventKey || !iTabGroup ) 
        {
        return EKeyWasNotConsumed;
        }

    return 	iTabGroup->OfferKeyEventL(aKeyEvent, aType);
    }

// ----------------------------------------------------
//  CBTUIAppUi::HandleCommandL
//
// Handle user commands. All commands are handled via
// this routine.
// ----------------------------------------------------
//
void CBTUIAppUi::HandleCommandL(TInt aCommand)
    {
    TRACE_INFO((_L("command = %d"), aCommand ))

    switch ( aCommand )
        {
        case EAknCmdExit:
        case EEikCmdExit:
            {
            Exit();
            break;
            }
        default:
            {
            break;
            }
        }
    }
// ----------------------------------------------------
// CBTUIAppUi::PluginViewActivated
// ----------------------------------------------------
//   
void CBTUIAppUi::PluginViewActivated(TPluginViewId aViewId)

    {
    TRACE_INFO((_L("view id = %d"), aViewId ))        
    if(iTabGroup->ActiveTabId()!=aViewId)
        {            
            iTabGroup->SetActiveTabById(aViewId);
        }
    }

// ----------------------------------------------------
// CBTUIAppUi::TabChangedL
// ----------------------------------------------------
//
void CBTUIAppUi::TabChangedL(TInt aIndex)
    {
    TRACE_FUNC
    switch (aIndex)
    	{
    	case 0:
		    ActivateLocalViewL(KBtuiPluginUidMainView);
			break;
    	case 1:
		    ActivateLocalViewL(KBtuiPluginUidPairedDevicesView);    	
			break;    	
    	case 2:
		    ActivateLocalViewL(KBtuiPluginUidBlockedDevicesView);    	
			break;    	
    	}
    }
    
