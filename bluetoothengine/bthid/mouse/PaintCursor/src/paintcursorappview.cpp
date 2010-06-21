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

#include <coecntrl.h>
#include <e32property.h> 
#include <bthidPsKey.h>
#include "paintcursorappview.h"
#include "pointmsgqueue.h"
#include "debug.h"


/**  PubSub key read and write policies */
_LIT_SECURITY_POLICY_C2( KBTHIDPSKeyReadPolicy, 
                          ECapabilityLocalServices, ECapabilityReadDeviceData );
_LIT_SECURITY_POLICY_C2( KBTHIDPSKeyWritePolicy, 
                          ECapabilityLocalServices, ECapabilityWriteDeviceData );


CPaintCursorAppView* CPaintCursorAppView::NewL(const TRect& aRect)
    {
    CPaintCursorAppView* self = CPaintCursorAppView::NewLC(aRect);
    CleanupStack::Pop(self);
    return self;
    }

CPaintCursorAppView* CPaintCursorAppView::NewLC(const TRect& aRect)
    {
    CPaintCursorAppView* self = new (ELeave) CPaintCursorAppView;
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    return self;
    }

void CPaintCursorAppView::ConstructL(const TRect& aRect)
    {
    
    User::LeaveIfError( RProperty::Define( KPSUidBthidSrv,
                                            KBTMouseCursorState,
                                            RProperty::EInt,
                                            KBTHIDPSKeyReadPolicy,
                                            KBTHIDPSKeyWritePolicy) );
                                                
    // Create a window for this application view
    CreateWindowL();

    // Set the windows size
    SetRect(aRect);

    // Initialise the RMouseCursorDll class
    TRACE_INFO(_L("[HID]\tCHidMouseDriver::Before SetupMouseCursorDllL()"));
    SetupMouseCursorDllL();
    TRACE_INFO(_L("[HID]\tCHidMouseDriver::After SetupMouseCursorDllL()"));
    // Initialise the RImageCommander class
    SetupImageCommanderL();

    // Activate the window, which makes it ready to be drawn
    ActivateL();

    ShowCursor();
    }

// -----------------------------------------------------------------------------
// CMenuAppUi::SetupMouseCursorDllL()
// Setup Mousr Cursor animation client Dll.
// -----------------------------------------------------------------------------
//

void CPaintCursorAppView::SetupMouseCursorDllL()
    {
    TRACE_INFO(_L("[HID]\tCHidMouseDriver::SetupMouseCursorDllL()"));
    // Create the server dll filename
    TFileName mouseCurorSrv( KMouseCurorSrvName );

    // Load the animation server, if an error occurs then
    // let higher level handle the problem
    User::LeaveIfError( iMouseCursorDll.Load( mouseCurorSrv ) );
    }


// -----------------------------------------------------------------------------
// CMenuAppUi::SetupImageCommanderL()
// Setup the animation image commander.
// -----------------------------------------------------------------------------
//
void CPaintCursorAppView::SetupImageCommanderL()
    {
    TRACE_INFO(_L("[HID]\tCHidMouseDriver::SetupImageCommanderL()"));
    // Tell client to construct a server side object
    TSize iconRect(13,23);
    iClientCommander.ImageConstructL( CCoeEnv::Static()->RootWin(),
                                      iconRect );
    }


CPaintCursorAppView::CPaintCursorAppView() :
        iMouseCursorDll( CCoeEnv::Static()->WsSession() ),
        iClientCommander( iMouseCursorDll, CCoeEnv::Static()->WsSession() )
    {
    // no implementation required
    }


CPaintCursorAppView::~CPaintCursorAppView()
    {
    if ( iMouseInitialized )
        {
        HideCursor();
        }

    // Close the animation object
    iClientCommander.Close ();

    // Close the animation server
    iMouseCursorDll.Close();
    
    RProperty::Delete( KPSUidBthidSrv, KBTMouseCursorState );
    }

void CPaintCursorAppView::SizeChanged()
    {
    DrawNow();
    }

void CPaintCursorAppView::Draw(const TRect& aRect) const
    {
    TRACE_INFO(_L("[PaintCursor]\t CPaintCursorAppView::Draw()"));
    TRgb color;
    
    CWindowGc& gc = SystemGc();

    // draw background
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    
    // get skin text color
    AknsUtils::GetCachedColor( skin, color , KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG6);
    gc.SetPenColor( color );
    
    MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
    AknsDrawUtils::Background( skin, cc, this, gc, aRect );
    }

void CPaintCursorAppView::HideCursor()
    {
    if (iMouseInitialized)
        {
        iClientCommander.ImageCommand( KStopBTCursorAnim );
        }

    iMouseInitialized = EFalse;
    }

void CPaintCursorAppView::ShowCursor()
    {
    if (!iMouseInitialized)
        {
        iClientCommander.ImageCommand( KStartBTCursorAnim );
        }

    iMouseInitialized = ETrue;
    }

void CPaintCursorAppView::RedrawCursor()
    {
    TRect mainPaneRect;
    AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, mainPaneRect );
    SetRect( mainPaneRect );
    DrawNow();
    iClientCommander.ImageCommand( KRedrawBTCursorAnim );
    iMouseInitialized = ETrue;
    }
 
 
void CPaintCursorAppView::ResetCursor()
    {   
    TRect mainPaneRect;
    AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, mainPaneRect );
    SetRect( mainPaneRect );
    DrawNow();
    iClientCommander.ImageCommand( KResetBTCursorAnim );
    iMouseInitialized = ETrue;
    }

void CPaintCursorAppView::HandleResourceChange( TInt aType )
    {
    CCoeControl::HandleResourceChange( aType );
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        ResetCursor();
        }
    }

void CPaintCursorAppView::HandleControlEventL(
    CCoeControl* /*aControl*/, TCoeEvent /*aEventType*/ )
    {
    }


