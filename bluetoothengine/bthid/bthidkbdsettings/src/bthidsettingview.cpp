/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the implementation of setting view class
*
*/

#include <aknViewAppUi.h>
#include <avkon.hrh>
#include <eiklabel.h>
#include <akntitle.h>
#include <StringLoader.h>
#include <hlplch.h>                   // Help launcher
#include <aknradiobuttonsettingpage.h>
#include <bautils.h>
#include <BthidResource.rsg>
#include <csxhelp/bt.hlp.hrh>           // Help IDs
#include <aknnavide.h>
#include <akncontext.h>
#include "bthidsettingview.h"
#include "bthidsettingcontainer.h"
#include "bthidsettings.h"
#include "bthidsettinglist.h"
#include "btkeyboard.hrh"
#include <btkeyboard.mbg>
#include "btengprivatecrkeys.h"
#include "layoutmgr.h"
#include "hidlayoutids.h"
#include "debug.h"

const TUid KBtHidSettingViewId =
    {
        0x2001E30B
    }
    ; // UID of HID device setting view.
// ---------------------------------------------------------
// CBtHidSettingView::NewNewGsPluginL
// Two phased constructor
// ---------------------------------------------------------
//
CBtHidSettingView* CBtHidSettingView::NewBtuiPluginL()
    {
    //TRACE_FUNC(_L("[BTHID]\t CBtHidSettingView::NewBtuiPluginL(), BEGIN"));

    CBtHidSettingView* self = new (ELeave) CBtHidSettingView();

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    //TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]::NewBtuiPluginL, END"),self) );

    return self;
    }


// ----------------------------------------------------
// CBtHidSettingView::CBtHidSettingView
// default constructor
// ----------------------------------------------------
//
CBtHidSettingView::CBtHidSettingView()
    {}

// ----------------------------------------------------
// CBtHidSettingView::ConstructL
// ----------------------------------------------------
//
void CBtHidSettingView::ConstructL()
    {
    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]:ConstructL(), BEGIN"),this) );

    User::LeaveIfError(iLayoutSession.Connect());

    // Check to see what type of layout code the layout manager will return
    // us. (i.e nokia or standard)
    // This is used to know which settings page to display.
    TBool ignore;
    TRACE_INFO(_L("[BTHID]\t CBtHidMainView::ConstructL(), get layout info..."));
    User::LeaveIfError(iLayoutSession.GetDeviceInfo(iIsNokiaKeyboard, ignore));

    iEikEnv = CEikonEnv::Static();
    TFileName filename;
    filename += KFileDrive;
    filename += KDC_RESOURCE_FILES_DIR;
    filename += KResourceFileName;
    BaflUtils::NearestLanguageFile( iEikEnv->FsSession(), filename );
    iResourceFileFlag=iEikEnv->AddResourceFileL(filename);
    BaseConstructL( R_BTKEYBOARD_SETTING_VIEW );

    CEikStatusPane *sp = ((CAknAppUi*)iEikonEnv->EikAppUi())->StatusPane();
    // Fetch pointer to the default title pane control
    iContextPane = (CAknContextPane *)sp->ControlL(TUid::Uid(EEikStatusPaneUidContext));

    iSettings = CBtHidSettings::NewL();

    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]:ConstructL(), END"),this) );
    }
// ----------------------------------------------------
// CBtHidSettingView::~CBtHidSettingView()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CBtHidSettingView::~CBtHidSettingView()
    {
    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]::~CBtHidSettingView()"), this) );
    iLayoutSession.Close();

    delete iNaviDecorator;
    iEikEnv->DeleteResourceFile(iResourceFileFlag);

    if ( iSettingList )
        {
        AppUi()->RemoveFromStack( (CCoeControl*) iSettingList );
        delete iSettingList;
        }
    if ( iContainer )
        {
        AppUi()->RemoveFromStack(iContainer );
        delete iContainer;
        }
    if( iCenRepWatcher )
        delete iCenRepWatcher;

    if (iSettings )
        delete iSettings;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// TUid CBtHidSettingView::Id
// ---------------------------------------------------------
//
TUid CBtHidSettingView::Id() const
    {
    TUid id = KBtHidSettingViewId;    //view id
    return id;
    }

// ----------------------------------------------------
// CBtHidSettingView::HandleCommandL(TInt aCommand)
// takes care of command handling
// ----------------------------------------------------
//
void CBtHidSettingView::HandleCommandL(TInt aCommand)
    {
    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]::HandleCommandL(%d)"),this, aCommand) );
    switch ( aCommand )
        {
        case EEikCmdExit:           // fall-through, handled similarily
        case EAknSoftkeyExit:       // fall-through, handled similarily
        //case EBTKeyCmdExit:
            {
            AppUi()->HandleCommandL( EEikCmdExit);
            }
        case EAknSoftkeyChange:
        case EBTKeyCmdAppChange:
            {
            if( iSettingList )
                {
                iCenRepWatcher->DisableNotifications();
                iSettingList->EditCurrentItemL( aCommand );
                iCenRepWatcher->EnableNotifications();
                }
            break;
            }
        case EAknSoftkeyBack:
            {
            AppUi()->ActivateLocalViewL( iPrevViewId.iViewUid );
            break;
            }

#ifdef __SERIES60_HELP
        case EBTKeyCmdAppHelp:
            {
            HlpLauncher::LaunchHelpApplicationL(
                    iEikEnv->WsSession(), AppUi()->AppHelpContextL());
            break;
            }

#endif
        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// CBtHidSettingView::DoActivateL()
// ---------------------------------------------------------------------------
//
void CBtHidSettingView::DoActivateL( const TVwsViewId& aPrevViewId,
                                     TUid aCustomMessageId,
                                     const TDesC8& aCustomMessage )
    {
    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]::DoActivateL, BEGIN"),this) );
    iPrevViewId = aPrevViewId;
    (void) aCustomMessageId;

    // Setup the title
    HBufC* title;
    title = iCoeEnv->AllocReadResourceLC(R_BTKEYBOARD_SETTINGS_STATUS_PANE_TITLE);

    CEikStatusPane* statusPane = StatusPane();
    CAknTitlePane* titlePane;
    titlePane = (CAknTitlePane*) statusPane->ControlL(TUid::Uid(EEikStatusPaneUidTitle));
    titlePane->SetTextL(title->Des());
    CleanupStack::PopAndDestroy(); // title
    title = NULL;

    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]::DoActivateL(), Creating container..."),this) );
    iContainer = new (ELeave) CBTHidSettingContainer;
    iContainer->SetMopParent(this);
    iContainer->ConstructL( ClientRect());
    iContainer->MakeVisible( ETrue );
    iContainer->ActivateL();
    AppUi()->AddToStackL(iContainer);

    CreateSettingListL();
    iCenRepWatcher = CBtHidCenRepWatcher::NewL( KCRUidBTEngPrivateSettings, *this ); // create listener for cenrep events.
    SetNavipaneTextL(aCustomMessage);

    TRACE_INFO((_L("[BTHID]\t CBtHidSettingView[0x%08x]::DoActivateL, END"),this) );
    }


// ---------------------------------------------------------------------------
// CBtHidSettingView::DoDeactivate
//
// Deactivate the settings view.
// ---------------------------------------------------------------------------
//
void CBtHidSettingView::DoDeactivate()
    {
    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]::DoDeactivate, BEGIN"),this) );
    if ( iSettingList )
        {
        AppUi()->RemoveFromStack( (CCoeControl*) iSettingList );
        delete iSettingList;
        iSettingList = NULL;
        }
    if( iContainer )
        {
        iContainer->MakeVisible(EFalse);
        AppUi()->RemoveFromStack( iContainer );
        delete iContainer;
        iContainer = NULL;
        }

    if (iNaviDecorator)
        {
        delete iNaviDecorator;
        iNaviDecorator = NULL;
        }
    iLastSelectedLayout = iSettings->LoadLayoutSetting();
    iLayoutSession.SetLayout(iLastSelectedLayout);
    delete iCenRepWatcher;
    iCenRepWatcher = NULL;
    TRACE_FUNC_EXIT
    }

// ----------------------------------------------------
// CBtHidSettingView::DynInitMenuPaneL
// ----------------------------------------------------
//
void CBtHidSettingView::DynInitMenuPaneL(
    TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    if (aResourceId == R_BTKEYBOARD_SETTING_MENU )
        {
        aMenuPane->SetItemDimmed(EBTKeyCmdAppChange, ETrue); 
        }
    
    
    }

// ----------------------------------------------------
// CBtHidSettingView::CreateSettingListL
// ----------------------------------------------------
//
void CBtHidSettingView::CreateSettingListL()
    {
    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]::CreateSettingListL, BEGIN"),this) );

    TInt resourceId = R_BTKEYBOARD_STANDARD_SETTING_LIST;

    TBool foundLayout;
    User::LeaveIfError(iLayoutSession.GetDeviceInfo(iIsNokiaKeyboard, foundLayout));

    if(iIsNokiaKeyboard)
        {
        resourceId = R_BTKEYBOARD_NOKIA_SETTING_LIST;
        }
    else
        {
        resourceId = R_BTKEYBOARD_STANDARD_SETTING_LIST;
        }

    TInt layoutCode;
    if ( !foundLayout )
        {
        //No layout found, find an appropriate one.
        layoutCode = CurrentLayoutL();
        SetCurrentLayoutL(static_cast<THidKeyboardLayoutId>(layoutCode));
        }
    else
        {
        //Layout found, load it as default
        iLayoutSession.GetLayout(layoutCode);
        SetCurrentLayoutL(static_cast<THidKeyboardLayoutId>(layoutCode));
        }

    if ( iSettingList )
        {
        AppUi()->RemoveFromStack( (CCoeControl*) iSettingList );
        }
    delete iSettingList;
    iSettingList = NULL;

    iSettingList = new (ELeave) CBTKeySettingList( *iSettings, this);
    iSettingList->SetMopParent(this);
    iSettingList->ConstructFromResourceL(resourceId);
    AppUi()->AddToStackL( (CCoeControl*) iSettingList );
    iSettingList->MakeVisible(ETrue);
    iSettingList->SetRect(ClientRect());
    iSettingList->ActivateL();
    iSettingList->DrawNow();
    TRACE_INFO( (_L("[BTHID]\t CBtHidSettingView[0x%08x]::CreateSettingListL, END"),this) );
    }

// ----------------------------------------------------
// CBtHidSettingView::SetCurrentLayoutL
// ----------------------------------------------------
//
void CBtHidSettingView::SetCurrentLayoutL(THidKeyboardLayoutId aLayoutValue)
    {
    // Ask the layout manager to change layout.
    // Use the leaving version so we the user can't confirm a selection
    // if an error occurs.
    iLayoutSession.SetLayoutL(aLayoutValue);

    iSettings->SaveLayoutSettingL( aLayoutValue );
    }

THidKeyboardLayoutId CBtHidSettingView::CurrentLayoutL() const
    {
    // Ask the layout manager for the current layout.
    // Use the leaving version.
    TInt layoutCode = iLayoutSession.GetLayoutL();

    // Convert the Int to the enum.
    THidKeyboardLayoutId layoutID =
        static_cast<THidKeyboardLayoutId>(layoutCode);

    // If we believe this to be a Nokia keyboard then the layout settings
    // page will contain the list of Nokia Su8 items.
    // Conversly if we think this is a Standard keyboard the layout settings
    // page will contain the list of Standard layouts.
    // We must pass a valid value to the Settings page or it will panic.
    // Validate the code from the Layout Manager before returning it.
    if ( iIsNokiaKeyboard )
        {
        CLayoutUtils::ValidateNokiaSU8Layout(layoutID);
        }
    else
        {
        CLayoutUtils::ValidateStandardLayout(layoutID);
        }

    return layoutID;
    }

void CBtHidSettingView::SetNavipaneTextL(const TDesC8& aDeviceName)
    {
    TRACE_INFO(_L("[BTHID]\t CBtHidSettingView::SetNavipaneTextL Start"));
    CEikStatusPane* sp =
        iEikonEnv->AppUiFactory()->StatusPane();

    CAknNavigationControlContainer* np =
        static_cast<CAknNavigationControlContainer*>(
            sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) );

    delete iNaviDecorator;
    iNaviDecorator = NULL;

    TInt blank(1);
    if (aDeviceName.Length()>0 )
        {
        HBufC* stringHolder = HBufC::NewLC(aDeviceName.Length() + blank);
        stringHolder->Des().Copy(aDeviceName);
        stringHolder->Des().Append(_L(" "));
        iNaviDecorator = np->CreateNavigationLabelL( *stringHolder );
        np->PushL( *iNaviDecorator );
        sp->DrawNow();
        CleanupStack::PopAndDestroy(stringHolder);  // stringHolder
        }
    else
        {
        HBufC* temp = HBufC::NewLC(aDeviceName.Length());
        temp->Des().Copy(aDeviceName);
        iNaviDecorator = np->CreateNavigationLabelL( *temp );
        np->PushL( *iNaviDecorator );
        sp->DrawNow();
        CleanupStack::PopAndDestroy(temp);  // temp
        }
    TRACE_INFO(_L("[BTHID]\t CBtHidSettingView::SetNavipaneTextL End"));
    }
// ----------------------------------------------------
// CBtHidSettingView::CenRepDataChanged
// ----------------------------------------------------
//
void CBtHidSettingView::CenRepDataChanged(TUid& aUid, TUint32 aKey)
    {
    // just recreate the setting list
    TRACE_INFO((_L("[BTHID]\t CBtHidSettingView[0x%08x]::CenRepDataChanged"),this));
    (void) aUid;
    if ( aKey == KBtHidKeyboardLayout )
        {
        TRAPD( err, CreateSettingListL());
        err = err;
        }
    }
// End of File
