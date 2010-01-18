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


#include "layoututils.h"

TInt CLayoutUtils::IndexFromStandardLayout(THidKeyboardLayoutId aLayoutId)
    {
    TInt returnValue = 0;
    // Map from layout code to item index position.
    switch (aLayoutId)
        {
        case EUnitedKingdom:
            returnValue = 0;
            break;
        case EUnitedStates:
            returnValue = 1;
            break;
        case EUSInternational:
            returnValue = 2;
            break;
        case EBelgian:
            returnValue = 3;
            break;
        case EDanish:
            returnValue = 4;
            break;
        case EDutch:
            returnValue = 5;
            break;
        case EFinnishSwedish:
            returnValue = 6;
            break;
        case EFrench:
            returnValue = 7;
            break;
        case EGerman:
            returnValue = 8;
            break;
        case EItalian:
            returnValue = 9;
            break;
        case ENorwegian:
            returnValue = 10;
            break;
        case EPortuguese:
            returnValue = 11;
            break;
        case ESpanish:
            returnValue = 12;
            break;
        case EUnitedStatesDvorak:
            returnValue = 13;
            break;
        default:
            break;
        }

    return returnValue;
    }

TInt CLayoutUtils::IndexFromNokiaLayout(THidKeyboardLayoutId aLayoutId)
    {
    TInt returnValue = 0;
    // Map from layout code to item index position.
    switch (aLayoutId)
        {
        case ESu8USEnglish:
            returnValue = 0;
            break;
        case ESu8FinnishSwedish:
            returnValue = 1;
            break;
        case ESu8German:
            returnValue = 2;
            break;
        case ESu8DanishNorwegian:
            returnValue = 3;
            break;
        case ESu8Russian:
            returnValue = 4;
            break;
        default:
            break;
        }

    return returnValue;
    }

THidKeyboardLayoutId CLayoutUtils::StandardLayoutFromIndex(TInt aIndex)
    {
    THidKeyboardLayoutId layoutCode = EUnitedKingdom;

    // Map it back from item index position to layout code.
    switch (aIndex)
        {
        case 0:
            layoutCode = EUnitedKingdom;
            break;
        case 1:
            layoutCode = EUnitedStates;
            break;
        case 2:
            layoutCode = EUSInternational;
            break;
        case 3:
            layoutCode = EBelgian;
            break;
        case 4:
            layoutCode = EDanish;
            break;
        case 5:
            layoutCode = EDutch;
            break;
        case 6:
            layoutCode = EFinnishSwedish;
            break;
        case 7:
            layoutCode = EFrench;
            break;
        case 8:
            layoutCode = EGerman;
            break;
        case 9:
            layoutCode = EItalian;
            break;
        case 10:
            layoutCode = ENorwegian;
            break;
        case 11:
            layoutCode = EPortuguese;
            break;
        case 12:
            layoutCode = ESpanish;
            break;
        case 13:
            layoutCode = EUnitedStatesDvorak;
            break;
        default:
            break;
        }

    return layoutCode;
    }

THidKeyboardLayoutId CLayoutUtils::NokiaLayoutFromIndex(TInt aIndex)
    {
    THidKeyboardLayoutId layoutCode = ESu8USEnglish;

    // Map it back from item index position to layout code.
    switch (aIndex)
        {
        case 0:
            layoutCode = ESu8USEnglish;
            break;
        case 1:
            layoutCode = ESu8FinnishSwedish;
            break;
        case 2:
            layoutCode = ESu8German;
            break;
        case 3:
            layoutCode = ESu8DanishNorwegian;
            break;
        case 4:
            layoutCode = ESu8Russian;
            break;
        default:
            break;
        }

    return layoutCode;
    }

void CLayoutUtils::ValidateStandardLayout(THidKeyboardLayoutId& aLayoutId)
    {
    // If the layout isn't in the range of standard layouts, set it to
    // UK
    if (!((aLayoutId >= EUnitedKingdom) && (aLayoutId <= EUnitedStatesDvorak)))
        {
        aLayoutId = EUnitedKingdom;
        }
    }

void CLayoutUtils::ValidateNokiaSU8Layout(THidKeyboardLayoutId& aLayoutId)
    {
    // If the layout isn't in the range of Nokia Su8 layouts, set it to
    // English Su8
    if (!((aLayoutId >= ESu8USEnglish) && (aLayoutId <= ESu8Russian)))
        {
        aLayoutId = ESu8USEnglish;
        }
    }

TInt CLayoutUtils::LayoutConfirmationStringL(THidKeyboardLayoutId aLayoutCode)
    {

    TInt result = 0;
    (void) aLayoutCode;
    // Only Standard layouts are supported.
    // Identify the layout confirmation string based on the layout code.
    /*
     switch ( aLayoutCode )
     {     
     case ESu8USEnglish:
     result = R_STRING_LAYOUT_ENGLISH;
     break;
     case ESu8German:
     result = R_STRING_LAYOUT_GERMAN;
     break;
     case ESu8FinnishSwedish:
     result = R_STRING_LAYOUT_FINNISH_SWEDISH;
     break;
     case ESu8DanishNorwegian:
     result = R_STRING_LAYOUT_NORWEGIAN_DANISH;
     break;
     case ESu8Russian:
     result = R_STRING_LAYOUT_RUSSIAN;
     break;            
     case EUnitedKingdom:
     result = R_STRING_LAYOUT_UK;
     break;
     case EUnitedStates:
     result = R_STRING_LAYOUT_US;
     break;
     case EUSInternational:
     result = R_STRING_LAYOUT_US_INT;
     break;
     case EBelgian:
     result = R_STRING_LAYOUT_BELGIAN;
     break;
     case EDanish:
     result = R_STRING_LAYOUT_DANISH;
     break;
     case EDutch:
     result = R_STRING_LAYOUT_DUTCH;
     break;
     case EFinnishSwedish:
     result = R_STRING_LAYOUT_FINNISH_SWEDISH;
     break;
     case EFrench:
     result = R_STRING_LAYOUT_FRENCH;
     break;
     case EGerman:
     result = R_STRING_LAYOUT_GERMAN;
     break;
     case EItalian:
     result = R_STRING_LAYOUT_ITALIAN;
     break;
     case ENorwegian:
     result = R_STRING_LAYOUT_NORWEGIAN;
     break;
     case EPortuguese:
     result = R_STRING_LAYOUT_PORTUGUESE;
     break;
     case ESpanish:
     result = R_STRING_LAYOUT_SPANISH;
     break;
     case EUnitedStatesDvorak:
     result = R_STRING_LAYOUT_US_DVORAK;
     break;    
     default:
     User::Leave(KErrNotSupported);
     break;
     }
     */
    return result;
    }

TBool CLayoutUtils::SameCategory(THidKeyboardLayoutId aInitialLayoutCode,
        THidKeyboardLayoutId aLayoutId)
    {
    // If the layout isn't in the range of Nokia Su8 layouts, set it to
    // English Su8
    TBool ret = EFalse;
    if (((aLayoutId >= ESu8USEnglish) && (aLayoutId <= ESu8Russian))
            && ((aInitialLayoutCode >= ESu8USEnglish) && (aInitialLayoutCode
                    <= ESu8Russian)))
        {
        ret = ETrue;
        }

    if (((aLayoutId >= EUnitedKingdom) && (aLayoutId <= EUnitedStatesDvorak))
            && ((aInitialLayoutCode >= EUnitedKingdom) && (aInitialLayoutCode
                    <= EUnitedStatesDvorak)))
        {
        ret = ETrue;
        }
    return ret;
    }

