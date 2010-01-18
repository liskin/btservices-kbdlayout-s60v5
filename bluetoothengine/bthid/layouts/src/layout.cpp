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


#include <e32std.h>
#include <e32svr.h>
#include <e32keys.h>
#include <coedef.h>   // for CTRL()

//#include "hidkeys.h"   // included from decode.h
#include "layout.h"
#include "decode.h"
#include "modifier.h"
#include "hiddebug.h"

#include "hidvalues.h"

// ----------------------------------------------------------------------
// Map HID keyboard usage values (usage page 7) to EPOC raw "scan" codes:

const TInt CKeyboardLayout::KRawCodes[] = 
    {
    EStdKeyNull,               // 0x00  0   No Event
    EStdKeyNull,               // 0x01  1   Overrun Error
    EStdKeyNull,               // 0x02  2   POST Fail
    EStdKeyNull,               // 0x03  3   ErrorUndefined
    'A',                       // 0x04  4   a A
    'B',                       // 0x05  5   b B
    'C',                       // 0x06  6   c C
    'D',                       // 0x07  7   d D
    'E',                       // 0x08  8   e E
    'F',                       // 0x09  9   f F
    'G',                       // 0x0A  10  g G
    'H',                       // 0x0B  11  h H
    'I',                       // 0x0C  12  i I
    'J',                       // 0x0D  13  j J
    'K',                       // 0x0E  14  k K
    'L',                       // 0x0F  15  l L
    'M',                       // 0x10  16  m M
    'N',                       // 0x11  17  n N
    'O',                       // 0x12  18  o O
    'P',                       // 0x13  19  p P
    'Q',                       // 0x14  20  q Q
    'R',                       // 0x15  21  r R
    'S',                       // 0x16  22  s S
    'T',                       // 0x17  23  t T
    'U',                       // 0x18  24  u U
    'V',                       // 0x19  25  v V
    'W',                       // 0x1A  26  w W
    'X',                       // 0x1B  27  x X
    'Y',                       // 0x1C  28  y Y
    'Z',                       // 0x1D  29  z Z
    '1',                       // 0x1E  30  1 !
    '2',                       // 0x1F  31  2 @
    '3',                       // 0x20  32  3 #
    '4',                       // 0x21  33  4 $ Euro
    '5',                       // 0x22  34  5 %
    '6',                       // 0x23  35  6 ^
    '7',                       // 0x24  36  7 &
    '8',                       // 0x25  37  8 *
    '9',                       // 0x26  38  9 (
    '0',                       // 0x27  39  0 )
    EStdKeyEnter,              // 0x28  40  Return
    EStdKeyEscape,             // 0x29  41  Escape
    EStdKeyBackspace,          // 0x2A  42  Backspace
    EStdKeyTab,                // 0x2B  43  Tab
    EStdKeySpace,              // 0x2C  44  Space
    EStdKeyMinus,              // 0x2D  45  - _
    EStdKeyEquals,             // 0x2E  46  = +
    EStdKeySquareBracketLeft,  // 0x2F  47  [ {
    EStdKeySquareBracketRight, // 0x30  48  ] }
    EStdKeyBackSlash,          // 0x31  49  US backslash and bar
    EStdKeyHash,               // 0x32  50  Near the enter key (UK #~)
    EStdKeySemiColon,          // 0x33  51  ; :
    EStdKeySingleQuote,        // 0x34  52  UK ' @  (US ' ")
    EStdKeyXXX,                // 0x35  53  Top left: UK grave/not/bar
    EStdKeyComma,              // 0x36  54  , <
    EStdKeyFullStop,           // 0x37  55  . >
    EStdKeyForwardSlash,       // 0x38  56  / ?
    EStdKeyCapsLock,           // 0x39  57  Caps Lock
    EStdKeyF1,                 // 0x3A  58  F1
    EStdKeyF2,                 // 0x3B  59  F2
    EStdKeyF3,                 // 0x3C  60  F3
    EStdKeyF4,                 // 0x3D  61  F4
    EStdKeyF5,                 // 0x3E  62  F5
    EStdKeyF6,                 // 0x3F  63  F6
    EStdKeyF7,                 // 0x40  64  F7
    EStdKeyF8,                 // 0x41  65  F8
    EStdKeyF9,                 // 0x42  66  F9
    EStdKeyF10,                // 0x43  67  F10
    EStdKeyF11,                // 0x44  68  F11
    EStdKeyF12,                // 0x45  69  F12
    EStdKeyPrintScreen,        // 0x46  70  Print Screen
    EStdKeyScrollLock,         // 0x47  71  Scroll Lock
    EStdKeyPause,              // 0x48  72  Break (Ctrl-Pause)
    EStdKeyInsert,             // 0x49  73  Insert
    EStdKeyHome,               // 0x4A  74  Home
    EStdKeyPageUp,             // 0x4B  75  Page Up
    EStdKeyDelete,             // 0x4C  76  Delete
    EStdKeyEnd,                // 0x4D  77  End
    EStdKeyPageDown,           // 0x4E  78  Page Down
    EStdKeyRightArrow,         // 0x4F  79  Right Arrow
    EStdKeyLeftArrow,          // 0x50  80  Left Arrow
    EStdKeyDownArrow,          // 0x51  81  Down Arrow
    EStdKeyUpArrow,            // 0x52  82  Up Arrow
    EStdKeyNumLock,            // 0x53  83  Num Lock
    EStdKeyNkpForwardSlash,    // 0x54  84  Keypad /
    EStdKeyNkpAsterisk,        // 0x55  85  Keypad *
    EStdKeyNkpMinus,           // 0x56  86  Keypad -
    EStdKeyNkpPlus,            // 0x57  87  Keypad +
    EStdKeyNkpEnter,           // 0x58  88  Keypad Enter
    EStdKeyNkp1,               // 0x59  89  Keypad 1 End
    EStdKeyNkp2,               // 0x5A  90  Keypad 2 Down
    EStdKeyNkp3,               // 0x5B  91  Keypad 3 PageDn
    EStdKeyNkp4,               // 0x5C  92  Keypad 4 Left
    EStdKeyNkp5,               // 0x5D  93  Keypad 5
    EStdKeyNkp6,               // 0x5E  94  Keypad 6 Right
    EStdKeyNkp7,               // 0x5F  95  Keypad 7 Home
    EStdKeyNkp8,               // 0x60  96  Keypad 8 Up
    EStdKeyNkp9,               // 0x61  97  Keypad 9 PageUp
    EStdKeyNkp0,               // 0x62  98  Keypad 0 Insert
    EStdKeyNkpFullStop,        // 0x63  99  Keypad . Delete
    EStdKeyBackSlash,          // 0x64  100 Next to LH shift (UK \|)
    EStdKeyMenu,               // 0x65  101 Application key
    EStdKeyNull,               // 0x66  102 Keyboard power
    EStdKeyEquals              // 0x67  103 Keypad =
    };

// ----------------------------------------------------------------------

// These apply in any modifier state:

const CKeyboardLayout::TUsagePageKey CKeyboardLayout::KEnhancedKeyCodes[] = 
    {
    // Usage page, usage ID, Symbian raw code, Symbian key code (unicode)
    //
    // Generic functionality:
    // 
    { 12, 0x00cd, EStdKeyApplication2, EKeyApplication2 }, // Play/pause (was: EKeyDictaphonePlay)
    { 12, 0x00b7, EStdKeyApplication3, EKeyApplication3 }, // Stop (was: EKeyDictaphoneStop)
    { 12, 0x00e9, EStdKeyIncVolume,      EKeyIncVolume },      // Volume -
    { 12, 0x00ea, EStdKeyDecVolume,      EKeyDecVolume },      // Volume +
    { 12, 0x0095, EStdKeyHelp,           EKeyHelp },           // Help
    { 12, 0x019c, EStdKeyDevice2,        EKeyDevice2 },        // Log off
    //
    // Map system control power and menu keys to the power key:
    //
    { 1,  0x0081, EStdKeyDevice2,        EKeyDevice2 },        // Power down
    { 1,  0x0082, EStdKeyDevice2,        EKeyDevice2 },        // Sleep
    { 1,  0x0083, EStdKeyDevice2,        EKeyDevice2 },        // Wake
    { 1,  0x0084, EStdKeyDevice2,        EKeyDevice2 },        // Context menu
    { 1,  0x0085, EStdKeyDevice2,        EKeyDevice2 },        // Main menu
    { 1,  0x0086, EStdKeyDevice2,        EKeyDevice2 },        // App menu
    //
    // Map common consumer keys to the application and device key ranges:
    // (Note that Series 60 AVKON reserves device keys 0-6, and app key 0.)
    // (Also, we're reserving EStdKeyApplication1 for FEP symbol menu)
    //
    { 12, 0x00e2, EStdKeyApplication3, EKeyApplication3 }, // Mute				(was: EkeyApplication2)
    { 12, 0x00b5, EStdKeyApplication4, EKeyApplication4 }, // Next track 	(Was: EKeyApplication3)
    { 12, 0x00b6, EStdKeyApplication5, EKeyApplication5 }, // Prev track	(was: EKeyApplication4)
    { 12, 0x021a, EStdKeyApplication5, EKeyApplication5 }, // Undo
    { 12, 0x0279, EStdKeyApplication6, EKeyApplication6 }, // Redo
    { 12, 0x0183, EStdKeyApplication7, EKeyApplication7 }, // Media
    { 12, 0x0223, EStdKeyF1, EKeyF1 					}, // Web/Home [WAP] was: EKeyApplication8
    { 12, 0x0199, EStdKeyApplication9, EKeyApplication9 }, // Messenger [SMS]
    { 12, 0x018a, EStdKeyF2, EKeyF2 }, 					   // Mail [Email] was: EKeyApplicationA
    { 12, 0x0192, EStdKeyApplicationC, EKeyApplicationB }, // Calculator
    { 12, 0x01ab, EStdKeyApplicationD, EKeyApplicationD }, // Spell
    { 12, 0x0207, EStdKeyApplicationE, EKeyApplicationE }, // Save
    { 12, 0x0208, EStdKeyApplicationF, EKeyApplicationF }, // Print
    { 12, 0x0201, EStdKeyDevice7,      EKeyDevice7 },      // New
    { 12, 0x0202, EStdKeyDevice8,      EKeyDevice8 },      // Open
    { 12, 0x0203, EStdKeyDevice9,      EKeyDevice9 },      // Close
    { 12, 0x0289, EStdKeyDeviceA,      EKeyDeviceA },      // Reply
    { 12, 0x028b, EStdKeyDeviceB,      EKeyDeviceB },      // Fwd
    { 12, 0x028C, EStdKeyDeviceC,      EKeyDeviceC },      // Send
    { 12, 0x01a7, EStdKeyDeviceD,      EKeyDeviceD },      // My Docs [Notepad]
    { 12, 0x01b6, EStdKeyDeviceE,      EKeyDeviceE },      // My Pics
    { 12, 0x01b7, EStdKeyDeviceF,      EKeyDeviceF }       // My Music
    };

// ----------------------------------------------------------------------

 TUint16 CKeyboardLayout::TranslateKey(TInt aHidKey, 
    TInt aUsagePage, THidModifier aModifiers, TLockKeys aLockKeys) const
    {
    if (aUsagePage == EUsagePageKeyboard)
        {
        return TranslateKeyboardKey(aHidKey, aModifiers, aLockKeys);
        }
    else
        {
        return TranslateOtherKey(aHidKey, aUsagePage, aModifiers, aLockKeys);
        }
    }

// ----------------------------------------------------------------------

TUint16 CKeyboardLayout::TranslateKeyboardKey(TInt aHidKey, 
    THidModifier aModifiers, TLockKeys aLockKeys) const
    {
    // For most layouts, right alt (AltGr) is defined to have the
    // same effect as ctrl-alt:
    //
    if (AltGrIsControlAlt() && aModifiers.RightAlt())
        {
        aModifiers.Merge(THidModifier::ELeftControl);
        }

    // Caps lock and num lock usually just invert the behaviour of
    // the shift key:
    //
    if ((aLockKeys.iCapsLock && ChangesWithCapsLock(aHidKey, aModifiers)) ||
        (!aLockKeys.iNumLock && ChangesWithNumLock(aHidKey)))
        {
        aModifiers.InvertShift();
        }

    // All standard layouts avoid control and shift-control modifiers.
    // We map the standard control and shift-control key combinations
    // to Symbian shortcut key codes:
    //
    TUint16 unicode = 0;

    if (aModifiers.Control() && !aModifiers.Alt())
        {
        // Symbian Knowledgebase articles FAQ-0507 and FAQ-0511 say
        // that this is the correct way to handle control keys so
        // that keyboard shortcuts (such as ctrl-x) will work:
        
        const TInt KCtrlMin = 4;   // aA
        const TInt KCtrlMax = 29;  // zZ
            
        if ((aHidKey >= KCtrlMin) && (aHidKey <= KCtrlMax))
            {
            unicode = static_cast<TUint16>(CTRL(
                Unicode(aHidKey, THidModifier(0))));
            }
        }

    // If the key wasn't a standard control or shift-control
    // combination:
    // 
    if (unicode == 0)
        {
        unicode = Unicode(aHidKey, aModifiers);
        }

    return unicode;
    }

// ----------------------------------------------------------------------

 TBool CKeyboardLayout::ChangesWithCapsLock(TInt aHidKey,
    THidModifier aModifiers) const
    {
    // For most layouts Caps Lock only changes those keys in the
    // standard A..Z positions:
    //
    const TInt KCapsLockKeysMin = 4;   // aA
    const TInt KCapsLockKeysMax = 29;  // zZ

    TBool inRange = (aHidKey >= KCapsLockKeysMin) && 
        (aHidKey <= KCapsLockKeysMax);

    // The standard behaviour is that Caps Lock shouldn't have any
    // effect on alt-ctrl ("Alt Gr") modified keys:
    //
    TBool altGr = aModifiers.Control() && aModifiers.Alt();

    return !altGr && inRange;
    }

// ----------------------------------------------------------------------

 TBool CKeyboardLayout::ChangesWithNumLock(TInt aHidKey) const
    {
    // Num Lock only ever affects a standard subset of the keypad keys:

    const TInt KNumLockMin = 89;  // Keypad End
    const TInt KNumLockMax = 99;  // Keypad Delete

    return (aHidKey >= KNumLockMin) && (aHidKey <= KNumLockMax);
    }

// ----------------------------------------------------------------------

 TBool CKeyboardLayout::AltGrIsControlAlt() const
    {
    return ETrue;
    }

// ----------------------------------------------------------------------

 TBool CKeyboardLayout::IsDeadKey(TInt /*aUnicodeKey*/) const
    {
    return EFalse;
    }

 TUint16 CKeyboardLayout::FindCombiningChar(
    TUint16 /*aDeadKeyUnicode*/, TUint16 /*aUnicodeKey*/) const
    {
    return 0;
    }

// ----------------------------------------------------------------------

 TBool CKeyboardLayout::IsRepeatingKey(TInt aUnicodeKey) const
    {
    const TInt KNoRepeatKeys[] = 
        {
        // The modifier and lock keys:
        EKeyLeftShift,
        EKeyRightShift,
        EKeyLeftAlt,
        EKeyRightAlt,
        EKeyLeftCtrl,
        EKeyRightCtrl,
        EKeyLeftFunc,
        EKeyRightFunc,
        EKeyCapsLock,
        EKeyNumLock,
        EKeyScrollLock,

        // Other keys that toggle:
        EKeyPause,
        EKeyInsert,

        // Some special cases:
        EKeyEscape,
        EKeyPrintScreen
        };

    const TInt KNumberOfNoRepeatKeys = 
        (sizeof (KNoRepeatKeys)) / (sizeof (TInt));

    TBool match = EFalse;
    
    for (TInt i=0; i<KNumberOfNoRepeatKeys; ++i)
        {
        if (aUnicodeKey == KNoRepeatKeys[i])
            {
            match = ETrue;
            }
        }

    return !match;
    }

// ----------------------------------------------------------------------

TUint16 CKeyboardLayout::TranslateOtherKey(TInt aHidKey, 
    TInt aUsagePage, THidModifier, TLockKeys) const
    {
    static const TInt KEnhancedKeyCodesSize = 
        (sizeof (KEnhancedKeyCodes)) / (sizeof (TUsagePageKey));

    TUint16 unicode = 0;

    for (TInt i=0; (unicode == 0) && (i<KEnhancedKeyCodesSize); ++i)
        {
        if ((aHidKey == KEnhancedKeyCodes[i].iHidId)
            && (aUsagePage == KEnhancedKeyCodes[i].iHidPage))
            {
            unicode = KEnhancedKeyCodes[i].iCode;
            }
        }

    return unicode;
    }

// ----------------------------------------------------------------------

 TInt CKeyboardLayout::RawScanCode(TInt aHidKey, 
    TInt aUsagePage, THidModifier aModifiers) const
    {
    TInt result = EStdKeyNull;

    if (aUsagePage == EUsagePageKeyboard)
        {
        result = CheckChangingKeys(aHidKey, aModifiers);
        
        if (result == EStdKeyNull)
            {
            const TInt KRawCodesSize = (sizeof (KRawCodes)) / (sizeof (TInt));
            if (aHidKey < KRawCodesSize)
                {
                result = KRawCodes[aHidKey];
                }
            else
                {
                result = CheckModifierKeys(aHidKey);
                }
            }
        }
    else
        {
        result = CheckSpecialRawCode(aHidKey, aUsagePage);
        }
    
    return result;
    }

TInt CKeyboardLayout::CheckChangingKeys(TInt aHidKey, THidModifier aModifiers)
    {
    // Handle any key that changes scan code depending on the modifier
    // state:

    // All these special cases only apply to the keyboard usage page:
    const TInt KTabKey = 0x2B;
    if ((aHidKey == KTabKey) && aModifiers.AltOnly())
        {
        // Application switch key:
        return EStdKeyApplication0;
        }
    
    const TInt KDelKey = 0x4c;
    const TInt KKeypadDelKey = 0x63;
    if (((aHidKey == KDelKey) || (aHidKey == KKeypadDelKey))
        && (aModifiers.Fold() == THidModifier::EAltCtrl))
        {
        // Power key:
        return EStdKeyDevice2;
        }
        
    const TInt KEnterKey = 0x28;
    const TInt KKeypadEnterKey = 0x58;
    if (((aHidKey == KEnterKey) || (aHidKey == KKeypadEnterKey))
        && (aModifiers.ShiftOnly() || aModifiers.ControlOnly()))
        {
        // OK key:
        return EStdKeyDevice3;
        }

    const TInt KLeftGuiKey = 0xe3;
    if (aHidKey == KLeftGuiKey)
        {
        // Call create or left softkey:
        return aModifiers.Shift() ? EStdKeyYes : EStdKeyDevice0;
        }

    const TInt KRightGuiKey = 0xe7;
    const TInt KMenuKey = 0x65;
    if ((aHidKey == KRightGuiKey) || (aHidKey == KMenuKey))
        {
        // Call end or right softkey:
        return aModifiers.Shift() ? EStdKeyNo : EStdKeyDevice1;
        }

    return EStdKeyNull;
    }

TInt CKeyboardLayout::CheckModifierKeys(TInt aHidKey)
    {
    const TInt KModifierCodes[] =
        {
        EStdKeyLeftCtrl,   // HID code 0xe0 224
        EStdKeyLeftShift,  //          0xe1 225
        EStdKeyLeftAlt,    //          0xe2 226
        EStdKeyLeftFunc,   //          0xe3 227 (overridden by left sofkey)
        EStdKeyRightCtrl,  //          0xe4 228
        EStdKeyRightShift, //          0xe5 229
        EStdKeyRightAlt,   //          0xe6 230
        EStdKeyRightFunc   //          0xe7 231 (overridden by right sofkey)
        };

    const TInt KHidIdLeftCtrl = 0xe0;
    const TInt KHidIdRightFunc = 0xe7;

    TInt result = EStdKeyNull;

    if ((aHidKey >= KHidIdLeftCtrl) && (aHidKey <= KHidIdRightFunc))
        {
        result = KModifierCodes[aHidKey - KHidIdLeftCtrl];
        }

    return result;
    }

TInt CKeyboardLayout::CheckSpecialRawCode(TInt aHidKey, TInt aUsagePage)
    {
    static const TInt KEnhancedKeyCodesSize = 
        (sizeof (KEnhancedKeyCodes)) / (sizeof (TUsagePageKey));

    TInt scanCode = EStdKeyNull;

    for (TInt i=0; (scanCode == EStdKeyNull) && (i<KEnhancedKeyCodesSize); ++i)
        {
        if ((aHidKey == KEnhancedKeyCodes[i].iHidId)
            && (aUsagePage == KEnhancedKeyCodes[i].iHidPage))
            {
            scanCode = KEnhancedKeyCodes[i].iScanCode;
            }
        }

    return scanCode;
    }

// ----------------------------------------------------------------------

 CStandardKeyboardLayout::CStandardKeyboardLayout(
    const TInt* aColumnMap, TInt aMapSize,
    const TUint16* aKeyCodes, TInt aTableSize,
    const TSpecialKey* aSpecialCases)
    : iColumnMap(aColumnMap), iKeyCodes(aKeyCodes),
      iNumColumns(aMapSize / sizeof (TInt)), 
      iNumKeys(aTableSize / (sizeof (TUint16) * iNumColumns)), 
      iSpecialCases(aSpecialCases)
    {
    // nothing else to do
    }

 TUint16 CStandardKeyboardLayout::Unicode(TInt aHidKey, 
    THidModifier aModifiers) const
    {
    TUint16 unicode = 0;
    TUint8 foldedModifiers = aModifiers.Fold();

    if ((aHidKey >= 0) && (aHidKey < iNumKeys))
        {
        for (TInt i=0; i<iNumColumns; ++i)
            {
            if (iColumnMap[i] == foldedModifiers)
                unicode = iKeyCodes[(aHidKey * iNumColumns) + i];
            }
        }

    if ((unicode == 0) && iSpecialCases)
        {
        unicode = FindSpecialKey(iSpecialCases, aHidKey, foldedModifiers);
        }
        
    return unicode;
    }

 TUint16 CStandardKeyboardLayout::FindSpecialKey(
    const TSpecialKey* aMap, TInt aHidKey, TUint8 aFoldedModifiers)
    {
    // Look for this HID key code in the appropriate special key
    // key table. A zero key code means "end of table".

    TUint16 keyCode = 0;
    for (TInt i=0; (keyCode == 0) && (aMap[i].iHidKey != 0); ++i)
        {
        if ((aHidKey == aMap[i].iHidKey) 
            && (aFoldedModifiers == aMap[i].iFoldedModifiers))
            {
            keyCode = aMap[i].iUnicode;
            }
        }
    
    return keyCode;
    }

// ----------------------------------------------------------------------

 CDeadKeyLayout::CDeadKeyLayout(const TInt* aColumnMap, 
    TInt aMapSize, const TUint16* aKeyCodes, TInt aTableSize,
    const TSpecialKey* aSpecialCases, const TIndexPair* aDeadKeyIndex)
    : CStandardKeyboardLayout(aColumnMap, aMapSize, aKeyCodes,
        aTableSize, aSpecialCases), iDeadKeyIndex(aDeadKeyIndex)
    {
    // nothing else to do
    }

 TBool CDeadKeyLayout::IsDeadKey(TInt aUnicodeKey) const
    {
    TBool match = EFalse;
    
    for (TInt i=0; !match && (iDeadKeyIndex[i].iUnicode != 0); ++i)
        {
        match = (aUnicodeKey == iDeadKeyIndex[i].iUnicode);
        }
    
    DBG(RDebug::Print(_L("[HID]\tCDeadKeyLayout::IsDeadKey(0x%08x:0x%04x) = %d"),
      iDeadKeyIndex, aUnicodeKey, match));

    return match;
    }

 TUint16 CDeadKeyLayout::FindCombiningChar(TUint16 aDeadKeyUnicode,
    TUint16 aUnicodeKey) const
    {
    const TKeyPair* map = 0;
    for (TInt i=0; (map == 0) && (iDeadKeyIndex[i].iUnicode != 0); ++i)
        {
        if (aDeadKeyUnicode == iDeadKeyIndex[i].iUnicode)
            map = iDeadKeyIndex[i].iMap;
        }

    TUint16 comboChar = 0;
    if (map != 0)
        {
        // Look for this base character code in the appropriate dead
        // key table.  (NB. A zero base code means "end of table".)
        //
        for (TInt j=0; (comboChar==0) && (map[j].iBaseChar != 0); ++j)
            {
            if (aUnicodeKey == map[j].iBaseChar)
                {
                comboChar = map[j].iComboChar;
                }
            }
        }
    
    return comboChar;
    }

// ----------------------------------------------------------------------
