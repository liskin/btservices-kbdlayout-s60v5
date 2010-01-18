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
* Description:  Identifying codes for keyboard layouts.
 *
*/


#ifndef __HIDLAYOUTIDS_H
#define __HIDLAYOUTIDS_H

enum THidKeyboardLayoutId
    {
    EInvalidLayoutId = 0,
    //
    // Nokia SU-8 layouts:
    //
    ESu8USEnglish = 1, //!< Nokia SU-8 US English
    ESu8German = 2, //!< Nokia SU-8 German
    ESu8FinnishSwedish = 3, //!< Nokia SU-8 Finnish and Swedish
    ESu8DanishNorwegian = 4, //!< Nokia SU-8 Danish and Norwegian
    ESu8Russian = 5, //!< Nokia SU-8 Russian
    //
    // Standard layouts:
    //
    EUnitedKingdom = 6, //!< UK layout
    EUnitedStates = 7, //!< Standard US layout
    EUSInternational = 8, //!< US layout with dead keys
    EBelgian = 9, //!< Belgian
    EDanish = 10, //!< Danish
    EDutch = 11, //!< Dutch
    EFinnishSwedish = 12, //!< Finnish and Swedish layouts are identical
    EFrench = 13, //!< French
    EGerman = 14, //!< German
    EItalian = 15, //!< Italian
    ENorwegian = 16, //!< Norwegian
    EPortuguese = 17, //!< Portuguese
    ESpanish = 18, //!< Spanish
    EUnitedStatesDvorak = 19, //!< Standard two-hand Dvorak US layout
    //
    // ------------------------------------------------------------
    // The following are for testing purposes only and may be removed:
    //
    // Removed from requirements:
    //
    ESwissFrench = 22,
    ESwissGerman = 23,
    //
    // Were never in requirements:
    //
    EUSDvorakLeftHand = 24,
    EUSDvorakRightHand = 25,
    ECanadianFrench = 26,
    ECanadianMultilingual = 27,
    EIcelandic = 28,
    ELatinAmerican = 29,
    EBelgianComma = 30, // As Belgian, but keypad '.' becomes ','
    EItalian142 = 31,
    EGermanIBM = 32,
    // New Requirements
    ERussian = 33
    };
#endif // __HIDLAYOUTIDS_H
// End of file
