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
* Description:  Declares main application class.
 *
*/


#ifndef __MAPPING_H
#define __MAPPING_H

#include <e32std.h>

// ----------------------------------------------------------------------

class TFindLayoutId
    {
public:
    TFindLayoutId(TInt aHidCountryId, TInt aVendorId, TInt aProductCode);

    inline TInt LayoutId() const;
    inline TBool IsNokiaSu8() const;
    inline TBool FoundLayout() const;

private:
    static TInt LanguageToLayout(TLanguage aLanguage, TBool aIsNokiaSu8);
    static TInt CountryToLayout(TInt aHidCountryId, TBool aIsNokiaSu8);

private:
    static const TInt KDefaultLayoutId;

    struct TCountryMapEntry
        {
        TInt iHidCountryId;
        TInt iLayoutId;
        };

    static const TCountryMapEntry KMapCountryToLayout[];
    static const TInt KCountryMapSize;

    static const TCountryMapEntry KMapSu8CountryToLayout[];
    static const TInt KSu8CountryMapSize;

    struct TLanguageMapEntry
        {
        TLanguage iLanguage;
        TInt iLayoutId;
        TInt iSu8LayoutId;
        };

    static const TLanguageMapEntry KMapLanguageToLayout[];
    static const TInt KLanguageMapSize;

private:
    TInt iLayout;

    TBool iIsNokiaSu8;
    TBool iFoundLayout;
    };

// ----------------------------------------------------------------------

inline TBool TFindLayoutId::IsNokiaSu8() const
    {
    return iIsNokiaSu8;
    }

inline TBool TFindLayoutId::FoundLayout() const
    {
    return iFoundLayout;
    }

inline TInt TFindLayoutId::LayoutId() const
    {
    return iLayout;
    }

// ----------------------------------------------------------------------

#endif

