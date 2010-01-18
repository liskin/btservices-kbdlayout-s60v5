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


#ifndef SU8RUSSIANINT_H
#define SU8RUSSIANINT_H

// INCLUDES
#include <e32std.h>
#include <e32svr.h>

#include "layout.h"
#include "modifier.h"

// CLASS DECLARATION

class CSu8RussianIntLayout : public CDeadKeyLayout
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
    CSu8RussianIntLayout();

    // Standard look-up table:
    static const TInt KColumnMap[];
    static const TUint16 KKeyCodes[];

    // Special cases:
    static const TSpecialKey KSpecialCases[];

    // Dead key tables:
    static const TIndexPair KDeadKeyIndex[];
    static const TKeyPair KDeadKeyAcute[];
    static const TKeyPair KDeadKeyGrave[];
    static const TKeyPair KDeadKeyCircumflex[];
    };

#endif  //SU8RUSSIANINT_H
//End of file

