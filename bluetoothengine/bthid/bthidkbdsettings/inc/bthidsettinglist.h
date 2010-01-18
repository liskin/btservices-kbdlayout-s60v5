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
* Description:  Declares settings list class.
 *
*/


#ifndef __BTHIDSETTINGLIST_H__
#define __BTHIDSETTINGLIST_H__

#include <aknsettingitemlist.h>

class CBtHidSettings;
class CBtHidSettingView;

class CBTKeySettingList : public CAknSettingItemList
    {
public:
    CBTKeySettingList(CBtHidSettings& aSettings,
            CBtHidSettingView* aBtHidSettingView);

    CAknSettingItem* CreateSettingItemL(TInt identifier);

    // handles the listbox events
    virtual void HandleListBoxEventL(CEikListBox* aListBox,
            TListBoxEvent aEventType);

    void EditCurrentItemL(TInt aCommand);

private:
    void SizeChanged();

private:
    // From CCoeControl

private:
    /*! A reference to the BtHidSettings object */
    CBtHidSettings& iSettings;

    /*! Temporary layout code variable */
    TInt iLayoutCode;

    CBtHidSettingView* iBtHidSettingView;
    };

#endif

