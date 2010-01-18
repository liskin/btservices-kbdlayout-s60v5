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


#include "su8russian.h"
#include "hiddebug.h"
#include "hidlayoutids.h"

// CONSTANTS

// Look-up tables to translate HID keyboard usage codes (usage page 7)
// to the corresponding Symbian "TKeyCode" (Unicode) values:

const TInt CSu8RussianLayout::KColumnMap[] =
    {
    THidModifier::EUnmodified,  // ----
    THidModifier::EShift        // ---S
    };

const TUint16 CSu8RussianLayout::KKeyCodes[] = 
    {
    //----    ---S   // Hex  Dec  Usage name (UK)
         0,      0,  // 0x00   0  Reserved (no event)
         0,      0,  // 0x01   1  Rollover error
         0,      0,  // 0x02   2  POST Fail
         0,      0,  // 0x03   3  Undefined error
       0x444,    0x424,  // 0x04   4  a
       0x438,    0x418,  // 0x05   5  b
       'c',    'C',  // 0x06   6  c
       0x432,    0x412,  // 0x07   7  d
       0x443,    0x423,  // 0x08   8  e
       0x430,    0x410,  // 0x09   9  f
       0x43f,    0x41f,  // 0x0a  10  g
       0x440,    0x420,  // 0x0b  11  h
       0x448,    0x428,  // 0x0c  12  i
       0x43e,    0x41e,  // 0x0d  13  j
       0x43b,    0x41b,  // 0x0e  14  k
       0x434,    0x414,  // 0x0f  15  l
       0x44c,    0x42c,  // 0x10  16  m
       0x442,    0x422,  // 0x11  17  n
       0x449,    0x429,  // 0x12  18  o
       0x437,    0x417,  // 0x13  19  p
       0x439,    0x419,  // 0x14  20  q
       0x43a,    0x41a,  // 0x15  21  r
       0x44b,    0x42b,  // 0x16  22  s
       0x435,    0x415,  // 0x17  23  t
       0x433,    0x413,  // 0x18  24  u
       0x43c,    0x41c,  // 0x19  25  v
       0x446,    0x426,  // 0x1a  26  w
       0x447,    0x427,  // 0x1b  27  x
       0x43d,    0x41d,  // 0x1c  28  y
       0x44f,    0x42f,  // 0x1d  29  z
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
       0x445,    0x425,  // 0x2f  47
       0x44a,    0x42a,  // 0x30  48
      '\\',    '|',  // 0x31  49
         0,      0,  // 0x32  50
       0x436,    0x416,  // 0x33  51
       0x44d,    0x42d,  // 0x34  52
       0x451,    0x401,  // 0x35  53
       0x431,    0x411,  // 0x36  54
       0x44e,    0x42e,  // 0x37  55
       '/',    '?'   // 0x38  56
    };

// ----------------------------------------------------------------------

const CStandardKeyboardLayout::TSpecialKey 
    CSu8RussianLayout::KSpecialCases[] =
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

CSu8RussianLayout::CSu8RussianLayout()
    : CStandardKeyboardLayout(KColumnMap, sizeof (KColumnMap),
        KKeyCodes, sizeof (KKeyCodes), KSpecialCases)
    {
    // nothing else to do
    }

// ----------------------------------------------------------------------

TInt CSu8RussianLayout::RawScanCode(TInt aHidKey, 
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

TInt CSu8RussianLayout::LayoutId() const
    {
    DBG(RDebug::Print(_L("[HID]\tCSu8RussianLayout::LayoutId()")));
    return ESu8Russian;
    }

CKeyboardLayout* CSu8RussianLayout::NewL()
    {
    DBG(RDebug::Print(_L("[HID]\tCSu8RussianLayout::NewL()")));
    CKeyboardLayout* layout = new (ELeave) CSu8RussianLayout;
    return layout;
    }

// End of file

