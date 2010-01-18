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
* Description:  Key types.
 *
*/


#ifndef __HIDKEYS_H
#define __HIDKEYS_H

#include <e32std.h>
#include <e32svr.h>

// CLASS DECLARATIONS

/*!
 TLockKeys is used to represent the current state of the locking
 modifier keys, i.e. Caps Lock and Num Lock. 
 */

class TLockKeys
    {
public:
    inline TLockKeys();
    inline TLockKeys(TBool aCapsLock, TBool aNumLock);

    TBool iCapsLock; //!< ETrue if Caps Lock is active
    TBool iNumLock; //!< ETrue if Num Lock is active
    };

/*!
 TTranslatedKey contains the details of a decoded HID key event.  It
 is returned (as part of a TDecodedKeyInfo object) from a call to
 RLayoutManager::KeyEvent().
 */

class TTranslatedKey
    {
public:
    inline TTranslatedKey();
    inline TTranslatedKey(TUint16 aUnicode, TInt aScanCode, TInt aHidKey,
            TInt aUsagePage, TBool aIsRepeatingKey);

    TUint16 iUnicode; //!< Symbian (Unicode) key code value (cf TKeyCode)
    TInt iScanCode; //!< Symbian raw scan code value (cf TStdScanCode)
    TInt iHidKey; //!< HID code for the key that caused this event
    TInt iUsagePage; //!< Usage page for the key that caused this event
    TBool iIsRepeatingKey; //!< ETrue if the key can repeat
    };

/*!
 TDecodedKeyInfo contains the results of decoding a HID key event.  It is
 the output of a call to RLayoutManager::KeyEvent().  
 
 Note that the iScanCode member corresponds to the current key event
 as given in the arguments to RLayoutManager::KeyEvent(), whereas the
 iScanCode values in #iEvent correspond to the key event that caused
 the translated key press; this could be the previous event in the
 case of a dead key.
 */

class TDecodedKeyInfo
    {
public:
    inline TDecodedKeyInfo();

    TInt iCount; //!< Number of key press events (0, 1 or 2)
    TInt iScanCode; //!< Symbian raw scan code value
    TTranslatedKey iEvent[2]; //!< Translated key press data
    };

// ----------------------------------------------------------------------

inline TTranslatedKey::TTranslatedKey() :
    iUnicode(0), iScanCode(0), iHidKey(0), iUsagePage(0), iIsRepeatingKey(
            EFalse)
    {
    // nothing else to do
    }

inline TTranslatedKey::TTranslatedKey(TUint16 aUnicode, TInt aScanCode,
        TInt aHidKey, TInt aUsagePage, TBool aIsRepeatingKey) :
    iUnicode(aUnicode), iScanCode(aScanCode), iHidKey(aHidKey), iUsagePage(
            aUsagePage), iIsRepeatingKey(aIsRepeatingKey)
    {
    // nothing else to do
    }

inline TDecodedKeyInfo::TDecodedKeyInfo() :
    iCount(0), iScanCode(0)
    {
    // nothing else to do
    }

// ----------------------------------------------------------------------

inline TLockKeys::TLockKeys() :
    iCapsLock(EFalse), iNumLock(EFalse)
    {
    // nothing else to do
    }

inline TLockKeys::TLockKeys(TBool aCapsLock, TBool aNumLock) :
    iCapsLock(aCapsLock), iNumLock(aNumLock)
    {
    // nothing else to do
    }

#endif //HIDKEYS_H
// End of File
