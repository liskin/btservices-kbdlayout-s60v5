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


#include "dutch.h"
#include "hiddebug.h"
#include "hidlayoutids.h"

// CONSTANTS

// Look-up tables to translate HID keyboard usage codes (usage page 7)
// to the corresponding Symbian "TKeyCode" (Unicode) values:

const TInt CDutchLayout::KColumnMap[] =
    {
    THidModifier::EUnmodified,  // ----
    THidModifier::EShift,       // ---S
    THidModifier::EAltCtrl      // -AC-
    };

const TUint16 CDutchLayout::KKeyCodes[] = 
    {
    //----    ---S    -AC-   // Hex  Dec  Usage name (UK)
         0,      0,      0,  // 0x00   0  Reserved (no event)
         0,      0,      0,  // 0x01   1  Rollover error
         0,      0,      0,  // 0x02   2  POST Fail
         0,      0,      0,  // 0x03   3  Undefined error
       'a',    'A',      0,  // 0x04   4  a A
       'b',    'B',      0,  // 0x05   5  b B
       'c',    'C', 0x00a2,  // 0x06   6  c C
       'd',    'D',      0,  // 0x07   7  d D
       'e',    'E', 0x20ac,  // 0x08   8  e E
       'f',    'F',      0,  // 0x09   9  f F
       'g',    'G',      0,  // 0x0a  10  g G
       'h',    'H',      0,  // 0x0b  11  h H
       'i',    'I',      0,  // 0x0c  12  i I
       'j',    'J',      0,  // 0x0d  13  j J
       'k',    'K',      0,  // 0x0e  14  k K
       'l',    'L',      0,  // 0x0f  15  l L
       'm',    'M', 0x00b5,  // 0x10  16  m M
       'n',    'N',      0,  // 0x11  17  n N
       'o',    'O',      0,  // 0x12  18  o O
       'p',    'P',      0,  // 0x13  19  p P
       'q',    'Q',      0,  // 0x14  20  q Q
       'r',    'R', 0x00b6,  // 0x15  21  r R
       's',    'S', 0x00df,  // 0x16  22  s S
       't',    'T',      0,  // 0x17  23  t T
       'u',    'U',      0,  // 0x18  24  u U
       'v',    'V',      0,  // 0x19  25  v V
       'w',    'W',      0,  // 0x1a  26  w W
       'x',    'X', 0x00bb,  // 0x1b  27  x X
       'y',    'Y',      0,  // 0x1c  28  y Y
       'z',    'Z', 0x00ab,  // 0x1d  29  z Z
       '1',    '!', 0x00b9,  // 0x1e  30  1 !
       '2',    '"', 0x00b2,  // 0x1f  31  2 "
       '3',    '#', 0x00b3,  // 0x20  32  3 �
       '4',    '$', 0x00bc,  // 0x21  33  4 $
       '5',    '%', 0x00bd,  // 0x22  34  5 %
       '6',    '&', 0x00be,  // 0x23  35  6 ^
       '7',    '_', 0x00a3,  // 0x24  36  7 &
       '8',    '(',    '{',  // 0x25  37  8 *
       '9',    ')',    '}',  // 0x26  38  9 (
       '0',   '\'',      0,  // 0x27  39  0 )
    0x000d, 0xf845,      0,  // 0x28  40  Enter (Return)
    0x001b, 0x001b,      0,  // 0x29  41  Escape
    0x0008, 0x0008,      0,  // 0x2a  42  Backspace (Delete)
    0x0009, 0x0009,      0,  // 0x2b  43  Tab
       ' ',    ' ',      0,  // 0x2c  44  Spacebar
       '/',    '?',   '\\',  // 0x2d  45  - _
    0x00b0,    '~', 0x00b8,  // 0x2e  46  = +
    0x00a8,    '^',      0,  // 0x2f  47  [ {
       '*',    '|',      0,  // 0x30  48  ] }
       '<',    '>',      0,  // 0x31  49  US \ bar
       '<',    '>',      0,  // 0x32  50  # ~
       '+', 0x00b1,      0,  // 0x33  51  ; :
    0x00b4,    '`',      0,  // 0x34  52  ' @
       '@', 0x00a7, 0x00ac,  // 0x35  53  Grave/not/bar
       ',',    ';',      0,  // 0x36  54  , <
       '.',    ':', 0x00b7,  // 0x37  55  . >
       '-',    '=',      0,  // 0x38  56  / ?
         0,      0,      0,  // 0x39  57  Caps Lock
    0xf816, 0xf816,      0,  // 0x3a  58  F1
    0xf817, 0xf817,      0,  // 0x3b  59  F2
    0xf818, 0xf818,      0,  // 0x3c  60  F3
    0xf819, 0xf819,      0,  // 0x3d  61  F4
    0xf81a, 0xf81a,      0,  // 0x3e  62  F5
    0xf81b, 0xf81b,      0,  // 0x3f  63  F6
    0xf81c, 0xf81c,      0,  // 0x40  64  F7
    0xf81d, 0xf81d,      0,  // 0x41  65  F8
    0xf81e, 0xf81e,      0,  // 0x42  66  F9
    0xf81f, 0xf81f,      0,  // 0x43  67  F10
    0xf820, 0xf820,      0,  // 0x44  68  F11
    0xf821, 0xf821,      0,  // 0x45  69  F12
    0xf800, 0xf800,      0,  // 0x46  70  Print Screen
    0xf815, 0xf815,      0,  // 0x47  71  Scroll Lock
    0xf801, 0xf801,      0,  // 0x48  72  Break / Pause
    0xf806, 0xf806,      0,  // 0x49  73  Insert
    0xf802, 0xf802,      0,  // 0x4a  74  Home
    0xf804, 0xf804,      0,  // 0x4b  75  Page Up
    0x007f, 0x007f, 0xf844,  // 0x4c  76  Delete
    0xf803, 0xf803,      0,  // 0x4d  77  End
    0xf805, 0xf805,      0,  // 0x4e  78  Page Down
    0xf808, 0xf808,      0,  // 0x4f  79  Right Arrow
    0xf807, 0xf807,      0,  // 0x50  80  Left Arrow
    0xf80a, 0xf80a,      0,  // 0x51  81  Down Arrow
    0xf809, 0xf809,      0,  // 0x52  82  Up Arrow
         0,      0,      0,  // 0x53  83  Num Lock
       '/',    '/',      0,  // 0x54  84  Keypad /
       '*',    '*',      0,  // 0x55  85  Keypad *
       '-',    '-',      0,  // 0x56  86  Keypad -
       '+',    '+',      0,  // 0x57  87  Keypad +
    0x000d, 0xf845,      0,  // 0x58  88  Keypad Enter
       '1', 0xf803,      0,  // 0x59  89  Keypad 1 End
       '2', 0xf80a,      0,  // 0x5a  90  Keypad 2 Down
       '3', 0xf805,      0,  // 0x5b  91  Keypad 3 PageDn
       '4', 0xf807,      0,  // 0x5c  92  Keypad 4 Left
       '5',    ' ',      0,  // 0x5d  93  Keypad 5
       '6', 0xf808,      0,  // 0x5e  94  Keypad 6 Right
       '7', 0xf802,      0,  // 0x5f  95  Keypad 7 Home
       '8', 0xf809,      0,  // 0x60  96  Keypad 8 Up
       '9', 0xf804,      0,  // 0x61  97  Keypad 9 PageUp
       '0', 0xf806,      0,  // 0x62  98  Keypad 0 Insert
       ',', 0x007f, 0xf844,  // 0x63  99  Keypad . Delete
       ']',    '[', 0x00a6,  // 0x64 100  \ | (near left shift)
    0xf843, 0xf863,      0,  // 0x65 101  Application key
         0,      0,      0,  // 0x66 102  Keyboard power
       '=',    '=',      0,  // 0x67 103  Keypad =
    };

// ----------------------------------------------------------------------

const CStandardKeyboardLayout::TSpecialKey CDutchLayout::KSpecialCases[] =
    {
    // HID usage, modifiers, Unicode            // Usage name (UK)
    { 0x28, THidModifier::ECtrl,      0xf845 }, // Enter (Return)
    { 0x2c, THidModifier::ECtrl,      0x0020 }, // Spacebar
    { 0x58, THidModifier::ECtrl,      0xf845 }, // Keypad Enter
    { 0x2b, THidModifier::EAlt,       0xf852 }, // Tab
    { 0xe3, THidModifier::EFunc,      0xf842 }, // Left GUI key
    { 0xe7, THidModifier::EFunc,      0xf843 }, // Right GUI key
    { 0xe3, THidModifier::EFuncShift, 0xf862 }, // Left GUI key
    { 0xe7, THidModifier::EFuncShift, 0xf863 }, // Right GUI key
    { 0x00 } // end of table
    };

// ----------------------------------------------------------------------

// 0x007e (Tilde)
//
const CDeadKeyLayout::TKeyPair CDutchLayout::KDeadKeyTilde[] =
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

// 0x00b8 (Cedilla)
//
const CDeadKeyLayout::TKeyPair CDutchLayout::KDeadKeyCedilla[] =
    {
    { 0x0020, 0x00b8 },    // space
    { 0x0043, 0x00c7 },    // C
    { 0x0063, 0x00e7 },    // c
    { 0x0000 }             // end of table
    };

// 0x00a8 (Diaeresis)
//
const CDeadKeyLayout::TKeyPair CDutchLayout::KDeadKeyDiaeresis[] =
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
const CDeadKeyLayout::TKeyPair CDutchLayout::KDeadKeyCircumflex[] =
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

// 0x00b4 (Acute accent)
//
const CDeadKeyLayout::TKeyPair CDutchLayout::KDeadKeyAcute[] =
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
const CDeadKeyLayout::TKeyPair CDutchLayout::KDeadKeyGrave[] =
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

// ----------------------------------------------------------------------

const CDeadKeyLayout::TIndexPair CDutchLayout::KDeadKeyIndex[] =
    {
    { 0x007e, KDeadKeyTilde },
    { 0x00b8, KDeadKeyCedilla },
    { 0x00a8, KDeadKeyDiaeresis },
    { 0x005e, KDeadKeyCircumflex },
    { 0x00b4, KDeadKeyAcute },
    { 0x0060, KDeadKeyGrave },
    { 0x0000 } // end of table
    };


CDutchLayout::CDutchLayout()
    : CDeadKeyLayout(KColumnMap, sizeof (KColumnMap),
        KKeyCodes, sizeof (KKeyCodes), KSpecialCases, KDeadKeyIndex)
    {
    // nothing else to do
    }

TInt CDutchLayout::LayoutId() const
    {
    DBG(RDebug::Print(_L("[HID]\tCDutchLayout::LayoutId()")));
    return EDutch;
    }

CKeyboardLayout* CDutchLayout::NewL()
    {
    DBG(RDebug::Print(_L("[HID]\tCDutchLayout::NewL()")));
    CKeyboardLayout* layout = new (ELeave) CDutchLayout;
    return layout;
    }

