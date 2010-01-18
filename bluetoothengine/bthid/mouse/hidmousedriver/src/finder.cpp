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
* Description:  This is the implementation of application class
*
*/


#include <e32std.h>
#include <e32svr.h>

#include "finder.h"

#ifndef DBG
#ifdef _DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif
#endif

// ----------------------------------------------------------------------


TMouseFinder::TMouseFinder()
        : iXY(0), iButtons(0), iWheel(0), iAppCollection(0)
    {
    // Nothing else to do
    }

// ----------------------------------------------------------------------

TBool TMouseFinder::BeginCollection(const CCollection *aCollection)
    {
    TBool examineCollection = ETrue;

    if ((aCollection->IsApplication()) && (iAppCollection == 0))
        {
        // Top-level application collection.

        if ((aCollection->UsagePage() == EUsagePageGenericDesktop)
                && (aCollection->Usage() == EGenericDesktopUsageMouse))
            {
            // Collection is a mouse device:
            iAppCollection = aCollection;
            iXY = iWheel = iButtons = 0;
            }
        else
            {
            // Skip other types of top-level application collection:
            examineCollection = EFalse;
            }
        }

    return examineCollection;
    }

TBool TMouseFinder::EndCollection(const CCollection *aCollection)
    {
    TBool continueSearch = ETrue;

    DBG(RDebug::Print(_L("[HID]\tTHeadsetFinder::EndCollection")));
    if (aCollection == iAppCollection)
        {
        // Top-level application(Generic Desktop:Mouse) finished:
        //
        iAppCollection = 0;

        // Stop if we've found a mouse we can use in this
        // application collection:
        //
        continueSearch = !Found();
        }

    return continueSearch;
    }

void TMouseFinder::Field(const CField* aField)
    {
    if (iAppCollection)
        {
        if (IsWheel(aField))
            {
            iWheel = aField;
            }

        if (IsXY(aField))
            {
            iXY = aField;
            }

        if (IsButtons(aField))
            {
            iButtons = aField;
            }
        }
    }

// ----------------------------------------------------------------------


TBool TMouseFinder::IsXY(const CField* aField) const
    {
    TBool found = EFalse;

    if (aField->IsInput() && aField->IsData() &&
            (aField->UsagePage() == EUsagePageGenericDesktop))
        {

        if ( aField->HasUsage(EGenericDesktopUsageX) &&
                aField->HasUsage(EGenericDesktopUsageY) )
            {
            DBG(RDebug::Print(_L("[HID]\tXY field found")));
            found = ETrue;
            }
        }

    return found;
    }

TBool TMouseFinder::IsButtons(const CField* aField) const
    {
    TBool found = EFalse;

    if (aField->IsInput() && aField->IsData() &&
            (aField->UsagePage() == EUsagePageButton))
        {
        const TInt KMinButtons = 1;
        const TInt KMaxButtons = 15;

        // Test for a field containing at least num lock or caps lock
        if ( (aField->UsageMin() >= KMinButtons)
                && (aField->UsageMax() <= KMaxButtons) )
            {
            DBG(RDebug::Print(_L("[HID]\tButtons field found")));
            found = ETrue;
            }
        }

    return found;
    }


TBool TMouseFinder::IsWheel(const CField* aField) const
    {
    TBool found = EFalse;

    if (aField->IsInput() && aField->IsData() &&
            (aField->UsagePage() == EUsagePageGenericDesktop))
        {
        if ( aField->HasUsage( EGenericDesktopUsageWheel ) )
            {
            found = ETrue;
            }
        }

    return found;
    }

// End of File

