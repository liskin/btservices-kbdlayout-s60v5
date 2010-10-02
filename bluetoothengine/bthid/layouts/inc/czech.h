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


#ifndef CZECH_H
#define CZECH_H

// INCLUDES
#include <e32std.h>
#include <e32svr.h>

#include "layout.h"
#include "modifier.h"

// CLASS DECLARATION

class CCzechLayout : public CDeadKeyLayout
    {
public:

    static CKeyboardLayout* NewL();

    // Report what layouts are supported:
    virtual TInt LayoutId() const;

    virtual TBool ChangesWithCapsLock(TInt aHidKey, THidModifier aModifiers) const {
	return EFalse;
    }

private:

    /* C++ constructor */
    CCzechLayout();

    // Standard look-up table:
    static const TInt KColumnMap[];
    static const TUint16 KKeyCodes[];

    // Special cases:
    static const TSpecialKey KSpecialCases[];

    // Dead key tables:
    static const TIndexPair KDeadKeyIndex[];
    static const TKeyPair KDeadKeyAcute[];      // ' wẃeérŕzźuúiíoópṕaásśgǵkḱlĺyýcćnńmḿWẂEÉRŔZŹUÚIÍOÓPṔAÁSŚGǴKḰLĹYÝCĆNŃMḾ
    static const TKeyPair KDeadKeyCaron[];      // ˇ eěrřtťzžuǔiǐoǒaǎsšdďgǧhȟjǰkǩlľcčnňEĚRŘTŤZŽUǓIǏOǑAǍSŠDĎGǦHȞKǨLĽCČUǙNŇ
    static const TKeyPair KDeadKeyDiaeresis[];  // " wẅeëtẗuüiïoöaähḧyÿxẍWẄEËUÜIÏOÖAÄHḦYŸXẌ
    static const TKeyPair KDeadKeyAbovering[];  // ° wẘuůaåyẙOŮAÅ
    static const TKeyPair KDeadKeyCircumflex[]; // ^ wŵeêzẑuûiîoôaâsŝgĝhĥjĵyŷcĉWŴEÊZẐUÛIÎOÔAÂSŜGĜHĤJĴYŶCĈ
    // I'm too lazy to fill in the other dead keys
    };

#endif  //CZECH_H
//End of file

