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
* Description:  Declares main application class.
 *
*/


#ifndef __LAYOUT_H
#define __LAYOUT_H

#include <e32std.h>
#include <e32base.h>
#include "hidlayoutplugininterface.h"

// FORWARD DECLARATIONS
class TLockKeys;
class THidModifier;

// CLASS DECLARATION

/*!
 CKeyboardLayout is the abstract base class for all keyboard layouts.
 */
class CKeyboardLayout : public CHidLayoutPluginInterface
    {
public:
    /*!
     Translate a HID usage into a Symbian raw scan code value,
     taking into account both the current modifier state, and the
     lock key (Num Lock, Caps Lock) state.

     This function is called by the default implementation of
     TranslateKey().

     @param aHidKey The HID usage ID of the key
     @param aUsagePage The HID usage page of the key
     @param aModifiers The current modifier state
     @param aLockKeys The current lock key state
     
     @result A Symbian Unicode key value, cf TKeyCode. EKeyNull (0)
     if the key code could not be translated.
     */
    virtual TUint16 TranslateKey(TInt aHidKey, TInt aUsagePage,
            THidModifier aModifiers, TLockKeys aLockKeys) const;

    /*!
     Translate a HID usage ID (keyboard usage page, 0x07) into a
     Symbian Unicode key value, taking into account the current
     modifier state. This is normally just an interface to one or
     more static look up tables.
     
     Unicode() is pure virtual.

     @param aHidKey The HID usage ID of the key
     @param aModifiers The current modifier state
     
     @result A Symbian Unicode key value, cf TKeyCode. EKeyNull (0)
     if the key code could not be translated.
     */
    virtual TUint16 Unicode(TInt aHidKey, THidModifier aModifiers) const = 0;

    /*!
     Translate a HID usage into a Symbian raw scan code value,
     taking into account the current modifier state.

     This function is called by the default implementation of
     TranslateKey().

     @param aHidKey The HID usage ID of the key
     @param aUsagePage The HID usage page of the key
     @param aModifiers The current modifier state
     
     @result A Symbian raw scan code value, cf TStdScanCode.
     EStdKeyNull (0) if the key code could not be translated.
     */
    virtual TInt RawScanCode(TInt aHidKey, TInt aUsagePage,
            THidModifier aModifiers) const;

    /*!
     Determine if a given Unicode key value corresponds to an
     auto-repeating key.

     @param aUnicodeKey Symbian Unicode key value

     @result ETrue if the key should be allowed to repeat.
     */
    virtual TBool IsRepeatingKey(TInt aUnicodeKey) const;

    /*!
     Determine if the given Unicode key value corresponds to a
     dead key.

     @param aUnicodeKey Symbian Unicode key value

     @result ETrue if the key is a dead key.
     */
    virtual TBool IsDeadKey(TInt aUnicodeKey) const;

    /*!
     Find the Unicode character, if any, which results from a
     combination of the dead key, aDeadKeyUnicode, followed by the
     second key press, aUnicodeKey.

     @result Symbian Unicode key value for the combination
     character. EKeyNull (0) if the combination wasn't found.
     */
    virtual TUint16 FindCombiningChar(TUint16 aDeadKeyUnicode,
            TUint16 aUnicodeKey) const;

    /*!
     Determine if the given HID key code behaves as if the shift
     modifier is inverted when Num Lock is active. This function is
     called by the default implementation of TranslateKey().

     @param aHidKey The HID usage ID of the key (keyboard usage page, 0x07)

     @result ETrue if the key changes with Num Lock
     */
    virtual TBool ChangesWithNumLock(TInt aHidKey) const;

    /*!
     Determine if the given HID key code behaves as if the shift
     modifier is inverted when Caps Lock is active, taking into
     account the current modifier state. This function is
     called by the default implementation of TranslateKey().

     @param aHidKey The HID usage ID of the key (keyboard usage page, 0x07)
     @param aModifiers The current modifier state

     @result ETrue if the key changes with Caps Lock
     */
    virtual TBool
            ChangesWithCapsLock(TInt aHidKey, THidModifier aModifiers) const;

    /*!
     Determine if the right Alt key (Alt Gr) should produce the same
     effect as pressing Control-Alt.
     
     @result ETrue if Alt Gr is equivalent to Control-Alt
     */
    virtual TBool AltGrIsControlAlt() const;

    /*!
     Return the keyboard layout ID for a given layout.
     */
    virtual TInt LayoutId() const = 0;

protected:
    TUint16 TranslateKeyboardKey(TInt aHidKey, THidModifier aModifiers,
            TLockKeys aLockKeys) const;
    TUint16 TranslateOtherKey(TInt aHidKey, TInt aUsagePage,
            THidModifier aModifiers, TLockKeys aLockKeys) const;

private:
    static TInt CheckChangingKeys(TInt aHidKey, THidModifier aModifiers);
    static TInt CheckModifierKeys(TInt aHidKey);
    static TInt CheckSpecialRawCode(TInt aHidKey, TInt aUsagePage);

protected:
    // Scan code look-up tables:

    static const TInt KRawCodes[]; // standard keys from HID usage page 7

    struct TUsagePageKey
        {
        TInt iHidPage;
        TInt iHidId;
        TInt iScanCode;
        TUint16 iCode;
        };

    static const TUsagePageKey KEnhancedKeyCodes[]; // Usage page != 0x07
    };

// ----------------------------------------------------------------------

/*!
 Base class for all standard keyboard layouts. Assumes a specific
 configuration of look-up tables:
 
 - A main look-up table, indexed by HID code. This contains one or
 more columns of data, where each column corresponds to a particular
 modifier state. The size of the table is specified in the constructor. 
 
 - A list of the modifier states that correspond to each column in
 the main table.
 
 - A table containing special cases.  This is intended to reduce the
 number of columns needed in the main table.  This table is searched
 linearly and is zero terminated.
 
 An implementation of CKeyboardLayout::Unicode() is provided that
 performs a HID key look-up using the main table, and then, if
 no match was found, searches the special case table.
 */
class CStandardKeyboardLayout : public CKeyboardLayout
    {
public:
    /*!
     TSpecialKey items are used to create a standard special case
     table. If the HID usage ID of the key matches iHidKey, and the
     current modifier state matches iFoldedModifiers, then the decoded
     key code will be iUnicode.
     */
    struct TSpecialKey
        {
        TUint16 iHidKey; //!< HID key usage (keyboard usage page)
        TUint8 iFoldedModifiers; //!< Modifiers, as from THidModifier::Fold()
        TUint16 iUnicode; //!< The resulting Unicode value
        };

public:
    /*!
     Constructor

     @param aColumnMap Mapping from modifier state to main table columns
     @param aMapSize Size of the columns map, in bytes
     @param aKeyCodes The main look-up table
     @param aTableSize Size of the main table, in bytes
     @param aSpecialCases The special case table
     */
    CStandardKeyboardLayout(const TInt* aColumnMap, TInt aMapSize,
            const TUint16* aKeyCodes, TInt aTableSize,
            const TSpecialKey* aSpecialCases);

    /*!
     Translate a HID usage ID (keyboard usage page, 0x07) into a
     Symbian Unicode key value, taking into account the current
     modifier state.      

     @param aHidKey The HID usage ID of the key
     @param aModifiers The current modifier state
     
     @result A Symbian Unicode key value, cf TKeyCode.  EKeyNull (0)
     if the key code could not be translated.
     */
    virtual TUint16 Unicode(TInt aHidKey, THidModifier aModifiers) const;

protected:
    /*!
     Search for a given HID usage ID (keyboard usage page, 0x07)
     in the special case table.

     @param aMap The special case table
     @param aHidKey HID usage ID to search for
     @param aFoldedModifiers Modifier state to search for

     @result A Symbian Unicode key value, cf TKeyCode. EKeyNull (0)
     if the key code could not be translated.
     */
    static TUint16 FindSpecialKey(const TSpecialKey* aMap, TInt aHidKey,
            TUint8 aFoldedModifiers);

protected:
    // Standard look-up table:
    const TInt* iColumnMap;
    const TUint16* iKeyCodes;

    const TInt iNumColumns;
    const TInt iNumKeys;

    // Special cases:
    const TSpecialKey* iSpecialCases;
    };

// ----------------------------------------------------------------------

/*!
 Base class for keyboard layouts that use dead keys, derived from
 CStandardKeyboardLayout. Assumes a specific configuration of look-up
 tables:
 
 - An index table containing one TIndexPair entry for each dead
 key. The TIndexPair provides the unicode value for the dead key,
 and a link to a TKeyPair table.
 
 - One table of TKeyPair items for each dead key. Each TKeyPair
 contains the Unicode value for the combination (second) key press
 together with the Unicode key value that should result.
 
 - Other tables as for CStandardKeyboardLayout
 
 Implementations of CKeyboardLayout::IsDeadKey() and
 CKeyboardLayout::FindCombiningChar() are provided that can
 use these dead keys tables.
 */
class CDeadKeyLayout : public CStandardKeyboardLayout
    {
public:
    /*!
     TKeyPair items are used to create dead key tables. The dead key
     followed by a Unicode key code iBaseChar will produce the decoded
     Unicode value of iComboChar. The Unicode value for the dead key
     itself is given in a TIndexPair structure.
     */
    struct TKeyPair
        {
        TUint16 iBaseChar; //!< Unicode character that follows the dead key
        TUint16 iComboChar; //!< Resulting Unicode value
        };

    /*!
     TIndexPair items are used to create dead key tables. There is one
     TIndexPair for each possible dead key. Each TIndexPair links in turn
     to a table of TKeyPair items.
     */
    struct TIndexPair
        {
        TUint16 iUnicode; //!< Unicode value for the dead key
        const TKeyPair *iMap; //!< Corresponding table of TKeyPair items
        };

public:
    /*!
     Constructor

     @param aColumnMap Mapping from modifier state to main table columns
     @param aMapSize Size of the columns map, in bytes
     @param aKeyCodes The main look-up table
     @param aTableSize Size of the main table, in bytes
     @param aSpecialCases The special case table
     @param aDeadKeyIndex The dead key index table
     */
            CDeadKeyLayout(const TInt* aColumnMap, TInt aMapSize,
                    const TUint16* aKeyCodes, TInt aTableSize,
                    const TSpecialKey* aSpecialCases,
                    const TIndexPair* aDeadKeyIndex);

    /*!
     Determine if the given Unicode key value corresponds to a
     dead key.

     @param aUnicodeKey Symbian Unicode key value

     @result ETrue if the key is a dead key.
     */
    virtual TBool IsDeadKey(TInt aUnicodeKey) const;

    /*!
     Find the Unicode character, if any, which results from a
     combination of the dead key, aDeadKeyUnicode, followed by the
     second key press, aUnicodeKey.

     @result Symbian Unicode key value for the combination
     character. EKeyNull (0) if the combination wasn't found.
     */
    virtual TUint16 FindCombiningChar(TUint16 aDeadKeyUnicode,
            TUint16 aKeyCode) const;

protected:
    const TIndexPair* iDeadKeyIndex;

    };

// ----------------------------------------------------------------------

#endif
