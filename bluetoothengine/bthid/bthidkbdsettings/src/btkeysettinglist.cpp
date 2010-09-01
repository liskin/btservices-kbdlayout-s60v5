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


#include "bthidsettingview.h"
#include "bthidsettings.h"
#include "bthidsettinglist.h"
#include "btkeyenumtextsettingitem.h"
#include "btkeyboard.hrh"

//-----------------------------------------------------------------------------
// CBTKeySettingList::CBTKeySettingList
//-----------------------------------------------------------------------------
//
CBTKeySettingList::CBTKeySettingList(CBtHidSettings& aSettings, CBtHidSettingView* aBtHidSettingView):
    CAknSettingItemList(), iSettings(aSettings), iBtHidSettingView(aBtHidSettingView)
    {
    }

//-----------------------------------------------------------------------------
// CBTKeySettingList::CreateSettingItemL
//-----------------------------------------------------------------------------
//
CAknSettingItem* CBTKeySettingList::CreateSettingItemL( TInt aIdentifier )
    {
    CAknSettingItem* settingItem = 0;
    switch ( aIdentifier )
        {
        case EBTKeySettingLayout:
            iLayoutCode = iSettings.LoadLayoutSetting();
            settingItem = new (ELeave) CBTKeyEnumTextSettingItem(
                                            aIdentifier, iLayoutCode, iSettings, iBtHidSettingView);
            break;      
        default:
            break;
        }

    return settingItem;
    }

//-----------------------------------------------------------------------------
// CBTKeySettingList::HandleListBoxEventL
//-----------------------------------------------------------------------------
//
void CBTKeySettingList::HandleListBoxEventL(
            CEikListBox* aListBox, TListBoxEvent aEventType)
    {
        // call the base classes handler to handle this event.
    CAknSettingItemList::HandleListBoxEventL(aListBox, aEventType);
    
    // store the values from all setting items to their external variables
    StoreSettingsL();
    }

//-----------------------------------------------------------------------------
// CBTKeySettingList::SizeChanged
//-----------------------------------------------------------------------------
//
void CBTKeySettingList::SizeChanged()
    {
    if ( ListBox() ) 
        {
        ListBox()->SetRect(Rect());
        }
    }

//-----------------------------------------------------------------------------
// CBTKeySettingList::EditCurrentItemL
//-----------------------------------------------------------------------------
//
void CBTKeySettingList::EditCurrentItemL( TInt aCommand )
    {
    if ( ListBox() )
        {
        TInt index = ListBox()->CurrentItemIndex();
        EditItemL(index, EBTKeyCmdAppChange == aCommand);
        StoreSettingsL();        
        }
    }    
//End of file   
