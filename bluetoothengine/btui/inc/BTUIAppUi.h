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
* Description:
*
*/


#ifndef BTUIAPPUI_H
#define BTUIAPPUI_H

// INCLUDES
#include <aknapp.h>         // AVKON components
#include <aknViewAppUi.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <AknTabObserver.h>
#include "BtuiPluginInterface.h"

// CLASS DECLARATION

/**
*  This class is a base class mandatory for all Symbian OS UI applications.
*/
class CBTUIAppUi : public CAknViewAppUi, 
                   public MAknTabObserver,
                   public MBtuiPluginViewActivationObserver
    {
    public: // Constructors and destructor

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

        /**
        * Destructor
        */
        virtual ~CBTUIAppUi();

       
    public: // Functions from base classes

        /**
        * From CAknViewAppUi Handles user commands.
        * @param aCommand A command id.        
        * @return None.
        */
        void HandleCommandL(TInt aCommand);

    private: // Functions from base classes

        /**
        * From CAknViewAppUi Initializes menu pane dynamically.
        * @param aResourceId Resource ID identifying the menu pane to initialise.
        * @param aMenuPane The in-memory representation of the menu pane.
        * @return None.
        */
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);
        
        /**
        * From CAknViewAppUi Handles key events.
        * @param aKeyEvent The key event that occurred.
        * @param aType The event type.
        * @return Response to the key event.
        */
        virtual TKeyResponse HandleKeyEventL( const TKeyEvent& aKeyEvent,TEventCode aType );
        
        /**
        * From MBtuiPluginViewActivationObserver
        * @param aViewId  activated plugin id
        * @return none
        */
        void PluginViewActivated(TPluginViewId aViewId);  

        /**
        * From MAknTabObserver Takes care of tab handling.
        * @param aIndex tab to be handled
        */
        virtual void TabChangedL(TInt aIndex); 

    private: // Data

		CAknNavigationControlContainer* iNaviPane;          // pointer to the navi pane control
        CAknTabGroup*                   iTabGroup;          // Tab group
        CAknNavigationDecorator*        iDecoratedTabGroup; // Frame for tabgroup                               
    };

#endif

