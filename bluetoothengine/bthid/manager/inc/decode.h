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


#ifndef __DECODE_H
#define __DECODE_H

// INCLUDES
#include "hidkeys.h"

// FORWARD DECLARATIONS

class CScanCodeStore;
class CKeyboardLayout;
class TDecodedKeyInfo;
class TKeyEventInfo;

// CLASS DECLARATION
class CKeyboardDecoder : public CBase
    {
public:
    static CKeyboardDecoder* NewL();
    virtual ~CKeyboardDecoder();

    void Event(const TKeyEventInfo& aKeyEvent, TDecodedKeyInfo &aDecodedKeys);
    void SetLayout(CKeyboardLayout* aLayout);
    void Reset();

private:
    CKeyboardDecoder();
    void ConstructL();

    void ProcessDeadKeys(TUint16 aUnicodeKey, TInt aHidKey, TInt aUsagePage,
            TDecodedKeyInfo &aDecodedKeys);
    TBool HandleRightAlt(const TKeyEventInfo &aEvent,
            TDecodedKeyInfo& aDecodedKeys);

private:
    TTranslatedKey iDeadKey;
    CKeyboardLayout* iLayout;
    CScanCodeStore* iScanCodes;

    TBool iAltGrSequence;
    TInt iNumKeysDown;
    };

#endif
