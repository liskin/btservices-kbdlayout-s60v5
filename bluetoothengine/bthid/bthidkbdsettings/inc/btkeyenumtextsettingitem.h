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


#ifndef __BTKEYENUMTEXTSETTINGITEM_H__
#define __BTKEYENUMTEXTSETTINGITEM_H__

#include <aknsettingitemlist.h>

class CBtHidSettings;
class CBtHidSettingView;

class CBTKeyEnumTextSettingItem : public CAknEnumeratedTextPopupSettingItem
    {
public:

    /*!
     Constructs this object
     */
    CBTKeyEnumTextSettingItem(TInt aResourceId, TInt& aValue,
            CBtHidSettings& aSettings, CBtHidSettingView* aBtHidSettingView);

    /*!
     Destroy the object and release all memory objects
     */
    ~CBTKeyEnumTextSettingItem();

public:
    // from CAknSettingItem
    void HandleSettingPageEventL(CAknSettingPage *aSettingPage,
            TAknSettingPageEvent aEventType);

private:
    CBtHidSettings& iSettings;
    CBtHidSettingView* iBtHidSettingView;
    };

#endif // __BTKEYENUMTEXTSETTINGITEM_H__
