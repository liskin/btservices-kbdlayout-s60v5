/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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


#include "btkeyenumtextsettingitem.h"
#include "btkeyboard.hrh"
#include "bthidsettingview.h"
#include "bthidsettings.h"
#include "debug.h"

//-----------------------------------------------------------------------------
// CBTKeyEnumTextSettingItem::CBTKeyEnumTextSettingItem
//-----------------------------------------------------------------------------
//
CBTKeyEnumTextSettingItem::CBTKeyEnumTextSettingItem(
    TInt aResourceId, TInt& aValue, CBtHidSettings& aSettings, CBtHidSettingView* aBtHidSettingView) :
        CAknEnumeratedTextPopupSettingItem ( aResourceId, aValue ),
        iSettings(aSettings),
        iBtHidSettingView(aBtHidSettingView)
    {}


//-----------------------------------------------------------------------------
// CBTKeyEnumTextSettingItem::~CBTKeyEnumTextSettingItem
//-----------------------------------------------------------------------------
//
CBTKeyEnumTextSettingItem::~CBTKeyEnumTextSettingItem()
    {}

//-----------------------------------------------------------------------------
// CBTKeyEnumTextSettingItem::HandleSettingPageEventL
//-----------------------------------------------------------------------------
//
void CBTKeyEnumTextSettingItem::HandleSettingPageEventL(
    CAknSettingPage *aSettingPage, TAknSettingPageEvent aEventType)
    {
    switch( aEventType )
        {
        case EEventSettingOked:
            {
            TRACE_INFO(_L("[BTHID]\t CBTKeyEnumTextSettingItem::HandleSettingPageEventL: OK"));
            // Get the new index and new value
            TInt newIndex = QueryValue()->CurrentValueIndex();
            TInt newValue = EnumeratedTextArray()->At(newIndex)->EnumerationValue();

            // Reset the index to the initial value, so if we leave when
            // trying to use the new value and the user then selects cancel
            // the setting item won't set the new value on the display.
            QueryValue()->SetCurrentValueIndex(IndexFromValue(InternalValue()));

            switch ( Identifier() )
                {
                case EBTKeySettingLayout:
                    {
                    THidKeyboardLayoutId newLayout =
                        static_cast<THidKeyboardLayoutId>(newValue);
                    iSettings.SaveLayoutSettingL(newLayout);
                    iBtHidSettingView->SetCurrentLayoutL(newLayout);
                    }
                break;

                default:
                break;
                }

            // We have finished using the new value. Set the index to the new
            // value so the UI is updated when this function exits
            QueryValue()->SetCurrentValueIndex(newIndex);

            // store the setting page internal value to the external variable
            StoreL();
            break;
            }
        case EEventSettingCancelled:
            {
            TRACE_INFO(_L("[BTHID]\t CBTKeyEnumTextSettingItem::HandleSettingPageEventL: Cancel"));
            // Reset the index to the initial value
            QueryValue()->SetCurrentValueIndex(IndexFromValue(InternalValue()));
            break;
            }
        case EEventSettingChanged:
            {
            TRACE_INFO(_L("[BTHID]\t CBTKeyEnumTextSettingItem::HandleSettingPageEventL: Changed"));
            // Flow through
            }
        default:
            {
            //do nothing
            break;
            }
        }
    // call the base class handler to take care of other event types
    CAknSettingItem::HandleSettingPageEventL(aSettingPage, aEventType);
    }


