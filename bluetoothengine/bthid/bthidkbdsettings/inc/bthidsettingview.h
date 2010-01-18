/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares the setting view for keyboard device.
 *
*/


#ifndef BTHIDSETTINGVIEW_H
#define BTHIDSETTINGVIEW_H

// INCLUDES

#include <aknview.h>        // AVKON components
#include <akncontext.h>
#include <eikmenup.h>       // Menu pane definitions
#include <ConeResLoader.h>
#include <btdevice.h>
#include "BtuiPluginInterface.h"
#include "bthidsettings.h"
#include "bthidsettinglist.h"
#include "bthidcenrepwatcher.h"
// FORWARD DECLARATIONS
class CBTHidSettingContainer;
class CBTKeySettingList;
#include "bthidsettings.h"
#include "layoutmgr.h"
#include "hidlayoutids.h"
// CONSTANTS
_LIT(KFileDrive,"z:");
_LIT(KResourceFileName, "BthidResource.rsc");

// CLASS DECLARATION

/**
 * Main view of the application. Handles view activation, 
 * deactivation, commands and dynamic options menus.
 */
class CBtHidSettingView : public CBtuiPluginInterface,
        public MBtHidCenRepObserver
    {
public:
    // Constructors and destructor

    /**
     * ECOM implementation instantiation function of 
     * interface "CBtuiPluginInterface", to be used by BtuiPairedView.
     */
    static CBtHidSettingView* NewBtuiPluginL( /* TAny* aAppUi */);

    /**
     * Destructor.
     */
    virtual ~CBtHidSettingView();

protected:

    /**
     * C++ default constructor.
     */
    CBtHidSettingView(/* CAknViewAppUi* aAppUi */);

    /**
     * Symbian 2nd phase constructor.
     */
    void ConstructL();

public:
    // Functions from base classes

    /**
     * From CAknView Returns view id.
     * @param None.
     * @return View id.
     */
    TUid Id() const;

    /**
     * From CAknView Handles user commands.
     * @param aCommand A command id.        
     * @return None.
     */
    void HandleCommandL(TInt aCommand);
    void CenRepDataChanged(TUid& aUid, TUint32 aKey);

private:
    // Functions from base classes

    /**
     * From CAknView Activates view.
     * @param aPrevViewId Id of previous view.
     * @param aCustomMessageId Custom message id.
     * @param aCustomMessage Custom message.
     * @return None.
     */
    void DoActivateL(const TVwsViewId& aPrevViewId, TUid aCustomMessageId,
            const TDesC8& aCustomMessage);

    /**
     * From CAknView Deactivates view.        
     * @param None.
     * @return None.
     */
    void DoDeactivate();

    /**
     * From CAknView Dynamically initialises options menu.
     * @param aResourceId Id identifying the menu pane to initialise.
     * @param aMenuPane The in-memory representation of the menu pane.
     * @return None.
     */
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

public:
    // New functions


private:

    /**
     * Creates List Box
     */
    void CreateSettingListL();
    THidKeyboardLayoutId CurrentLayoutL() const;

    void SetNavipaneTextL(const TDesC8& aDeviceName);

public:
    void SetCurrentLayoutL(THidKeyboardLayoutId aLayoutValue);

private:
    // Data
    CBTHidSettingContainer* iContainer;
    CBtHidSettings* iSettings; // Settings handler class
    CBTKeySettingList* iSettingList; // The settings-list list view

    //CIdle*                          iIdle;              	// Idle time class (for doing operation delayed)
    TVwsViewId iPrevViewId; // Previous view of GS app.
    TInt iResourceFileFlag; // Flag for eikon env.
    CEikonEnv* iEikEnv;

    /*! The Layout Manager Server session */
    RLayoutManager iLayoutSession;

    /*! The last keyboard layout selected by the user */
    THidKeyboardLayoutId iLastSelectedLayout;
    TBool iIsNokiaKeyboard;
    /**
     * Needed when creating navipane text
     * Own this pointer
     */
    CAknNavigationDecorator* iNaviDecorator;
    CAknContextPane* iContextPane;
    CBtHidCenRepWatcher* iCenRepWatcher;

    };

#endif // BTHIDSETTINGVIEW_H
// End of File
