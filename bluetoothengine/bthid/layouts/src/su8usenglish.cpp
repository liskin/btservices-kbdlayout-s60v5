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


#include "su8usenglish.h"
#include "hiddebug.h"
#include "hidlayoutids.h"

// CONSTANTS

// Look-up tables to translate HID keyboard usage codes (usage page 7)
// to the corresponding Symbian "TKeyCode" (Unicode) values:

const TInt CUSInternationalLayout::KColumnMap[] =
    {
    THidModifier::EUnmodified,  // ----
    THidModifier::EShift,       // ---S
    THidModifier::EAltCtrl,     // -AC-
    THidModifier::EAltCtrlShift // -ACS
    };

const TUint16 CUSInternationalLayout::KKeyCodes[] = 
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
    
/* OLD AND ORIGINAL SU-8W ENGLISH LAYOUT
const TInt CSu8UnitedStatesLayout::KColumnMap[] =
    {
    THidModifier::EUnmodified,  // ----
    THidModifier::EShift        // ---S
    };

const TUint16 CSu8UnitedStatesLayout::KKeyCodes[] = 
    {
    //----    ---S   // Hex  Dec  Usage name (UK)
         0,      0,  // 0x00   0  Reserved (no event)
         0,      0,  // 0x01   1  Rollover error
         0,      0,  // 0x02   2  POST Fail
         0,      0,  // 0x03   3  Undefined error
       'a',    'A',  // 0x04   4  a
       'b',    'B',  // 0x05   5  b
       'c',    'C',  // 0x06   6  c
       'd',    'D',  // 0x07   7  d
       'e',    'E',  // 0x08   8  e
       'f',    'F',  // 0x09   9  f
       'g',    'G',  // 0x0a  10  g
       'h',    'H',  // 0x0b  11  h
       'i',    'I',  // 0x0c  12  i
       'j',    'J',  // 0x0d  13  j
       'k',    'K',  // 0x0e  14  k
       'l',    'L',  // 0x0f  15  l
       'm',    'M',  // 0x10  16  m
       'n',    'N',  // 0x11  17  n
       'o',    'O',  // 0x12  18  o
       'p',    'P',  // 0x13  19  p
       'q',    'Q',  // 0x14  20  q
       'r',    'R',  // 0x15  21  r
       's',    'S',  // 0x16  22  s
       't',    'T',  // 0x17  23  t
       'u',    'U',  // 0x18  24  u
       'v',    'V',  // 0x19  25  v
       'w',    'W',  // 0x1a  26  w
       'x',    'X',  // 0x1b  27  x
       'y',    'Y',  // 0x1c  28  y
       'z',    'Z',  // 0x1d  29  z
       '1',    '!',  // 0x1e  30  1
       '2',    '@',  // 0x1f  31  2
       '3',    '#',  // 0x20  32  3
       '4',    '$',  // 0x21  33  4
       '5',    '%',  // 0x22  34  5
       '6',    '^',  // 0x23  35  6
       '7',    '&',  // 0x24  36  7
       '8',    '*',  // 0x25  37  8
       '9',    '(',  // 0x26  38  9
       '0',    ')',  // 0x27  39  0
    0x000d, 0xf845,  // 0x28  40  Enter
    0x001b, 0x001b,  // 0x29  41  Escape
    0x0008, 0x0008,  // 0x2a  42  Backspace
    0x0009, 0x0009,  // 0x2b  43  Tab
       ' ',    ' ',  // 0x2c  44  Space
       '-',    '_',  // 0x2d  45
       '=',    '+',  // 0x2e  46
       '[',    '{',  // 0x2f  47
       ']',    '}',  // 0x30  48
      '\\',    '|',  // 0x31  49
         0,      0,  // 0x32  50
       ';',    ':',  // 0x33  51
      '\'',    '"',  // 0x34  52
       '`',    '~',  // 0x35  53
       ',',    '<',  // 0x36  54
       '.',    '>',  // 0x37  55
       '/',    '?'   // 0x38  56
    };
*/
// ----------------------------------------------------------------------

const CStandardKeyboardLayout::TSpecialKey 
    CSu8UnitedStatesLayout::KSpecialCases[] =
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
    { 0xe3, THidModifier::EFuncShift,  0xf862 }, // Shift-left GUI key
    { 0xe7, THidModifier::EFuncShift,  0xf863 }, // Shift-right GUI key
    { 0x76, THidModifier::EUnmodified, 0xf852 }, // App switch
    { 0x76, THidModifier::EShift,      0xf852 }, // Shift-app switch
    { 0x77, THidModifier::EUnmodified, 0xf845 }, // MSK to OK
    { 0x77, THidModifier::EShift,      0xf845 }, // Shift-MSK to OK
    //
    { 0x00 } // end of table
    };

// ----------------------------------------------------------------------

CSu8UnitedStatesLayout::CSu8UnitedStatesLayout()
    : CStandardKeyboardLayout(KColumnMap, sizeof (KColumnMap),
        KKeyCodes, sizeof (KKeyCodes), KSpecialCases)
    {
    // nothing else to do
    }


TInt CSu8UnitedStatesLayout::RawScanCode(TInt aHidKey, 
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
        result = CStandardKeyboardLayout::RawScanCode(aHidKey,
            aUsagePage, aModifiers);
        }
    
    return result;
    }

TInt CSu8UnitedStatesLayout::LayoutId() const
    {
    DBG(RDebug::Print(_L("[HID]\tCSu8UnitedStatesLayout::LayoutId()")));
    return ESu8IntRussian;
    }

CKeyboardLayout* CSu8UnitedStatesLayout::NewL()
    {
    DBG(RDebug::Print(_L("[HID]\tCSu8UnitedStatesLayout::NewL()")));
    CKeyboardLayout* layout = new (ELeave) CSu8UnitedStatesLayout;
    return layout;
    }

// End of file

