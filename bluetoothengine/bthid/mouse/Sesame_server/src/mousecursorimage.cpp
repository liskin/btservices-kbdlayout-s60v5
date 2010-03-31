/*
* Copyright (c) 2004 - 2006 Nokia Corporation and/or its subsidiary(-ies).
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


// INCLUDE FILES
#include <e32std.h>
#include <e32base.h>
#include <txtfrmat.h>

#include "mousecursorimage.h"
#include "animation.pan"


#ifndef DBG
#ifdef _DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif
#endif

// ============================ MEMBER FUNCTIONS ===============================

// Animation server object class functions.
// These functions are called by the windows server from
// requests made by the client animation dll.

// -----------------------------------------------------------------------------
// CImage::CImage()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CImage::CImage()
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CImage::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CImage::ConstructL( TAny* /* aArgs */ )
    {
    DBG(RDebug::Print(_L(" CImage::ConstructL")));
    // Initialise starting points to the top left hand corner
    iFunctions->GetRawEvents( EFalse );

    TSpriteMember* spriteMember = iSpriteFunctions->GetSpriteMember(0);

    DBG(RDebug::Print(_L(" CImage::ConstructL mainbitmap")));
    if (spriteMember == NULL)
        {
        DBG(RDebug::Print(_L(" CImage::ConstructL No member !!!!")));
        User::Leave(KErrNotReady);
        }

    iBitmapDevice = CFbsBitmapDevice::NewL(spriteMember->iBitmap);

    DBG(RDebug::Print(_L(" CImage::ConstructL maskbitmap")));

    iBitmapMaskDevice = CFbsBitmapDevice::NewL(spriteMember->iMaskBitmap);
    iSpriteGc = CFbsBitGc::NewL();

    DBG(RDebug::Print(_L(" CImage::ConstructL DuplicateBitmaps")));

    iCursorBitmap = iFunctions->DuplicateBitmapL(spriteMember->iBitmap->Handle());
    iCursorBitmapMask = iFunctions->DuplicateBitmapL(spriteMember->iMaskBitmap->Handle());

    iSpriteGc->Reset();
    iSpriteFunctions->SetPosition(TPoint());
    iSpriteFunctions->SizeChangedL();

    // Set the screen visible
    // We are using a timer, not the built in synchronising, so turn it off
    iFunctions->SetSync( MAnimGeneralFunctions::ESyncNone );

    DBG(RDebug::Print(_L(" CImage::ConstructL Done")));
    }


// -----------------------------------------------------------------------------
// CPointerAnim::OfferRawEvent(const TRawEvent& aRawEvent)
// -----------------------------------------------------------------------------
TBool CImage::OfferRawEvent(const TRawEvent& aRawEvent)
    {
    (void) aRawEvent;
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CImage::~CImage()
// Destructor.
// -----------------------------------------------------------------------------
//
CImage::~CImage()
    {
    delete iBitmapDevice;
    delete iBitmapMaskDevice;
    delete iSpriteGc;
    }

// -----------------------------------------------------------------------------
// CImage::FocusChanged()
// Not bothered if the focus has changed.
// -----------------------------------------------------------------------------
//
void CImage::FocusChanged( TBool /* aState */ )
    {}

// -----------------------------------------------------------------------------
// CImage::Animate()
// Animate the animation server object.
// -----------------------------------------------------------------------------
//
void CImage::Animate( TDateTime* /* aDateTime */ )
    {
    }

// -----------------------------------------------------------------------------
// CImage::CommandReplyL()
// Function to be used for nonbuffered commands. Not used in this example.
// -----------------------------------------------------------------------------
//
TInt CImage::CommandReplyL( TInt aOpcode, TAny* /* aArgs */ )
    {
    return aOpcode;
    }

void CImage::DrawCursor(  )
    {
    DBG(RDebug::Print(_L(" CImage::DrawCursor Start")));

    iSpriteGc->SetPenStyle(CGraphicsContext::ESolidPen);
    iSpriteGc->SetBrushStyle(CGraphicsContext::ESolidBrush);
    iSpriteGc->SetBrushColor(TRgb(255,255,255));
    iSpriteGc->SetPenColor(TRgb(255,255,255));

    if ( (iBitmapDevice) && (iBitmapMaskDevice) )
        {
        DBG(RDebug::Print(_L(" CImage::DrawCursor iBitmapMaskDevice")));
        // Draw mask
        iSpriteGc->Activate(iBitmapMaskDevice);
        iSpriteGc->BitBlt(TPoint(0,0), iCursorBitmapMask);

        DBG(RDebug::Print(_L(" CImage::DrawCursor iBitmapMaskDevice")));

        // Draw bitmap
        iSpriteGc->Activate(iBitmapDevice);
        iSpriteGc->BitBlt(TPoint(0,0), iCursorBitmap);
        }
    DBG(RDebug::Print(_L(" CImage::DrawCursor END")));
    }

// -----------------------------------------------------------------------------
// CImage::Command()
// Function for buffered commands and commands that cannot fail/leave.
// -----------------------------------------------------------------------------
//
void CImage::Command( TInt aOpcode, TAny* aArgs )
    {
    DBG(RDebug::Print(_L("CImage::Command %d"), aOpcode ));

    switch ( aOpcode )
        {
        case KStartBTCursorAnim:
            {
            iSpriteFunctions->Activate(ETrue);
            DrawCursor();
            }
        break;

        case KStopBTCursorAnim:
            {
            iSpriteFunctions->Activate(EFalse);
            }
        break;
        
        case KRedrawBTCursorAnim:
            {
            iSpriteFunctions->Activate(EFalse);
            iSpriteGc->Reset();
            iSpriteFunctions->SetPosition(TPoint());
            iSpriteFunctions->SizeChangedL();
            iSpriteFunctions->Activate(ETrue);
            DrawCursor();
            }
        break;
        
        case KChangeCursor:
            {
            TPoint pos = *(TPoint *)aArgs;
            iSpriteFunctions->SetPosition(pos);
            }
        break;
        case KSendRawEvent:
            {
            TRawEvent rawEvent = *(TRawEvent *)aArgs;
            iSpriteFunctions->SetPosition(rawEvent.Pos());
            iFunctions->PostRawEvent( rawEvent );
            }
        break;
        default:
        User::Panic( KAnimation, EPanicAnimationServer );
        break;
        }
    }


// End of File
