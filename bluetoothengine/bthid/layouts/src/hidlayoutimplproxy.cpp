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
* Description: 
*        ECOM proxy table for this plugin
*
*/


// System includes
//
#include <e32std.h>
#include <ecom/implementationproxy.h>

// User includes
//
#include "belgian.h"
#include "danish.h"
#include "dutch.h"
#include "finnishswedish.h"
#include "french.h"
#include "german.h"
#include "italian.h"
#include "norwegian.h"
#include "portuguese.h"
#include "spanish.h"
#include "su8danishnorwegian.h"
#include "su8finnishswedish.h"
#include "su8german.h"
//#include "su8russian.h"
#include "su8russianint.h"
#include "su8intusenglish.h"
//#include "su8usenglish.h"
#include "unitedkingdom.h"
#include "unitedstates.h"
#include "unitedstatesdvorak.h"
#include "usinternational.h"

// Constants
//
const TImplementationProxy KHidKeyboardLayoutProxy[] = 
	{
	IMPLEMENTATION_PROXY_ENTRY( BELGIAN_LAYOUT_IMP,	CBelgianLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( DANISH_LAYOUT_IMP,	CDanishLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( DUTCH_LAYOUT_IMP,	CDutchLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( FINSWE_LAYOUT_IMP,	CFinnishSwedishLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( FRENCH_LAYOUT_IMP,	CFrenchLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( GERMAN_LAYOUT_IMP,	CGermanLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( ITALIAN_LAYOUT_IMP,	CItalianLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( NORWEGIAN_LAYOUT_IMP,	CNorwegianLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( PORTUGUESE_LAYOUT_IMP,	CPortugueseLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( SPANISH_LAYOUT_IMP,	CSpanishLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( SU8DANNOR_LAYOUT_IMP,	CSu8DanishNorwegianLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( SU8FINSWE_LAYOUT_IMP,	CSu8FinnishSwedishLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( SU8GERMAN_LAYOUT_IMP,	CSu8GermanLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( SU8RUSSIANINT_LAYOUT_IMP,	CSu8RussianIntLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( SU8INTUSENGLISH_LAYOUT_IMP,	CSu8IntUsEnglishLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( UK_LAYOUT_IMP,	CUnitedKingdomLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( US_LAYOUT_IMP,	CUnitedStatesLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( USDVORAK_LAYOUT_IMP,	CUnitedStatesDvorakLayout::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( USINT_LAYOUT_IMP,	CUSInternationalLayout::NewL )
	};


// ---------------------------------------------------------------------------
// ImplementationGroupProxy
// Gate/factory function
//
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(KHidKeyboardLayoutProxy) 
        / sizeof(TImplementationProxy);
	return KHidKeyboardLayoutProxy;
	}
