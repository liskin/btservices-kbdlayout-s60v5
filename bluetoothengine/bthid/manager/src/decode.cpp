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
#include <ecom/ecom.h>
#include "modifier.h"
#include "layout.h"
#include "decode.h"
#include "codestore.h"
#include "client.h"
#include "debug.h"
#include "hidvalues.h"

// ----------------------------------------------------------------------

CKeyboardDecoder::CKeyboardDecoder()
        : iLayout(0), iAltGrSequence(EFalse), iNumKeysDown(0)
    {
    TRACE_INFO( (
                    _L("[HID]\tCKeyboardDecoder::CKeyboardDecoder at 0x%08x"), this));
    }

CKeyboardDecoder* CKeyboardDecoder::NewL()
    {
    CKeyboardDecoder* self =
        new (ELeave) CKeyboardDecoder;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

void CKeyboardDecoder::ConstructL()
    {
    iScanCodes = CScanCodeStore::NewL();
    }

CKeyboardDecoder::~CKeyboardDecoder()
    {
    TRACE_INFO( (_L("[HID]\t~CKeyboardDecoder() 0x%08x"), this));

    iLayout = 0; // (we don't own iLayout)

    delete iScanCodes;
    }

// ----------------------------------------------------------------------

void CKeyboardDecoder::Event(const TKeyEventInfo &aEvent,
                             TDecodedKeyInfo& aDecodedKeys)
    {
    aDecodedKeys.iCount = 0;
    aDecodedKeys.iScanCode = EStdKeyNull;

    iNumKeysDown += (aEvent.iIsKeyDown ? 1 : -1);
    // (NB. it is possible to get an up without a down just after a new
    // decoder goes into use when the keyboard layout is changed -- but
    // only if the Bluetooth keyboard is used to make the layout change.)
    if (iNumKeysDown < 0)
        iNumKeysDown = 0;

    if (iLayout)
        {
        if (!HandleRightAlt(aEvent, aDecodedKeys))
            {
            if (aEvent.iIsKeyDown)
                {
                // Key down event:

                TInt rawCode = iLayout->RawScanCode(aEvent.iHidKey,
                                                    aEvent.iUsagePage, aEvent.iModifiers);

                // Store the raw scan code, so that the scan code for
                // the "key up" event for this key will match the one
                // sent for the "key down" event.  These could be
                // different, for example, if the scan code changes
                // depending on the current modifier state.
                //
                if (iScanCodes->Store(aEvent.iHidKey, aEvent.iUsagePage,
                                      rawCode) == KErrNone)
                    {
                    aDecodedKeys.iScanCode = rawCode;
                    }
                else
                    {
                    // Store() may fail due to low memory. In that
                    // case we don't send key down (or key up) events:
                    //
                    aDecodedKeys.iScanCode = EStdKeyNull;
                    }

                // Decode any key press events:

                TUint16 unicodeKey = iLayout->TranslateKey(aEvent.iHidKey,
                                     aEvent.iUsagePage, aEvent.iModifiers, aEvent.iLockKeys);

                if (unicodeKey != 0)
                    {
                    ProcessDeadKeys(unicodeKey, aEvent.iHidKey,
                                    aEvent.iUsagePage, aDecodedKeys);
                    }
                }
            else
                {
                // Key up event:

                aDecodedKeys.iScanCode =
                    iScanCodes->Retrieve(aEvent.iHidKey, aEvent.iUsagePage);
                }
            }
        }
    }

// ----------------------------------------------------------------------

TBool CKeyboardDecoder::HandleRightAlt(const TKeyEventInfo &aEvent,
                                       TDecodedKeyInfo& aDecodedKeys)
    {
    // A press and release of the right-alt (Alt Gr) key should cause
    // the S60 symbol selection dialog to appear, if no other keys are
    // pressed in between.

    TBool handledEvent = EFalse;

    const TInt KRightAltKey = 0xe6;

    if ((aEvent.iHidKey == KRightAltKey) &&
            (aEvent.iUsagePage == EUsagePageKeyboard))
        {
        if (aEvent.iIsKeyDown && (iNumKeysDown == 1))
            {
            iAltGrSequence = ETrue;
            }

        if (!aEvent.iIsKeyDown && iAltGrSequence)
            {
            // We use EKeyApplication2 to indicate to the modified T9
            // FEP that we wish it to produce the symbol selection dialog:
            // EKeyApplication2 is not passed to Fep anymore, but
            // it is converted to '*' in SendKeyPress() (keyboard.cpp)
            aDecodedKeys.iEvent[0] = TTranslatedKey(EKeyApplication2,
                                                    EStdKeyApplication2, 0, 0, EFalse);
            aDecodedKeys.iCount = 1;

            handledEvent = ETrue;
            iAltGrSequence = EFalse;
            }
        }
    else
        {
        iAltGrSequence = EFalse;
        }

    return handledEvent;
    }

// ----------------------------------------------------------------------

void CKeyboardDecoder::Reset()
    {
    iDeadKey.iUnicode = 0;
    iAltGrSequence = EFalse;
    iNumKeysDown = 0;
    }

// ----------------------------------------------------------------------

void CKeyboardDecoder::SetLayout(CKeyboardLayout* aLayout)
    {
    TRACE_INFO( (_L("[HID]\tCKeyboardDecoder::SetLayout(0x%08x)"), aLayout));

    iLayout = aLayout;
    Reset();
    }

// ----------------------------------------------------------------------

void CKeyboardDecoder::ProcessDeadKeys(TUint16 aUnicodeKey,
                                       TInt aHidKey, TInt aUsagePage, TDecodedKeyInfo &aDecodedKeys)
    {
    if (iDeadKey.iUnicode == 0)
        {
        // Check if the current key is a dead key:
        //
        if (iLayout->IsDeadKey(aUnicodeKey))
            {
            // We've received a dead key event, so there's no key press
            // to issue -- just record the details for later:
            //
            TRACE_INFO( (_L("[HID]\tDeadkey 0x%04x (%d:%d)"),
                         aUnicodeKey, aUsagePage, aHidKey));

            iDeadKey = TTranslatedKey(aUnicodeKey,
                                      aDecodedKeys.iScanCode, aHidKey, aUsagePage, EFalse);
            aDecodedKeys.iCount = 0;
            }
        else
            {
            // No dead key processing to do, so just send a
            // normal key code:
            //
            TRACE_INFO( (_L("[HID]\tNo dead key processing, one event")));

            aDecodedKeys.iEvent[0] = TTranslatedKey(aUnicodeKey,
                                                    aDecodedKeys.iScanCode, aHidKey, aUsagePage,
                                                    iLayout->IsRepeatingKey(aUnicodeKey));
            aDecodedKeys.iCount = 1;
            }
        }
    else
        {
        // We've an active dead key, see if the current key produces
        // a combination character:
        //
        TUint16 comboChar =
            iLayout->FindCombiningChar(iDeadKey.iUnicode, aUnicodeKey);

        if (comboChar == 0)
            {
            TRACE_INFO( (_L("[HID]\tNo match for 0x%04x, sending after 0x%04x"),
                         aUnicodeKey, iDeadKey.iUnicode));

            // No match, so two key events to send:

            aDecodedKeys.iEvent[0] = iDeadKey;
            aDecodedKeys.iEvent[1] = TTranslatedKey(aUnicodeKey,
                                                    aDecodedKeys.iScanCode, aHidKey, aUsagePage,
                                                    iLayout->IsRepeatingKey(aUnicodeKey));
            aDecodedKeys.iCount = 2;
            }
        else
            {
            TRACE_INFO( (_L("[HID]\tFound a match 0x%04x, one combo to send 0x%04x"),
                         aUnicodeKey, comboChar));

            // Found a match, so one combo key event to send:

            aDecodedKeys.iEvent[0] = TTranslatedKey(comboChar,
                                                    aDecodedKeys.iScanCode, aHidKey, aUsagePage,
                                                    iLayout->IsRepeatingKey(comboChar));
            aDecodedKeys.iCount = 1;
            }

        // We've finished processing the dead key sequence:
        iDeadKey.iUnicode = 0;
        }
    }

// ----------------------------------------------------------------------
