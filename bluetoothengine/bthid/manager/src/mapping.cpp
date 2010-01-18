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

#include "hidlayoutids.h"
#include "mapping.h"
#include "layout.h"
#include "debug.h"

// ----------------------------------------------------------------------

const TInt TFindLayoutId::KDefaultLayoutId = EUnitedKingdom;

// ----------------------------------------------------------------------
// Map a HID country ID to a keyboard layout ID:
//
const TFindLayoutId::TCountryMapEntry TFindLayoutId::KMapCountryToLayout[] =
    {
        // HID ID -> Standard layout ID
        //
        {  2, EBelgian               }, // Belgian
        {  6, EDanish                }, // Danish
        {  7, EFinnishSwedish        }, // Finnish
        {  8, EFrench                }, // French
        {  9, EGerman                }, // German
        { 14, EItalian               }, // Italian
        { 18, EDutch                 }, // Netherlands/Dutch
        { 19, ENorwegian             }, // Norwegian
        { 22, EPortuguese            }, // Portuguese
        { 25, ESpanish               }, // Spanish
        { 26, EFinnishSwedish        }, // Swedish
        { 32, EUnitedKingdom         }, // UK
        { 33, EUnitedStates          }, // US
        //
        // Not in requirements, but layouts do exist. (Note that it
        // doesn't matter if a layout is listed here but isn't installed,
        // as the default layout will be used instead.)
        //
        { 4, ECanadianFrench         }, // Canadian-French
        {  3, ECanadianMultilingual  }, // Canadian-Bilingual
        { 13, EUSInternational       }, // International (ISO)
        { 17, ELatinAmerican         }, // Latin American
        { 27, ESwissFrench           }, // Swiss/French
        { 28, ESwissGerman           }, // Swiss/German
    };

const TInt TFindLayoutId::KCountryMapSize =
    (sizeof (KMapCountryToLayout)) / (sizeof (TCountryMapEntry));

// ----------------------------------------------------------------------
// Map a HID country ID (Nokia SU-8) to a keyboard layout ID:
//
const TFindLayoutId::TCountryMapEntry TFindLayoutId::KMapSu8CountryToLayout[] =
    {
    // HID ID -> Nokia SU-8 layout ID
    //
    // We don't know what language code the SU-8 keyboards will return,
    // so we'll include them all for now:
    //
    { 33, ESu8USEnglish       },  // US
    {  7, ESu8FinnishSwedish  },  // Finnish
    { 26, ESu8FinnishSwedish  },  // Swedish
    {  6, ESu8DanishNorwegian },  // Danish
    { 19, ESu8DanishNorwegian },  // Norwegian
    {  9, ESu8German          }   // German
    };

const TInt TFindLayoutId::KSu8CountryMapSize =
    (sizeof (KMapSu8CountryToLayout)) / (sizeof (TCountryMapEntry));

// ----------------------------------------------------------------------
// Map a TLanguage enum, as returned by User::Language(), to a
// keyboard layout ID:
//
const TFindLayoutId::TLanguageMapEntry TFindLayoutId::KMapLanguageToLayout[] =
    {
        // TLanguage ID           ->  Standard layout ID,  Nokia SU-8 layout ID
        //
        { ELangEnglish,               EUnitedKingdom,      ESu8USEnglish },
        { ELangFrench,                EFrench,             0 },
        { ELangGerman,                EGerman,             ESu8German },
        { ELangSpanish,               ESpanish,            0 },
        { ELangItalian,               EItalian,            0 },
        { ELangSwedish,               EFinnishSwedish,     ESu8FinnishSwedish },
        { ELangDanish,                EDanish,             ESu8DanishNorwegian },
        { ELangNorwegian,             ENorwegian,          ESu8DanishNorwegian },
        { ELangFinnish,               EFinnishSwedish,     ESu8FinnishSwedish },
        { ELangAmerican,              EUnitedStates,       ESu8USEnglish },
        { ELangSwissFrench,           ESwissFrench,        0 },
        { ELangSwissGerman,           ESwissGerman,        0 },
        { ELangPortuguese,            EPortuguese,         0 },
        { ELangIcelandic,             EIcelandic,          0 },
        { ELangDutch,                 EDutch,              0 },
        { ELangBelgianFlemish,        EBelgian,            0 },
        { ELangBelgianFrench,         EBelgian,            0 },
        { ELangInternationalEnglish,  EUSInternational,    ESu8USEnglish },
        { ELangCanadianFrench,        ECanadianFrench,     0 },
        { ELangLatinAmericanSpanish,  ELatinAmerican,      0 },
        { ELangFinlandSwedish,        EFinnishSwedish,     ESu8FinnishSwedish },
        { ELangRussian,				  			ERussian,			   		 ESu8Russian }

        //    	}
    };

const TInt TFindLayoutId::KLanguageMapSize =
    (sizeof (KMapLanguageToLayout)) / (sizeof (TLanguageMapEntry));

// ----------------------------------------------------------------------

TFindLayoutId::TFindLayoutId(TInt aHidCountryId,
                             TInt aVendorId, TInt aProductCode)
    {
    TRACE_INFO( (_L("TFindLayoutId(Country %d, Vendor %d, Product %d)"),
                 aHidCountryId, aVendorId, aProductCode));

    const TInt KNokiaVendorId = 0x5555;
    const TInt KSu8ProductCode = 0x00dd;

    iIsNokiaSu8 = (aVendorId == KNokiaVendorId)
                  && (aProductCode == KSu8ProductCode);

    //Used to identify the NOKIA SU8 PROTOTYPE
    if ((aVendorId == 0x5555) && (aProductCode == 0x5555))
        {
        iIsNokiaSu8 = ETrue;
        }

    iLayout = 0; // 0 = invalid layout ID = "unknown"

    iFoundLayout = EFalse;

    if (aHidCountryId != 0)
        {
        // Use the look-up table to find the most appropriate keyboard
        // layout for this country code:
        //
        TRACE_INFO( (_L("TFindLayoutId(): country %d"),
                     aHidCountryId));
        iLayout = CountryToLayout(aHidCountryId, iIsNokiaSu8);

        if (iLayout != 0)
            {
            iFoundLayout = ETrue;
            }
        }

    if (iLayout == 0)
        {
        // Either no country code was specified, or we couldn't find
        // an appropriate layout listed in the look-up table.
        // Instead, attempt to infer the layout using the current
        // phone language setting:
        //
        TRACE_INFO( (_L("TFindLayoutId(): language %d"),
                     User::Language()));
        iLayout = LanguageToLayout(User::Language(), iIsNokiaSu8);
        }

    if (iLayout == 0)
        {
        iLayout = KDefaultLayoutId;
        TRACE_INFO( (_L("TFindLayoutId(): default layout")));
        }

    TRACE_INFO( (_L("TFindLayoutId(): layout is %d"), iLayout));
    }

// ----------------------------------------------------------------------

TInt TFindLayoutId::CountryToLayout(TInt aHidCountryId, TBool aIsNokiaSu8)
    {
    const TCountryMapEntry* table =
        aIsNokiaSu8 ? KMapSu8CountryToLayout : KMapCountryToLayout;
    const TInt size =
        aIsNokiaSu8 ? KSu8CountryMapSize : KCountryMapSize;

    TInt layout = 0;

    for (TInt i=0; (i<size) && (layout == 0); ++i)
        {
        const TCountryMapEntry& entry = table[i];

        if (entry.iHidCountryId == aHidCountryId)
            {
            layout = entry.iLayoutId;
            }
        }

    return layout;
    }

TInt TFindLayoutId::LanguageToLayout(TLanguage aLanguage, TBool aIsNokiaSu8)
    {
    TInt layout = 0;

    for (TInt i=0; (i<KLanguageMapSize) && (layout == 0); ++i)
        {
        const TLanguageMapEntry& entry = KMapLanguageToLayout[i];

        if (entry.iLanguage == aLanguage)
            {
            layout = aIsNokiaSu8 ? entry.iSu8LayoutId : entry.iLayoutId;
            }
        }

    return layout;
    }

// ----------------------------------------------------------------------
