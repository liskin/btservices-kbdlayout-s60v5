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
* Description:  This is the implementation of setting container class
*
*/


// INCLUDE FILES
#include <AknUtils.h>  // for font
#include <AknsDrawUtils.h>// skin
#include <AknsBasicBackgroundControlContext.h> //skin
#include <BthidResource.rsg>
#include "bthidsettingcontainer.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBTHidSettingContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CBTHidSettingContainer::ConstructL(const TRect& aRect)
    {
    CreateWindowL(); //makes the control a window-owning control
    BaseConstructL( aRect );
    }

// ---------------------------------------------------------
// CBTHidBaseContainer::BaseConstructL()
// ---------------------------------------------------------
//
void CBTHidSettingContainer::BaseConstructL( const TRect& aRect )
    {
    SetRect(aRect);
    iBackGround = CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, Rect(), EFalse );
    ActivateL();
    }


// Destructor
CBTHidSettingContainer::~CBTHidSettingContainer()
    {
    delete iBackGround;
    }


// ---------------------------------------------------------------------------
// CBTHidSettingContainer::SizeChanged
// called by framwork when the view size is changed
//
// ---------------------------------------------------------------------------
//
void CBTHidSettingContainer::SizeChanged()
    {
    TRACE_INFO( (_L("[BTHID]\t CBTHidBaseContainer::SizeChanged(); Width %d Height %d "),Rect().Height(), Rect().Width()));

    //Handle SizeChanged for Skin
    TRect parentRect(Rect());
    if (iBackGround)
        iBackGround->SetRect(parentRect);
    }

// ---------------------------------------------------------------------------
// CBTHidSettingContainer::CountComponentControls
//
//
// ---------------------------------------------------------------------------
//
TInt CBTHidSettingContainer::CountComponentControls() const
    {
    return 0;
    }

// ---------------------------------------------------------------------------
// CBTHidSettingContainer::ComponentControl
//
//
// ---------------------------------------------------------------------------
//
CCoeControl* CBTHidSettingContainer::ComponentControl(TInt /*aIndex*/) const
    {
    return NULL;
    }

void CBTHidSettingContainer::HandleControlEventL(CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    // TODO: Add your control event handler code here
    }

// ---------------------------------------------------------
// CBTHidBaseContainer::HandleResourceChange
// ---------------------------------------------------------
//

void CBTHidSettingContainer::HandleResourceChange( TInt aType )
    {

    //Handle change in layout orientation
    if (aType == KEikDynamicLayoutVariantSwitch)
        {

        //TRect rect = iAvkonAppUi->ClientRect();
        // The line above provides to big rectangle in the bottom causing cba's overdrawn by by blank area.
        // Correct way to do this is below.
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        DrawNow();
        }

    else
        {
        CCoeControl::HandleResourceChange(aType);
        }
    }

// ---------------------------------------------------------
// CBTHidBaseContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CBTHidSettingContainer::Draw(const TRect& aRect) const
    {
    TRACE_FUNC(_L("[BTHID]\t CBTHidSettingContainer::Draw()"));
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

// ---------------------------------------------------------
// CBTHidBaseContainer::MopSupplyObject()
// Pass skin information if needed.
// ---------------------------------------------------------
//
TTypeUid::Ptr CBTHidSettingContainer::MopSupplyObject(TTypeUid aId)
    {
    if(aId.iUid == MAknsControlContext::ETypeId && iBackGround)
        {
        return MAknsControlContext::SupplyMopObject( aId, iBackGround);
        }

    return CCoeControl::MopSupplyObject( aId );
    }
// End of File
