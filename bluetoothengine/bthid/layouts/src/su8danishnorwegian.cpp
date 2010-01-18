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


#include "su8danishnorwegian.h"
#include "hiddebug.h"
#include "hidlayoutids.h"

// CONSTANTS

// Look-up tables to translate HID keyboard usage codes (usage page 7)
// to the corresponding Symbian "TKeyCode" (Unicode) values:

const TInt CSu8DanishNorwegianLayout::KColumnMap[] =
    {
    THidModifier::EUnmodified,
    THidModifier::EShift,
    THidModifier::EAltCtrl
    };

const TUint16 CSu8DanishNorwegianLayout::KKeyCodes[] = 
    {
    //----    ---S    -AC-   // Hex  Dec
         0,      0,      0,  // 0x00   0
         0,      0,      0,  // 0x01   1
         0,      0,      0,  // 0x02   2
         0,      0,      0,  // 0x03   3
       'a',    'A',      0,  // 0x04   4
       'b',    'B',      0,  // 0x05   5
       'c',    'C',      0,  // 0x06   6
       'd',    'D',      0,  // 0x07   7
       'e',    'E', 0x20ac,  // 0x08   8
       'f',    'F',      0,  // 0x09   9
       'g',    'G',      0,  // 0x0a  10
       'h',    'H',      0,  // 0x0b  11
       'i',    'I',    '[',  // 0x0c  12 //was: }
       'j',    'J',      0,  // 0x0d  13
       'k',    'K',      0,  // 0x0e  14
       'l',    'L',      0,  // 0x0f  15
       'm',    'M',      0,  // 0x10  16
       'n',    'N',      0,  // 0x11  17
       'o',    'O',    ']',  // 0x12  18 was: [
       'p',    'P',    '}',  // 0x13  19 was: ]
       'q',    'Q', 0x005e,  // 0x14  20
       'r',    'R',    '$',  // 0x15  21
       's',    'S',      0,  // 0x16  22
       't',    'T', 0x00a3,  // 0x17  23
       'u',    'U',    '{',  // 0x18  24
       'v',    'V',      0,  // 0x19  25
       'w',    'W',    '@',  // 0x1a  26
       'x',    'X',      0,  // 0x1b  27
       'y',    'Y',      0,  // 0x1c  28
       'z',    'Z',      0,  // 0x1d  29
       '1',    '!', 0x005e,  // 0x1e  30
       '2',    '"',    '@',  // 0x1f  31
       '3',    '#', 0x20ac,  // 0x20  32
       '4', 0x00a4,    '$',  // 0x21  33
       '5',    '%', 0x00a3,  // 0x22  34
       '6',    '&',      0,  // 0x23  35
       '7',    '/',    '{',  // 0x24  36
       '8',    '(',    '[',  // 0x25  37 was: }
       '9',    ')',    ']',  // 0x26  38 was: [
       '0',    '=',    '}',  // 0x27  39 was: ]
    0x000d, 0xf845,      0,  // 0x28  40
    0x001b, 0x001b,      0,  // 0x29  41
    0x0008, 0x0008,      0,  // 0x2a  42
    0x0009, 0x0009,      0,  // 0x2b  43
       ' ',    ' ',      0,  // 0x2c  44
       '+',    '?',      0,  // 0x2d  45
    0x00b4, 0x0060,   '\\',  // 0x2e  46
    0x00e5, 0x00c5,      0,  // 0x2f  47
    0x00df, 0x00df,   '\\',  // 0x30  48
    0x00e4, 0x00c4,      0,  // 0x31  49
         0,      0,      0,  // 0x32  50
    0x00f6, 0x00d6,      0,  // 0x33  51
    0x00e4, 0x00c4,      0,  // 0x34  52
    0x0027, 0x002a, 0x007e,  // 0x35  53
       ',',    ';',    '<',  // 0x36  54
       '.',    ':',    '>',  // 0x37  55
       '-',    '_', 0x00a8   // 0x38  56
    };

// ----------------------------------------------------------------------
// Keys 0x33 and 0x34 change for the Danish/Norwegian layout:

const CStandardKeyboardLayout::TSpecialKey 
    CSu8DanishNorwegianLayout::KDanishKeyCodes[] =
    {
    { 0x33, THidModifier::EUnmodified, 0x00f8 },
    { 0x33, THidModifier::EShift,      0x00d8 },
    { 0x34, THidModifier::EUnmodified, 0x00e6 },
    { 0x34, THidModifier::EShift,      0x00c6 },
    //
    { 0x00 } // end of table
    };

// ----------------------------------------------------------------------
// Other codes not included in the KKeyCodes table:

const CStandardKeyboardLayout::TSpecialKey 
    CSu8DanishNorwegianLayout::KSpecialCases[] =
    {
    // HID usage, modifiers, Unicode
    { 0x28, THidModifier::ECtrl,       0xf845 }, // Ctrl-enter (Return)
    { 0x2c, THidModifier::ECtrl,       0x0020 }, // Ctrl-spacebar
    { 0x2b, THidModifier::EAlt,        0xf852 }, // Alt-tab
    { 0x4c, THidModifier::EUnmodified, 0x007f }, // Delete
    { 0x4c, THidModifier::EShift,      0x007f }, // Shift-Delete
    { 0x4c, THidModifier::EAltCtrl,    0xf844 }, // Alt-ctrl-delete
    { 0x4f, THidModifier::EUnmodified, 0xf808 }, // Right arrow
    { 0x4f, THidModifier::EShift,      0xf808 }, // Shift-right arrow
    { 0x50, THidModifier::EUnmodified, 0xf807 }, // Left arrow
    { 0x50, THidModifier::EShift,      0xf807 }, // Shift-left arrow
    { 0x51, THidModifier::EUnmodified, 0xf80a }, // Down arrow
    { 0x51, THidModifier::EShift,      0xf80a }, // Shift-down arrow
    { 0x52, THidModifier::EUnmodified, 0xf809 }, // Up arrow
    { 0x52, THidModifier::EShift,      0xf809 }, // Shift-up arrow
    { 0xe3, THidModifier::EFunc,       0xf842 }, // Left GUI key
    { 0xe7, THidModifier::EFunc,       0xf843 }, // Right GUI key
    { 0xe3, THidModifier::EFuncShift,  0xf862 }, // Left GUI key
    { 0xe7, THidModifier::EFuncShift,  0xf863 }, // Right GUI key
    { 0x76, THidModifier::EUnmodified, 0xf852 }, // App switch to App0
    { 0x76, THidModifier::EShift,      0xf852 }, // Shift-App switch to App0
    { 0x77, THidModifier::EUnmodified, 0xf845 }, // MSK to OK
    { 0x77, THidModifier::EShift,      0xf845 }, // Shift-MSK to OK
    //
    { 0x00 } // end of table
    };

// ----------------------------------------------------------------------

// 0x00b4 (Acute accent)
//
const CDeadKeyLayout::TKeyPair CSu8DanishNorwegianLayout::KDeadKeyAcute[] =
    {
    { 0x0020, 0x00b4 },    // space
    { 0x0041, 0x00c1 },    // A
    { 0x0045, 0x00c9 },    // E
    { 0x0049, 0x00cd },    // I
    { 0x004f, 0x00d3 },    // O
    { 0x0055, 0x00da },    // U
    { 0x0059, 0x00dd },    // Y
    { 0x0061, 0x00e1 },    // a
    { 0x0065, 0x00e9 },    // e
    { 0x0069, 0x00ed },    // i
    { 0x006f, 0x00f3 },    // o
    { 0x0075, 0x00fa },    // u
    { 0x0079, 0x00fd },    // y
    { 0x0000 }             // end of table
    };

// 0x0060 (Grave accent)
//
const CDeadKeyLayout::TKeyPair CSu8DanishNorwegianLayout::KDeadKeyGrave[] =
    {
    { 0x0020, 0x0060 },    // space
    { 0x0041, 0x00c0 },    // A
    { 0x0045, 0x00c8 },    // E
    { 0x0049, 0x00cc },    // I
    { 0x004f, 0x00d2 },    // O
    { 0x0055, 0x00d9 },    // U
    { 0x0061, 0x00e0 },    // a
    { 0x0065, 0x00e8 },    // e
    { 0x0069, 0x00ec },    // i
    { 0x006f, 0x00f2 },    // o
    { 0x0075, 0x00f9 },    // u
    { 0x0000 }             // end of table
    };

// 0x00a8 (Diaeresis)
//
const CDeadKeyLayout::TKeyPair CSu8DanishNorwegianLayout::KDeadKeyDiaeresis[] =
    {
    { 0x0020, 0x00a8 },    // space
    { 0x0041, 0x00c4 },    // A
    { 0x0045, 0x00cb },    // E
    { 0x0049, 0x00cf },    // I
    { 0x004f, 0x00d6 },    // O
    { 0x0055, 0x00dc },    // U
    { 0x0061, 0x00e4 },    // a
    { 0x0065, 0x00eb },    // e
    { 0x0069, 0x00ef },    // i
    { 0x006f, 0x00f6 },    // o
    { 0x0075, 0x00fc },    // u
    { 0x0079, 0x00ff },    // y
    { 0x0000 }             // end of table
    };

// 0x005e (Circumflex accent)
//
const CDeadKeyLayout::TKeyPair CSu8DanishNorwegianLayout::KDeadKeyCircumflex[] =
    {
    { 0x0020, 0x005e },    // space
    { 0x0041, 0x00c2 },    // A
    { 0x0045, 0x00ca },    // E
    { 0x0049, 0x00ce },    // I
    { 0x004f, 0x00d4 },    // O
    { 0x0055, 0x00db },    // U
    { 0x0061, 0x00e2 },    // a
    { 0x0065, 0x00ea },    // e
    { 0x0069, 0x00ee },    // i
    { 0x006f, 0x00f4 },    // o
    { 0x0075, 0x00fb },    // u
    { 0x0000 }             // end of table
    };

// 0x007e (Tilde)
//
const CDeadKeyLayout::TKeyPair CSu8DanishNorwegianLayout::KDeadKeyTilde[] =
    {
    { 0x0020, 0x007e },    // space
    { 0x0041, 0x00c3 },    // A
    { 0x004e, 0x00d1 },    // N
    { 0x004f, 0x00d5 },    // O
    { 0x0061, 0x00e3 },    // a
    { 0x006e, 0x00f1 },    // n
    { 0x006f, 0x00f5 },    // o
    { 0x0000 }             // end of table
    };

// ----------------------------------------------------------------------

const CDeadKeyLayout::TIndexPair CSu8DanishNorwegianLayout::KDeadKeyIndex[] =
    {
    { 0x00b4, KDeadKeyAcute },
    { 0x0060, KDeadKeyGrave },
    { 0x00a8, KDeadKeyDiaeresis },
    { 0x005e, KDeadKeyCircumflex },
    { 0x007e, KDeadKeyTilde },
    { 0x0000 } // end of table
    };

// ----------------------------------------------------------------------

CSu8DanishNorwegianLayout::CSu8DanishNorwegianLayout()
    : CDeadKeyLayout(KColumnMap, sizeof (KColumnMap),
      KKeyCodes, sizeof (KKeyCodes), KSpecialCases, KDeadKeyIndex)
    {
    // nothing else to do
    }

TUint16 CSu8DanishNorwegianLayout::Unicode(TInt aHidKey, 
    THidModifier aModifiers) const
    {
    TUint16 unicode = 0;
    
    // Keys 0x33 and 0x34 change for the Danish/Norwegian layout:
    unicode = CDeadKeyLayout::FindSpecialKey(
        KDanishKeyCodes, aHidKey, aModifiers.Fold());
    
    return (unicode != 0) ? unicode : 
        CDeadKeyLayout::Unicode(aHidKey, aModifiers);
    }

TBool CSu8DanishNorwegianLayout::ChangesWithCapsLock(TInt aHidKey,
    THidModifier aModifiers) const
    {
    if (aModifiers.None() || aModifiers.ShiftOnly())
        {
        // Additional keys that should behave as if shift is pressed
        // whenever caps lock is active:
        //
        TBool isExtraCapsKey = 
            (aHidKey == 0x2f) ||
            (aHidKey == 0x31) ||
            (aHidKey == 0x33) ||
            (aHidKey == 0x34);

        if (isExtraCapsKey)
            return ETrue;
        }

    return CDeadKeyLayout::ChangesWithCapsLock(aHidKey, aModifiers);
    }

TInt CSu8DanishNorwegianLayout::RawScanCode(TInt aHidKey, 
    TInt aUsagePage, THidModifier aModifiers) const
    {
    TInt result = EStdKeyNull;

    const TInt KKeyboardUsagePage = 0x07;
    if (aUsagePage == KKeyboardUsagePage)
        {
        // Application switch key is HID "Keyboard Menu" usage:
        //
        const TInt KHidKeyboardMenu = 0x76;
        if (aHidKey == KHidKeyboardMenu)
            {
            result = EStdKeyApplication0;
            }
        
        // Middle soft key is HID "Keyboard Select" usage:
        //
        const TInt KHidKeyboardSelect = 0x77;
        if (aHidKey == KHidKeyboardSelect)
            {
            result = EStdKeyDevice3;
            }
        }
    
    if (result == EStdKeyNull)
        {
        result = CDeadKeyLayout::RawScanCode(aHidKey,
            aUsagePage, aModifiers);
        }
    
    return result;
    }

TInt CSu8DanishNorwegianLayout::LayoutId() const
    {
    DBG(RDebug::Print(_L("[HID]\tCSu8DanishNorwegianLayout::LayoutId()")));
    return ESu8DanishNorwegian;
    }

CKeyboardLayout* CSu8DanishNorwegianLayout::NewL()
    {
    DBG(RDebug::Print(_L("[HID]\tCSu8DanishNorwegianLayout::NewL()")));
    CKeyboardLayout* layout = new (ELeave) CSu8DanishNorwegianLayout;
    return layout;
    }

// End of file

