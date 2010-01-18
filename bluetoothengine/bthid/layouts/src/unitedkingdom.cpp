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


#include "unitedkingdom.h"
#include "hiddebug.h"
#include "hidlayoutids.h"

// CONSTANTS

// Look-up tables to translate HID keyboard usage codes (usage page 7)
// to the corresponding Symbian "TKeyCode" (Unicode) values:

const TInt CUnitedKingdomLayout::KColumnMap[] =
    {
    THidModifier::EUnmodified,  // ----
    THidModifier::EShift        // ---S
    };

const TUint16 CUnitedKingdomLayout::KKeyCodes[] = 
    {
    //----    ---S   // Hex  Dec  Usage name (UK)
         0,      0,  // 0x00   0  Reserved (no event)
         0,      0,  // 0x01   1  Rollover error
         0,      0,  // 0x02   2  POST Fail
         0,      0,  // 0x03   3  Undefined error
       'a',    'A',  // 0x04   4  a A
       'b',    'B',  // 0x05   5  b B
       'c',    'C',  // 0x06   6  c C
       'd',    'D',  // 0x07   7  d D
       'e',    'E',  // 0x08   8  e E
       'f',    'F',  // 0x09   9  f F
       'g',    'G',  // 0x0a  10  g G
       'h',    'H',  // 0x0b  11  h H
       'i',    'I',  // 0x0c  12  i I
       'j',    'J',  // 0x0d  13  j J
       'k',    'K',  // 0x0e  14  k K
       'l',    'L',  // 0x0f  15  l L
       'm',    'M',  // 0x10  16  m M
       'n',    'N',  // 0x11  17  n N
       'o',    'O',  // 0x12  18  o O
       'p',    'P',  // 0x13  19  p P
       'q',    'Q',  // 0x14  20  q Q
       'r',    'R',  // 0x15  21  r R
       's',    'S',  // 0x16  22  s S
       't',    'T',  // 0x17  23  t T
       'u',    'U',  // 0x18  24  u U
       'v',    'V',  // 0x19  25  v V
       'w',    'W',  // 0x1a  26  w W
       'x',    'X',  // 0x1b  27  x X
       'y',    'Y',  // 0x1c  28  y Y
       'z',    'Z',  // 0x1d  29  z Z
       '1',    '!',  // 0x1e  30  1 !
       '2',    '"',  // 0x1f  31  2 "
       '3', 0x00a3,  // 0x20  32  3 �
       '4',    '$',  // 0x21  33  4 $
       '5',    '%',  // 0x22  34  5 %
       '6',    '^',  // 0x23  35  6 ^
       '7',    '&',  // 0x24  36  7 &
       '8',    '*',  // 0x25  37  8 *
       '9',    '(',  // 0x26  38  9 (
       '0',    ')',  // 0x27  39  0 )
    0x000d, 0xf845,  // 0x28  40  Enter (Return)
    0x001b, 0x001b,  // 0x29  41  Escape
    0x0008, 0x0008,  // 0x2a  42  Backspace (Delete)
    0x0009, 0x0009,  // 0x2b  43  Tab
       ' ',    ' ',  // 0x2c  44  Spacebar
       '-',    '_',  // 0x2d  45  - _
       '=',    '+',  // 0x2e  46  = +
       '[',    '{',  // 0x2f  47  [ {
       ']',    '}',  // 0x30  48  ] }
       '#',    '~',  // 0x31  49  US \ bar
       '#',    '~',  // 0x32  50  # ~
       ';',    ':',  // 0x33  51  ; :
      '\'',    '@',  // 0x34  52  ' @
       '`', 0x00ac,  // 0x35  53  Grave/not/bar
       ',',    '<',  // 0x36  54  , <
       '.',    '>',  // 0x37  55  . >
       '/',    '?',  // 0x38  56  / ?
         0,      0,  // 0x39  57  Caps Lock
    0xf816, 0xf816,  // 0x3a  58  F1
    0xf817, 0xf817,  // 0x3b  59  F2
    0xf818, 0xf818,  // 0x3c  60  F3
    0xf819, 0xf819,  // 0x3d  61  F4
    0xf81a, 0xf81a,  // 0x3e  62  F5
    0xf81b, 0xf81b,  // 0x3f  63  F6
    0xf81c, 0xf81c,  // 0x40  64  F7
    0xf81d, 0xf81d,  // 0x41  65  F8
    0xf81e, 0xf81e,  // 0x42  66  F9
    0xf81f, 0xf81f,  // 0x43  67  F10
    0xf820, 0xf820,  // 0x44  68  F11
    0xf821, 0xf821,  // 0x45  69  F12
    0xf800, 0xf800,  // 0x46  70  Print Screen
    0xf815, 0xf815,  // 0x47  71  Scroll Lock
    0xf801, 0xf801,  // 0x48  72  Break / Pause
    0xf806, 0xf806,  // 0x49  73  Insert
    0xf802, 0xf802,  // 0x4a  74  Home
    0xf804, 0xf804,  // 0x4b  75  Page Up
    0x007f, 0x007f,  // 0x4c  76  Delete
    0xf803, 0xf803,  // 0x4d  77  End
    0xf805, 0xf805,  // 0x4e  78  Page Down
    0xf808, 0xf808,  // 0x4f  79  Right Arrow
    0xf807, 0xf807,  // 0x50  80  Left Arrow
    0xf80a, 0xf80a,  // 0x51  81  Down Arrow
    0xf809, 0xf809,  // 0x52  82  Up Arrow
         0,      0,  // 0x53  83  Num Lock
       '/',    '/',  // 0x54  84  Keypad /
       '*',    '*',  // 0x55  85  Keypad *
       '-',    '-',  // 0x56  86  Keypad -
       '+',    '+',  // 0x57  87  Keypad +
    0x000d, 0xf845,  // 0x58  88  Keypad Enter
       '1', 0xf803,  // 0x59  89  Keypad 1 End
       '2', 0xf80a,  // 0x5a  90  Keypad 2 Down
       '3', 0xf805,  // 0x5b  91  Keypad 3 PageDn
       '4', 0xf807,  // 0x5c  92  Keypad 4 Left
       '5',    ' ',  // 0x5d  93  Keypad 5
       '6', 0xf808,  // 0x5e  94  Keypad 6 Right
       '7', 0xf802,  // 0x5f  95  Keypad 7 Home
       '8', 0xf809,  // 0x60  96  Keypad 8 Up
       '9', 0xf804,  // 0x61  97  Keypad 9 PageUp
       '0', 0xf806,  // 0x62  98  Keypad 0 Insert
       '.', 0x007f,  // 0x63  99  Keypad . Delete
      '\\',    '|',  // 0x64 100  \ | (near left shift)
    0xf843, 0xf863,  // 0x65 101  Application key
         0,      0,  // 0x66 102  Keyboard power
       '=',    '=',  // 0x67 103  Keypad =
    };

// ----------------------------------------------------------------------

const CStandardKeyboardLayout::TSpecialKey CUnitedKingdomLayout::KSpecialCases[] =
    {
    // HID usage, modifiers, Unicode               // Usage name (UK)
    { 0x28, THidModifier::ECtrl,         0xf845 }, // Enter (Return)
    { 0x2c, THidModifier::ECtrl,         0x0020 }, // Spacebar
    { 0x58, THidModifier::ECtrl,         0xf845 }, // Keypad Enter
    { 0x2b, THidModifier::EAlt,          0xf852 }, // Tab
    { 0x04, THidModifier::EAltCtrl,      0x00e1 }, // a A
    { 0x08, THidModifier::EAltCtrl,      0x00e9 }, // e E
    { 0x0c, THidModifier::EAltCtrl,      0x00ed }, // i I
    { 0x12, THidModifier::EAltCtrl,      0x00f3 }, // o O
    { 0x18, THidModifier::EAltCtrl,      0x00fa }, // u U
    { 0x21, THidModifier::EAltCtrl,      0x20ac }, // 4 $
    { 0x35, THidModifier::EAltCtrl,      0x00a6 }, // Grave/not/bar
    { 0x4c, THidModifier::EAltCtrl,      0xf844 }, // Delete
    { 0x63, THidModifier::EAltCtrl,      0xf844 }, // Keypad . Delete
    { 0x04, THidModifier::EAltCtrlShift, 0x00c1 }, // a A
    { 0x08, THidModifier::EAltCtrlShift, 0x00c9 }, // e E
    { 0x0c, THidModifier::EAltCtrlShift, 0x00cd }, // i I
    { 0x12, THidModifier::EAltCtrlShift, 0x00d3 }, // o O
    { 0x18, THidModifier::EAltCtrlShift, 0x00da }, // u U
    { 0xe3, THidModifier::EFunc,         0xf842 }, // Left GUI key
    { 0xe7, THidModifier::EFunc,         0xf843 }, // Right GUI key
    { 0xe3, THidModifier::EFuncShift,    0xf862 }, // Left GUI key
    { 0xe7, THidModifier::EFuncShift,    0xf863 }, // Right GUI key
    { 0x00 } // end of table
    };

// ----------------------------------------------------------------------

CUnitedKingdomLayout::CUnitedKingdomLayout()
    : CStandardKeyboardLayout(KColumnMap, sizeof (KColumnMap),
        KKeyCodes, sizeof (KKeyCodes), KSpecialCases)
    {
    // nothing else to do
    }

TBool CUnitedKingdomLayout::ChangesWithCapsLock(TInt aHidKey,
    THidModifier aModifiers) const
    {
    if (aModifiers.ControlAlt())
        {
        // Allow caps lock to invert the shift state for these ctrl-alt keys:
        //
        TBool isAltGrCapsKey = 
            (aHidKey == 0x04) ||  // a A
            (aHidKey == 0x08) ||  // e E
            (aHidKey == 0x0c) ||  // i I
            (aHidKey == 0x12) ||  // o O
            (aHidKey == 0x18);    // u U

        if (isAltGrCapsKey)
            return ETrue;
        }

    return CStandardKeyboardLayout::ChangesWithCapsLock(aHidKey, aModifiers);
    }

TInt CUnitedKingdomLayout::LayoutId() const
    {
    DBG(RDebug::Print(_L("[HID]\tCUnitedKingdomLayout::LayoutId()")));
    return EUnitedKingdom;
    }

CKeyboardLayout* CUnitedKingdomLayout::NewL()
    {
    DBG(RDebug::Print(_L("[HID]\tCUnitedKingdomLayout::NewL()")));
    CKeyboardLayout* layout = new (ELeave) CUnitedKingdomLayout;
    return layout;
    }

// End of file

