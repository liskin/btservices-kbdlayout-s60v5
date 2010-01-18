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


#ifndef SU8FINNISHSWEDISH_H
#define SU8FINNISHSWEDISH_H

// INCLUDES
#include <e32std.h>
#include <e32svr.h>

#include "layout.h"
#include "modifier.h"

// CLASS DECLARATION

class CSu8FinnishSwedishLayout : public CDeadKeyLayout
    {
public:

    static CKeyboardLayout* NewL();

    // Report what layouts are supported:
    virtual TInt LayoutId() const;

    // Layout has additional Caps Lock keys:
    virtual TBool
            ChangesWithCapsLock(TInt aHidKey, THidModifier aModifiers) const;

    // For application switch key and middle softkey scancodes:
    TInt
            RawScanCode(TInt aHidKey, TInt aUsagePage,
                    THidModifier aModifiers) const;

private:

    /* C++ constructor */
    CSu8FinnishSwedishLayout();

    // Standard look-up table:
    static const TInt KColumnMap[];
    static const TUint16 KKeyCodes[];

    // Special cases:
    static const TSpecialKey KSpecialCases[];

    // Dead key tables:
    static const TIndexPair KDeadKeyIndex[];
    static const TKeyPair KDeadKeyAcute[];
    static const TKeyPair KDeadKeyGrave[];
    static const TKeyPair KDeadKeyDiaeresis[];
    static const TKeyPair KDeadKeyCircumflex[];
    static const TKeyPair KDeadKeyTilde[];

    // Caps Lock look-up table for Danish/Norwegian:
    static const TSpecialKey KDanishKeyCodes[];
    };

#endif  //SU8FINNISHSWEDISH_H
//End of file

