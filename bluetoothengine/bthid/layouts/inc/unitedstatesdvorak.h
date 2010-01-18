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


#ifndef UNITEDSTATESDVORAK_H
#define UNITEDSTATESDVORAK_H

// INCLUDES
#include <e32std.h>
#include <e32svr.h>

#include "layout.h"
#include "modifier.h"

// CLASS DECLARATION

class CUnitedStatesDvorakLayout : public CStandardKeyboardLayout
    {
public:

    static CKeyboardLayout* NewL();

    // Report what layouts are supported:
    virtual TInt LayoutId() const;

    // Layout has additional Caps Lock keys:
    virtual TBool
            ChangesWithCapsLock(TInt aHidKey, THidModifier aModifiers) const;

private:

    /* C++ constructor */
    CUnitedStatesDvorakLayout();

    // Standard look-up table:
    static const TInt KColumnMap[];
    static const TUint16 KKeyCodes[];

    // Special cases:
    static const TSpecialKey KSpecialCases[];
    };

#endif  //UNITEDSTATESDVORAK_H
//End of file

