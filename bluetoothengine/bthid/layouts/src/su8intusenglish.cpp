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


#include "su8intusenglish.h"
#include "hiddebug.h"
#include "hidlayoutids.h"

// CONSTANTS

// Look-up tables to translate HID keyboard usage codes (usage page 7)
// to the corresponding Symbian "TKeyCode" (Unicode) values:

const TInt CSu8IntUsEnglishLayout::KColumnMap[] =
    {
    THidModifier::EUnmodified,  // ----
    THidModifier::EShift,       // ---S
    THidModifier::EAltCtrl,     // -AC-
    THidModifier::EAltCtrlShift // -ACS
    };

const TUint16 CSu8IntUsEnglishLayout::KKeyCodes[] = 
    {
    //----    ---S    -AC-    -ACS   // Hex  Dec  Usage name (UK)
         0,      0,      0,      0,  // 0x00   0  Reserved (no event)
         0,      0,      0,      0,  // 0x01   1  Rollover error
         0,      0,      0,      0,  // 0x02   2  POST Fail
         0,      0,      0,      0,  // 0x03   3  Undefined error
       'a',    'A', 0x00e1, 0x00c1,  // 0x04   4  a A
       'b',    'B',      0,      0,  // 0x05   5  b B
       'c',    'C', 0x00a9, 0x00a2,  // 0x06   6  c C
       'd',    'D', 0x00f0, 0x00d0,  // 0x07   7  d D
       'e',    'E', 0x00e9, 0x00c9,  // 0x08   8  e E
       'f',    'F',      0,      0,  // 0x09   9  f F
       'g',    'G',      0,      0,  // 0x0a  10  g G
       'h',    'H',      0,      0,  // 0x0b  11  h H
       'i',    'I', 0x00ed, 0x00cd,  // 0x0c  12  i I
       'j',    'J',      0,      0,  // 0x0d  13  j J
       'k',    'K',      0,      0,  // 0x0e  14  k K
       'l',    'L', 0x00f8, 0x00d8,  // 0x0f  15  l L
       'm',    'M', 0x00b5,      0,  // 0x10  16  m M
       'n',    'N', 0x00f1, 0x00d1,  // 0x11  17  n N
       'o',    'O', 0x00f3, 0x00d3,  // 0x12  18  o O
       'p',    'P', 0x00f6, 0x00d6,  // 0x13  19  p P
       'q',    'Q', 0x00e4, 0x00c4,  // 0x14  20  q Q
       'r',    'R', 0x00ae,      0,  // 0x15  21  r R
       's',    'S', 0x00df, 0x00a7,  // 0x16  22  s S
       't',    'T', 0x00fe, 0x00de,  // 0x17  23  t T
       'u',    'U', 0x00fa, 0x00da,  // 0x18  24  u U
       'v',    'V',      0,      0,  // 0x19  25  v V
       'w',    'W', 0x00e5, 0x00c5,  // 0x1a  26  w W
       'x',    'X',      0,      0,  // 0x1b  27  x X
       'y',    'Y', 0x00fc, 0x00dc,  // 0x1c  28  y Y
       'z',    'Z', 0x00e6, 0x00c6,  // 0x1d  29  z Z
       '1',    '!', 0x00a1, 0x00b9,  // 0x1e  30  1 !
       '2',    '@', 0x00b2,      0,  // 0x1f  31  2 "
       '3',    '#', 0x00b3,      0,  // 0x20  32  3 £
       '4',    '$', 0x00a4, 0x00a3,  // 0x21  33  4 $
       '5',    '%', 0x20ac,      0,  // 0x22  34  5 %
       '6',    '^', 0x00bc,      0,  // 0x23  35  6 ^
       '7',    '&', 0x00bd,      0,  // 0x24  36  7 &
       '8',    '*', 0x00be,      0,  // 0x25  37  8 *
       '9',    '(', 0x2018,      0,  // 0x26  38  9 (
       '0',    ')', 0x2019,      0,  // 0x27  39  0 )
    0x000d, 0xf845,      0,      0,  // 0x28  40  Enter (Return)
    0x001b, 0x001b,      0,      0,  // 0x29  41  Escape
    0x0008, 0x0008,      0,      0,  // 0x2a  42  Backspace (Delete)
    0x0009, 0x0009,      0,      0,  // 0x2b  43  Tab
       ' ',    ' ',      0,      0,  // 0x2c  44  Spacebar
       '-',    '_', 0x00a5,      0,  // 0x2d  45  - _
       '=',    '+', 0x00d7, 0x00f7,  // 0x2e  46  = +
       '[',    '{', 0x00ab,      0,  // 0x2f  47  [ {
       ']',    '}', 0x00bb,      0,  // 0x30  48  ] }
      '\\',    '|', 0x00ac, 0x00a6,  // 0x31  49  US \ bar
      '\\',    '|', 0x00ac, 0x00a6,  // 0x32  50  # ~
       ';',    ':', 0x00b6, 0x00b0,  // 0x33  51  ; :
      '\'',    '"', 0x00b4, 0x00a8,  // 0x34  52  ' @
       '`',    '~',      0,      0,  // 0x35  53  Grave/not/bar
       ',',    '<', 0x00e7, 0x00c7,  // 0x36  54  , <
       '.',    '>',      0,      0,  // 0x37  55  . >
       '/',    '?', 0x00bf,      0,  // 0x38  56  / ?      
    };

// ----------------------------------------------------------------------
// Other codes not included in the KKeyCodes table:

const CStandardKeyboardLayout::TSpecialKey 
    CSu8IntUsEnglishLayout::KSpecialCases[] =
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


// 0x005e (Circumflex accent)
//
const CDeadKeyLayout::TKeyPair CSu8IntUsEnglishLayout::KDeadKeyCircumflex[] =
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

// 0x0027 (Apostrophe)
//
const CDeadKeyLayout::TKeyPair CSu8IntUsEnglishLayout::KDeadKeyApostrophe[] =
    {
    { 0x0020, 0x0027 },    // space
    { 0x0041, 0x00c1 },    // A
    { 0x0043, 0x00c7 },    // C
    { 0x0045, 0x00c9 },    // E
    { 0x0049, 0x00cd },    // I
    { 0x004f, 0x00d3 },    // O
    { 0x0055, 0x00da },    // U
    { 0x0059, 0x00dd },    // Y
    { 0x0061, 0x00e1 },    // a
    { 0x0063, 0x00e7 },    // c
    { 0x0065, 0x00e9 },    // e
    { 0x0069, 0x00ed },    // i
    { 0x006f, 0x00f3 },    // o
    { 0x0075, 0x00fa },    // u
    { 0x0079, 0x00fd },    // y
    { 0x0000 }             // end of table
    };

// 0x0022 (Quotation mark)
//
const CDeadKeyLayout::TKeyPair CSu8IntUsEnglishLayout::KDeadKeyQuotationMark[] =
    {
    { 0x0020, 0x0022 },    // space
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

// 0x0060 (Grave accent)
//
const CDeadKeyLayout::TKeyPair CSu8IntUsEnglishLayout::KDeadKeyGrave[] =
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

// 0x007e (Tilde)
//
const CDeadKeyLayout::TKeyPair CSu8IntUsEnglishLayout::KDeadKeyTilde[] =
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

const CDeadKeyLayout::TIndexPair CSu8IntUsEnglishLayout::KDeadKeyIndex[] =
    {
    { 0x005e, KDeadKeyCircumflex },
    { 0x0027, KDeadKeyApostrophe },
    { 0x0022, KDeadKeyQuotationMark },
    { 0x0060, KDeadKeyGrave },
    { 0x007e, KDeadKeyTilde },
    { 0x0000 } // end of table
    };


// ----------------------------------------------------------------------

CSu8IntUsEnglishLayout::CSu8IntUsEnglishLayout()
    : CDeadKeyLayout(KColumnMap, sizeof (KColumnMap),
        KKeyCodes, sizeof (KKeyCodes), KSpecialCases, KDeadKeyIndex)
    {
    // nothing else to do
    }

TBool CSu8IntUsEnglishLayout::ChangesWithCapsLock(TInt aHidKey,
    THidModifier aModifiers) const
    {
    if (aModifiers.None() || aModifiers.ShiftOnly())
        {
        // Additional keys that should behave as if shift is pressed
        // whenever caps lock is active:
        //
        TBool isExtraCapsKey = 
            (aHidKey == 0x1e) || (aHidKey == 0x1f) || (aHidKey == 0x20) ||
            (aHidKey == 0x21) || (aHidKey == 0x22) || (aHidKey == 0x23) ||
            (aHidKey == 0x24) || (aHidKey == 0x25) || (aHidKey == 0x26) ||
            (aHidKey == 0x27) || (aHidKey == 0x2d) || (aHidKey == 0x2f) ||
            (aHidKey == 0x31) || (aHidKey == 0x33) || (aHidKey == 0x34) ||
            (aHidKey == 0x36) || (aHidKey == 0x37);  

        if (isExtraCapsKey)
            return ETrue;
        }

    return CDeadKeyLayout::ChangesWithCapsLock(aHidKey, aModifiers);
    }

TInt CSu8IntUsEnglishLayout::RawScanCode(TInt aHidKey, 
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

TInt CSu8IntUsEnglishLayout::LayoutId() const
    {
    DBG(RDebug::Print(_L("[HID]\tCSu8IntUsEnglishLayout::LayoutId()")));
    return ESu8USEnglish;
    }

CKeyboardLayout* CSu8IntUsEnglishLayout::NewL()
    {
    DBG(RDebug::Print(_L("[HID]\tCSu8IntUsEnglishLayout::NewL()")));
    CKeyboardLayout* layout = new (ELeave) CSu8IntUsEnglishLayout;
    return layout;
    }

// End of file


