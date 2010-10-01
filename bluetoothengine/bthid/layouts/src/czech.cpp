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


#include "czech.h"
#include "hiddebug.h"
#include "hidlayoutids.h"

// CONSTANTS

// Look-up tables to translate HID keyboard usage codes (usage page 7)
// to the corresponding Symbian "TKeyCode" (Unicode) values:

const TInt CCzechLayout::KColumnMap[] =
    {
    THidModifier::EUnmodified,  // ----
    THidModifier::EShift,       // ---S
    THidModifier::EAltCtrl,     // -AC-
    THidModifier::EAltCtrlShift // -ACS
    };

const TUint16 CCzechLayout::KKeyCodes[] = 
    {
    //----    ---S    -AC-    -ACS   // Hex  Dec  Usage name (UK)
         0,      0,      0,      0,  // 0x00   0  Reserved (no event)
         0,      0,      0,      0,  // 0x01   1  Rollover error
         0,      0,      0,      0,  // 0x02   2  POST Fail
         0,      0,      0,      0,  // 0x03   3  Undefined error
       'a',    'A', 0x007e, 0x00c6,  // 0x04   4  a A
       'b',    'B', 0x007b, 0x2019,  // 0x05   5  b B
       'c',    'C', 0x0026, 0x00a9,  // 0x06   6  c C
       'd',    'D', 0x0110, 0x00d0,  // 0x07   7  d D
       'e',    'E', 0x20ac,      0,  // 0x08   8  e E
       'f',    'F', 0x005b, 0x00aa,  // 0x09   9  f F
       'g',    'G', 0x005d, 0x014a,  // 0x0a  10  g G
       'h',    'H', 0x0060, 0x0126,  // 0x0b  11  h H
       'i',    'I', 0x2192, 0x0131,  // 0x0c  12  i I
       'j',    'J', 0x0027,      0,  // 0x0d  13  j J
       'k',    'K', 0x0142, 0x0026,  // 0x0e  14  k K
       'l',    'L', 0x0141, 0x0141,  // 0x0f  15  l L
       'm',    'M', 0x005e, 0x00ba,  // 0x10  16  m M
       'n',    'N', 0x007d,      0,  // 0x11  17  n N
       'o',    'O', 0x00f8, 0x00d8,  // 0x12  18  o O
       'p',    'P', 0x00fe, 0x00de,  // 0x13  19  p P
       'q',    'Q', 0x005c, 0x03a9,  // 0x14  20  q Q
       'r',    'R', 0x00b6, 0x00ae,  // 0x15  21  r R
       's',    'S', 0x0111, 0x00a7,  // 0x16  22  s S
       't',    'T', 0x0167, 0x0166,  // 0x17  23  t T
       'u',    'U', 0x2193, 0x2191,  // 0x18  24  u U
       'v',    'V', 0x0040, 0x2018,  // 0x19  25  v V
       'w',    'W', 0x007c, 0x0141,  // 0x1a  26  w W
       'x',    'X', 0x0023, 0x003e,  // 0x1b  27  x X
       'z',    'Z', 0x2190, 0x00a5,  // 0x1c  28  y Y
       'y',    'Y', 0x00b0, 0x003c,  // 0x1d  29  z Z
       '+',    '1', 0x0021,      0,  // 0x1e  30  1 ! // TODO 0xf606 dead tilde
    0x011b,    '2', 0x0040, 0xf602,  // 0x1f  31  2 @
    0x0161,    '3', 0x0023, 0xf605,  // 0x20  32  3 #
    0x010d,    '4', 0x0024,      0,  // 0x21  33  4 $ // TODO 0xf607 dead breve
    0x0159,    '5', 0x0025, 0xf604,  // 0x22  34  5 %
    0x017e,    '6', 0x005e,      0,  // 0x23  35  6 ^ // TODO 0xf608 dead ogonek
    0x00fd,    '7', 0x0026,      0,  // 0x24  36  7 & // TODO 0xf609 dead grave
    0x00e1,    '8', 0x002a,      0,  // 0x25  37  8 * // TODO 0xf60a dead abovedot
    0x00ed,    '9', 0x007b, 0xf601,  // 0x26  38  9 (
    0x00e9,    '0', 0x007d,      0,  // 0x27  39  0 ) // TODO 0xf60b dead doubleacute
    0x000d, 0xf845,      0,      0,  // 0x28  40  Enter (Return)
    0x001b, 0x001b,      0,      0,  // 0x29  41  Escape
    0x0008, 0x0008,      0,      0,  // 0x2a  42  Backspace (Delete)
    0x0009, 0x0009,      0,      0,  // 0x2b  43  Tab
       ' ',    ' ',      0,      0,  // 0x2c  44  Spacebar
       '=',    '%',   '\\', 0xf602,  // 0x2d  45  - _
    0xf601, 0xf602,      0,      0,  // 0x2e  46  = + // TODO 0xf60c dead macron, 0xf60d dead cedilla
    0x00fa, 0x002f, 0x005b, 0x00f7,  // 0x2f  47  [ {
    0x0029, 0x0028, 0x005d, 0x00d7,  // 0x30  48  ] }
    0xf603,   '\'',   '\\',    '|',  // 0x31  49  US \ bar
    0xf603,   '\'',   '\\',    '|',  // 0x32  50  # ~
    0x016f, 0x0022, 0x0024,      0,  // 0x33  51  ; : // TODO 0xf60b dead doubleacute
    0x00a7, 0x0021, 0x0027, 0x00df,  // 0x34  52  ' @
    0x003b, 0xf604, 0x0060, 0x007e,  // 0x35  53  Grave/not/bar
       ',',    '?', 0x003c, 0x00d7,  // 0x36  54  , <
       '.',    ':', 0x003e, 0x00f7,  // 0x37  55  . >
       '-',    '_',    '*',      0,  // 0x38  56  / ? // TODO 0xf60a dead abovedot
    0x001b, 0x001b,      0,      0,  // 0x39  57  Caps Lock
    0xf816, 0xf816,      0,      0,  // 0x3a  58  F1
    0xf817, 0xf817,      0,      0,  // 0x3b  59  F2
    0xf818, 0xf818,      0,      0,  // 0x3c  60  F3
    0xf819, 0xf819,      0,      0,  // 0x3d  61  F4
    0xf81a, 0xf81a,      0,      0,  // 0x3e  62  F5
    0xf81b, 0xf81b,      0,      0,  // 0x3f  63  F6
    0xf81c, 0xf81c,      0,      0,  // 0x40  64  F7
    0xf81d, 0xf81d,      0,      0,  // 0x41  65  F8
    0xf81e, 0xf81e,      0,      0,  // 0x42  66  F9
    0xf81f, 0xf81f,      0,      0,  // 0x43  67  F10
    0xf820, 0xf820,      0,      0,  // 0x44  68  F11
    0xf821, 0xf821,      0,      0,  // 0x45  69  F12
    0xf800, 0xf800,      0,      0,  // 0x46  70  Print Screen
    0xf815, 0xf815,      0,      0,  // 0x47  71  Scroll Lock
    0xf801, 0xf801,      0,      0,  // 0x48  72  Break / Pause
    0xf806, 0xf806,      0,      0,  // 0x49  73  Insert
    0xf802, 0xf802,      0,      0,  // 0x4a  74  Home
    0xf804, 0xf804,      0,      0,  // 0x4b  75  Page Up
    0x007f, 0x007f, 0xf844,      0,  // 0x4c  76  Delete
    0xf803, 0xf803,      0,      0,  // 0x4d  77  End
    0xf805, 0xf805,      0,      0,  // 0x4e  78  Page Down
    0xf808, 0xf808,      0,      0,  // 0x4f  79  Right Arrow
    0xf807, 0xf807,      0,      0,  // 0x50  80  Left Arrow
    0xf80a, 0xf80a,      0,      0,  // 0x51  81  Down Arrow
    0xf809, 0xf809,      0,      0,  // 0x52  82  Up Arrow
         0,      0,      0,      0,  // 0x53  83  Num Lock
       '/',    '/',      0,      0,  // 0x54  84  Keypad /
       '*',    '*',      0,      0,  // 0x55  85  Keypad *
       '-',    '-',      0,      0,  // 0x56  86  Keypad -
       '+',    '+',      0,      0,  // 0x57  87  Keypad +
    0x000d, 0xf845,      0,      0,  // 0x58  88  Keypad Enter
       '1', 0xf803,      0,      0,  // 0x59  89  Keypad 1 End
       '2', 0xf80a,      0,      0,  // 0x5a  90  Keypad 2 Down
       '3', 0xf805,      0,      0,  // 0x5b  91  Keypad 3 PageDn
       '4', 0xf807,      0,      0,  // 0x5c  92  Keypad 4 Left
       '5',    ' ',      0,      0,  // 0x5d  93  Keypad 5
       '6', 0xf808,      0,      0,  // 0x5e  94  Keypad 6 Right
       '7', 0xf802,      0,      0,  // 0x5f  95  Keypad 7 Home
       '8', 0xf809,      0,      0,  // 0x60  96  Keypad 8 Up
       '9', 0xf804,      0,      0,  // 0x61  97  Keypad 9 PageUp
       '0', 0xf806,      0,      0,  // 0x62  98  Keypad 0 Insert
       '.', 0x007f, 0xf844,      0,  // 0x63  99  Keypad . Delete
      '\\',    '|',      0,      0,  // 0x64 100  \ | (near left shift)
    0xf843, 0xf863,      0,      0,  // 0x65 101  Application key
         0,      0,      0,      0,  // 0x66 102  Keyboard power
       '=',    '=',      0,      0,  // 0x67 103  Keypad =
    };

// ----------------------------------------------------------------------

const CStandardKeyboardLayout::TSpecialKey CCzechLayout::KSpecialCases[] =
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

#if 0
-- Haskell code to generate dead key mappings

import Data.Char
import Text.Printf

map2 f (x:y:xs) = f x y : map2 f xs
map2 _ _        = []

f :: String -> String -> String
f [dead_key] = concat . map2 (\x y -> printf "    { 0x%.4x, 0x%.4x },    // %c + %c = %c\n" (ord x) (ord y) dead_key x y)

-- ghci> putStr $ f "'" " 'wẃeérŕ…"
#endif

#if 0
const CDeadKeyLayout::TKeyPair CCzechLayout::KDeadKey…[] =
    {
    { 0x0000 }             // end of table
    };
#endif

const CDeadKeyLayout::TKeyPair CCzechLayout::KDeadKeyAcute[] =
    {
    { 0x0020, 0x0027 },    // ' +   = '
    { 0x0077, 0x1e83 },    // ' + w = ẃ
    { 0x0065, 0x00e9 },    // ' + e = é
    { 0x0072, 0x0155 },    // ' + r = ŕ
    { 0x007a, 0x017a },    // ' + z = ź
    { 0x0075, 0x00fa },    // ' + u = ú
    { 0x0069, 0x00ed },    // ' + i = í
    { 0x006f, 0x00f3 },    // ' + o = ó
    { 0x0070, 0x1e55 },    // ' + p = ṕ
    { 0x0061, 0x00e1 },    // ' + a = á
    { 0x0073, 0x015b },    // ' + s = ś
    { 0x0067, 0x01f5 },    // ' + g = ǵ
    { 0x006b, 0x1e31 },    // ' + k = ḱ
    { 0x006c, 0x013a },    // ' + l = ĺ
    { 0x0079, 0x00fd },    // ' + y = ý
    { 0x0063, 0x0107 },    // ' + c = ć
    { 0x006e, 0x0144 },    // ' + n = ń
    { 0x006d, 0x1e3f },    // ' + m = ḿ
    { 0x0057, 0x1e82 },    // ' + W = Ẃ
    { 0x0045, 0x00c9 },    // ' + E = É
    { 0x0052, 0x0154 },    // ' + R = Ŕ
    { 0x005a, 0x0179 },    // ' + Z = Ź
    { 0x0055, 0x00da },    // ' + U = Ú
    { 0x0049, 0x00cd },    // ' + I = Í
    { 0x004f, 0x00d3 },    // ' + O = Ó
    { 0x0050, 0x1e54 },    // ' + P = Ṕ
    { 0x0041, 0x00c1 },    // ' + A = Á
    { 0x0053, 0x015a },    // ' + S = Ś
    { 0x0047, 0x01f4 },    // ' + G = Ǵ
    { 0x004b, 0x1e30 },    // ' + K = Ḱ
    { 0x004c, 0x0139 },    // ' + L = Ĺ
    { 0x0059, 0x00dd },    // ' + Y = Ý
    { 0x0043, 0x0106 },    // ' + C = Ć
    { 0x004e, 0x0143 },    // ' + N = Ń
    { 0x004d, 0x1e3e },    // ' + M = Ḿ
    { 0x0000 }             // end of table
    };

const CDeadKeyLayout::TKeyPair CCzechLayout::KDeadKeyCaron[] =
    {
    { 0x0020, 0x02c7 },    // ˇ +   = ˇ
    { 0x0065, 0x011b },    // ˇ + e = ě
    { 0x0072, 0x0159 },    // ˇ + r = ř
    { 0x0074, 0x0165 },    // ˇ + t = ť
    { 0x007a, 0x017e },    // ˇ + z = ž
    { 0x0075, 0x01d4 },    // ˇ + u = ǔ
    { 0x0069, 0x01d0 },    // ˇ + i = ǐ
    { 0x006f, 0x01d2 },    // ˇ + o = ǒ
    { 0x0061, 0x01ce },    // ˇ + a = ǎ
    { 0x0073, 0x0161 },    // ˇ + s = š
    { 0x0064, 0x010f },    // ˇ + d = ď
    { 0x0067, 0x01e7 },    // ˇ + g = ǧ
    { 0x0068, 0x021f },    // ˇ + h = ȟ
    { 0x006a, 0x01f0 },    // ˇ + j = ǰ
    { 0x006b, 0x01e9 },    // ˇ + k = ǩ
    { 0x006c, 0x013e },    // ˇ + l = ľ
    { 0x0063, 0x010d },    // ˇ + c = č
    { 0x006e, 0x0148 },    // ˇ + n = ň
    { 0x0045, 0x011a },    // ˇ + E = Ě
    { 0x0052, 0x0158 },    // ˇ + R = Ř
    { 0x0054, 0x0164 },    // ˇ + T = Ť
    { 0x005a, 0x017d },    // ˇ + Z = Ž
    { 0x0055, 0x01d3 },    // ˇ + U = Ǔ
    { 0x0049, 0x01cf },    // ˇ + I = Ǐ
    { 0x004f, 0x01d1 },    // ˇ + O = Ǒ
    { 0x0041, 0x01cd },    // ˇ + A = Ǎ
    { 0x0053, 0x0160 },    // ˇ + S = Š
    { 0x0044, 0x010e },    // ˇ + D = Ď
    { 0x0047, 0x01e6 },    // ˇ + G = Ǧ
    { 0x0048, 0x021e },    // ˇ + H = Ȟ
    { 0x004b, 0x01e8 },    // ˇ + K = Ǩ
    { 0x004c, 0x013d },    // ˇ + L = Ľ
    { 0x0043, 0x010c },    // ˇ + C = Č
    { 0x0055, 0x01d9 },    // ˇ + U = Ǚ
    { 0x004e, 0x0147 },    // ˇ + N = Ň
    { 0x0000 }             // end of table
    };

const CDeadKeyLayout::TKeyPair CCzechLayout::KDeadKeyDiaeresis[] =
    {
    { 0x0020, 0x0022 },    // " +   = "
    { 0x0077, 0x1e85 },    // " + w = ẅ
    { 0x0065, 0x00eb },    // " + e = ë
    { 0x0074, 0x1e97 },    // " + t = ẗ
    { 0x0075, 0x00fc },    // " + u = ü
    { 0x0069, 0x00ef },    // " + i = ï
    { 0x006f, 0x00f6 },    // " + o = ö
    { 0x0061, 0x00e4 },    // " + a = ä
    { 0x0068, 0x1e27 },    // " + h = ḧ
    { 0x0079, 0x00ff },    // " + y = ÿ
    { 0x0078, 0x1e8d },    // " + x = ẍ
    { 0x0057, 0x1e84 },    // " + W = Ẅ
    { 0x0045, 0x00cb },    // " + E = Ë
    { 0x0055, 0x00dc },    // " + U = Ü
    { 0x0049, 0x00cf },    // " + I = Ï
    { 0x004f, 0x00d6 },    // " + O = Ö
    { 0x0041, 0x00c4 },    // " + A = Ä
    { 0x0048, 0x1e26 },    // " + H = Ḧ
    { 0x0059, 0x0178 },    // " + Y = Ÿ
    { 0x0058, 0x1e8c },    // " + X = Ẍ
    { 0x0000 }             // end of table
    };

const CDeadKeyLayout::TKeyPair CCzechLayout::KDeadKeyAbovering[] =
    {
    { 0x0020, 0x00b0 },    // ° +   = °
    { 0x0077, 0x1e98 },    // ° + w = ẘ
    { 0x0075, 0x016f },    // ° + u = ů
    { 0x0061, 0x00e5 },    // ° + a = å
    { 0x0079, 0x1e99 },    // ° + y = ẙ
    { 0x004f, 0x016e },    // ° + O = Ů
    { 0x0041, 0x00c5 },    // ° + A = Å
    { 0x0000 }             // end of table
    };

const CDeadKeyLayout::TKeyPair CCzechLayout::KDeadKeyCircumflex[] =
    {
    { 0x0020, 0x005e },    // ^ +   = ^
    { 0x0077, 0x0175 },    // ^ + w = ŵ
    { 0x0065, 0x00ea },    // ^ + e = ê
    { 0x007a, 0x1e91 },    // ^ + z = ẑ
    { 0x0075, 0x00fb },    // ^ + u = û
    { 0x0069, 0x00ee },    // ^ + i = î
    { 0x006f, 0x00f4 },    // ^ + o = ô
    { 0x0061, 0x00e2 },    // ^ + a = â
    { 0x0073, 0x015d },    // ^ + s = ŝ
    { 0x0067, 0x011d },    // ^ + g = ĝ
    { 0x0068, 0x0125 },    // ^ + h = ĥ
    { 0x006a, 0x0135 },    // ^ + j = ĵ
    { 0x0079, 0x0177 },    // ^ + y = ŷ
    { 0x0063, 0x0109 },    // ^ + c = ĉ
    { 0x0057, 0x0174 },    // ^ + W = Ŵ
    { 0x0045, 0x00ca },    // ^ + E = Ê
    { 0x005a, 0x1e90 },    // ^ + Z = Ẑ
    { 0x0055, 0x00db },    // ^ + U = Û
    { 0x0049, 0x00ce },    // ^ + I = Î
    { 0x004f, 0x00d4 },    // ^ + O = Ô
    { 0x0041, 0x00c2 },    // ^ + A = Â
    { 0x0053, 0x015c },    // ^ + S = Ŝ
    { 0x0047, 0x011c },    // ^ + G = Ĝ
    { 0x0048, 0x0124 },    // ^ + H = Ĥ
    { 0x004a, 0x0134 },    // ^ + J = Ĵ
    { 0x0059, 0x0176 },    // ^ + Y = Ŷ
    { 0x0043, 0x0108 },    // ^ + C = Ĉ
    { 0x0000 }             // end of table
    };

const CDeadKeyLayout::TIndexPair CCzechLayout::KDeadKeyIndex[] =
    {
    { 0xf601, KDeadKeyAcute },
    { 0xf602, KDeadKeyCaron },
    { 0xf603, KDeadKeyDiaeresis },
    { 0xf604, KDeadKeyAbovering },
    { 0xf605, KDeadKeyCircumflex },
    { 0x0000 } // end of table
    };

// ----------------------------------------------------------------------

CCzechLayout::CCzechLayout()
    : CDeadKeyLayout(KColumnMap, sizeof (KColumnMap),
        KKeyCodes, sizeof (KKeyCodes), KSpecialCases, KDeadKeyIndex)
    {
    // nothing else to do
    }

TInt CCzechLayout::LayoutId() const
    {
    DBG(RDebug::Print(_L("[HID]\tCCzechLayout::LayoutId()")));
    return 0; // TODO
    }

CKeyboardLayout* CCzechLayout::NewL()
    {
    DBG(RDebug::Print(_L("[HID]\tCCzechLayout::NewL()")));
    CKeyboardLayout* layout = new (ELeave) CCzechLayout;
    return layout;
    }

// End of file

