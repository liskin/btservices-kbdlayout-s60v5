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
* Description:
*
*/


#include <avkon.hrh>
#include <eikmenup.h>
#include <e32property.h>
#include <paintcursor.rsg>
#include <apgwgnam.h>

#include "paintcursorapp.h"
#include "paintcursorappui.h"
#include "paintcursorapp.h"
#include "paintcursorappview.h"
#include "debug.h"
#include "bthidPsKey.h"

void CPaintCursorAppUi::ConstructL()
    {
    TRACE_INFO(_L("[BTHID]\t CPaintCursorAppUi::ConstructL()"));
    BaseConstructL();

    iAppView = CPaintCursorAppView::NewL(ClientRect());

    iAppView->SetMopParent(this);   //  so view can update scroll bars

    AddToStackL(iAppView);

    
    iPsKeyWatcher = CBTMouseCursorStatusObserver::NewL();
    
    iPsKeyWatcher->SubscribeMouseCursorStatusL(this);
    }

CPaintCursorAppUi::~CPaintCursorAppUi()
    {

    if (iAppView)
        {
        RemoveFromStack(iAppView);

        delete iAppView;
        iAppView = 0;
        }

    if (iPsKeyWatcher)
        delete iPsKeyWatcher;
    }



// ------------------------------------------------------------------------------
// CPaintCursorAppUi::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CPaintCursorAppUi::DynInitMenuPaneL(
    TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
    {}

TKeyResponse CPaintCursorAppUi::HandleKeyEventL(
    const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
    {
    return EKeyWasNotConsumed;
    }

void CPaintCursorAppUi::HandleCommandL(TInt aCommand)
    {
    TRACE_INFO((_L("[BTHID]\tCPaintCursorAppUi::HandleCommandL(%d)"),aCommand));
    switch ( aCommand )
        {
        case EEikCmdExit :
        case EAknSoftkeyExit:
            {
            Exit();
            break;
            }

        case EAknSoftkeyBack:
            {
            SendToBackground();
            break;
            }

        default:
        break;
        }
    }

void CPaintCursorAppUi::SendToBackground()
    {
    // Construct en empty TApaTask object
    // giving it a reference to the Window Server session
    TApaTask task(CEikonEnv::Static()->WsSession( ));

    // Initialise the object with the window group id of
    // our application (so that it represent our app)
    task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());

    // Request window server to bring our application
    // to background
    task.SendToBackground();
    }

void CPaintCursorAppUi::MouseCursorStatusChangedL(TInt aStatus)
    {
    TRACE_INFO((_L("[BTHID]\tCPaintCursorAppUi::MouseCursorStatusChangedL(%d)"),aStatus));
    switch (aStatus)
        {
        case ECursorShow:
            {
            iAppView->ShowCursor();
            break; 
            }
        case ECursorHide:
            {
            iAppView->HideCursor();
            break;
            }
        case ECursorNotInitialized:
            {
            iAppView->HideCursor();
            EndTask();
            break;
            }
        case ECursorRedraw:
            {
            iAppView->RedrawCursor();
            break;
            }
        case ECursorReset:
            {
            iAppView->ResetCursor();
            break;
            }
        default:
            {
            break;
            }
        }
    }


void CPaintCursorAppUi::EndTask()
    {
    // Construct en empty TApaTask object
    // giving it a reference to the Window Server session
    TApaTask task(CEikonEnv::Static()->WsSession( ));

    // Initialise the object with the window group id of
    // our application (so that it represent our app)
    task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());

    // Request window server to end our application
    task.EndTask();
    }

void CPaintCursorAppUi::HandleForegroundEventL(TBool aForeground)
    {
    if(aForeground)
        {
        TInt wgId = iEikonEnv->RootWin().Identifier();

        TApaTask self( iCoeEnv->WsSession() );

        self.SetWgId( wgId );
        self.SendToBackground();

        RWsSession session = iEikonEnv->WsSession();

        CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC(session, wgId);

        wgName->SetHidden(ETrue);

        wgName->SetWindowGroupName(iEikonEnv->RootWin());

        CleanupStack::PopAndDestroy(); // wgName
        }
    }

