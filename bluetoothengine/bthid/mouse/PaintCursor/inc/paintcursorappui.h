/*
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: BT Hid Simulator Application UI Header File
*
*/


#ifndef PaintCursorAPPUI_H
#define PaintCursorAPPUI_H

#include <eikapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknappui.h>
#include <eikdialg.h>
#include <flogger.h>
#include "bthidpskeywatcher.h"

class CPaintCursorAppView;

/**
 * Application UI class.
 * Provides support for the following features:
 * - dialog architecture
 * 
 */
class CPaintCursorAppUi : public CAknAppUi, MMouseCursorStatusObserver
    {
public:
    // // Constructors and destructor

    /**
     * EPOC default constructor.
     */
    void ConstructL();

    /**
     * Destructor.
     */
    ~CPaintCursorAppUi();

    //void CenRepDataChanged(TUid& aUid, TUint32 aKey);
    void MouseCursorStatusChangedL(TInt aStatus);

private:
    // From MEikMenuObserver
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

private:
    /**
     * From CEikAppUi, takes care of command handling.
     * @param aCommand command to be handled
     */
    void HandleCommandL(TInt aCommand);

    /**
     * From CEikAppUi, handles key events.
     * @param aKeyEvent Event to handled.
     * @param aType Type of the key event. 
     * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
     */
    virtual TKeyResponse HandleKeyEventL(const TKeyEvent& aKeyEvent,
            TEventCode aType);

    void SendToBackground();

    void EndTask();
    
    void HandleForegroundEventL(TBool aForeground);

private:
    CPaintCursorAppView* iAppView;
    CBTMouseCursorStatusObserver* iPsKeyWatcher;
    };

#endif

