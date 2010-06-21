/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
#include <w32std.h>
#include <coedef.h>
#include <eiksvdef.h>
#include <apgcli.h>
#include <apgtask.h>
#include <e32keys.h>
#include <avkondomainpskeys.h>

#include "hidtranslate.h"
#include "finder.h"
#include "keyboard.h"
#include "layoutmgr.h"
#include "debug.h"
#include "timeouttimer.h"
#include "layoututils.h"
#include "bthidPsKey.h"

// ----------------------------------------------------------------------

// Application UIDs for finding window groups.
const TInt KIdleAppUid = 0x102750f0;
const TInt KPhoneAppUid = 0x100058b3;
const TInt KMenuAppUid = 0x101f4cd2;
const TInt KSysApUid = 0x100058f3;
const TInt KActiveNotesUid = 0x10281a31;
const TInt KMessagingUid = 0x100058C5;
const TInt KServicesUid = 0x10008D39;
//const TInt KMultimediaMenuUid = 0x10281cfb;

// Key repeat parameters to be configured
const TInt KRepeatEndTimeout = 5000000; // 5 seconds

const TInt KKeyRepeatDelay = 500000;
const TInt KKeyRepeatInterval = 75000;

// The bitmap of modifier byte is defined in the HID spec.
// 8.3 Report Format for Array Items (HID1_11.pdf p56)
// 
// bit  description     mask
// -------------------------
// 0    LEFT CTRL     = 0x01
// 1    LEFT SHIFT    = 0x02
// 2    LEFT ALT      = 0x04
// 3    LEFT GUI      = 0x08
// 4    RIGHT CTRL    = 0x10
// 5    RIGHT SHIFT   = 0x20
// 6    RIGHT ALT     = 0x40
// 7    RIGHT GUI     = 0x80

const TUint KHidModifierCtrl = 0x01;
const TUint KHidModifierCtrlRight = 0x10;
const TUint KHidModifierAlt = 0x04;
const TUint KHidModifierAltRight = 0x40;
const TUint KHidModifierShift = 0x02;
const TUint KHidModifierShiftRight = 0x20;

//----------------------------------------------------------------------------
// CHidKeyboardDriver::CHidKeyboardDriver
//----------------------------------------------------------------------------
//
CHidKeyboardDriver::CHidKeyboardDriver(MDriverAccess* aGenericHid) :
    iDriverState(EUninitialised), iGenericHid(aGenericHid), iMmKeyDown(0)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::CHidKeyboardDriver(0x%08x)"), aGenericHid));
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::NewLC
//----------------------------------------------------------------------------
//
CHidKeyboardDriver* CHidKeyboardDriver::NewLC(MDriverAccess* aGenericHid)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::NewLC(0x%08x)"), aGenericHid));
    CHidKeyboardDriver* self = new (ELeave) CHidKeyboardDriver(aGenericHid);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::NewL
//----------------------------------------------------------------------------
//
CHidKeyboardDriver* CHidKeyboardDriver::NewL(MDriverAccess* aGenericHid)
    {
    CHidKeyboardDriver* self = CHidKeyboardDriver::NewLC(aGenericHid);
    CleanupStack::Pop();
    return self;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::ConstructL
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::ConstructL()
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver{0x%08x}::ConstructL()"), this));

    User::LeaveIfNull(iGenericHid);

    // Create a session with the window server:
    User::LeaveIfError(iWsSession.Connect());

    // Create a session with the layout manager:
    User::LeaveIfError(iLayoutMgr.Connect());

    // We also need a key repeat timer:
    iRepeatTimer = CPeriodic::NewL(CActive::EPriorityStandard);

    // repeat ending timer
    iRepeatEndTimer = CTimeOutTimer::NewL(EPriorityNormal, *this);
    iAppMenuId = AppMenuId();
    iPhoneAppId = PhoneAppId();
    iIdleAppId = IdleAppId();

    iSettings = CBtHidSettings::NewL();
    
    // create a keylock session
/*    TInt err = iKeyLock.Connect();
    TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::ConstructL: key lock err = %d"), err));
    User::LeaveIfError(err);    */
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::~CHidKeyboardDriver
//----------------------------------------------------------------------------
//
CHidKeyboardDriver::~CHidKeyboardDriver()
    {
        TRACE_INFO( (_L("[HID]\t~CHidKeyboardDriver() 0x%08x"), this));

    CancelAllKeys();
    delete iRepeatTimer;
    delete iRepeatEndTimer;
    iWsSession.Close();
    iLayoutMgr.Close();

    for (TInt i = 0; i < KNumInputFieldTypes; ++i)
        {
        iKeys[i].Reset();
        }

    if (iSettings)
        delete iSettings;

//    iKeyLock.Close();
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver:::StartL
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::StartL(TInt aConnectionId)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::StartL")));
    aConnectionId = aConnectionId;

    // No keys are pressed:
    iModifiers = 0;
    for (TInt i = 0; i < KNumInputFieldTypes; ++i)
        {
        if (iField[i])
            {
            // Reset the keys pressed array to all zeros
            RArray<TInt>& keys = iKeys[i];

            for (TInt j = 0; j < keys.Count(); j++)
                {
                keys[j] = 0;
                }
            }
        }

    // Start up with Num Lock active:
    iLockState = ENumLock;
    SetKeyboardLeds();

    //In case of reconnection, try to initialize the Keybord driver...
    InitialiseL(aConnectionId);

    // Reset the state of the layout decoder:
    iLayoutMgr.Reset();

    TInt initialLayout;
    User::LeaveIfError(iLayoutMgr.GetLayout(initialLayout));
    iLastSelectedLayout = iSettings->LoadLayoutSetting();
    TBool sameCategory = CLayoutUtils::SameCategory(
            static_cast<THidKeyboardLayoutId> (initialLayout),
            iLastSelectedLayout);
    if (sameCategory)
        {
        //Used the layoutID from CenRep
        iLayoutMgr.SetLayout(iLastSelectedLayout);
        }

    // Ready to process keyboard events:
    iDriverState = EInitialised;

    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::InitialiseL
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::InitialiseL(TInt aConnectionId)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::InitialiseL(%d)"),
                        aConnectionId));

    // Store the connection ID:
    iConnectionId = aConnectionId;

    // Initialise the layout manager:
    TInt country = iGenericHid->CountryCodeL(aConnectionId);
    TInt vendor = iGenericHid->VendorIdL(aConnectionId);
    TInt product = iGenericHid->ProductIdL(aConnectionId);

        TRACE_INFO( (
                        _L("[HID]\t  Country = %d, vendor = %d, product = %d"),
                        country, vendor, product));

    User::LeaveIfError(iLayoutMgr.SetInitialLayout(country, vendor, product));
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::Stop
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::Stop()
    {
    iDriverState = EDisabled;
    CancelAllKeys();
    }

// ----------------------------------------------------------------------
// CHidDriver mandatory functions:

//----------------------------------------------------------------------------
// CHidKeyboardDriver::DataIn
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::DataIn(CHidTransport::THidChannelType aChannel,
        const TDesC8& aPayload)
    {
    TInt err = KErrNone;
    switch (aChannel)
        {
        case CHidTransport::EHidChannelInt:
            if (EInitialised == iDriverState)
                {
                InterruptData(aPayload);
                }
            break;

        case CHidTransport::EHidChannelCtrl:
            break;

        default:
            break;
        }
    return err;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::CommandResult
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::CommandResult(TInt /*aCmdAck*/)
    {
    // No implementation as we don't issue any requests to be acknowledged
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::Disconnected
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::Disconnected(TInt aReason)
    {
    RDebug::Print(_L("CHidKeyboardDriver::Disconnected(%d)"), aReason);
    Stop();
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::InterruptData
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::InterruptData(const TDesC8& aPayload)
    {
    // If the report has a report ID, it is in the first byte.
    // If not, this value is ignored (see CField::IsInReport()).
    //
    TInt firstByte = aPayload[0];

        TRACE_INFO( (
                        _L("[HID]\tCHidKeyboardDriver::InterruptData(), report %d, length %d"),
                        firstByte, aPayload.Length()));
#ifdef _DEBUG

    for (TInt ii = 0; ii < aPayload.Length(); ii++)
        {
        TInt nextByte = aPayload[ii];
        DBG(RDebug::Print(
                        _L("[HID]\tCHidKeyboardDriver::InterruptData()  report[%d] =  %d"),
                        ii, nextByte));
        }
#endif
    // First check for any rollover errors:
    //
    TInt i;
    for (i = 0; i < KNumInputFieldTypes; ++i)
        {
        if (iField[i] && iField[i]->IsInReport(firstByte)
                && (iField[i]->UsagePage() == EUsagePageKeyboard))
            {
            if (IsRollover(i, aPayload))
                {
                CancelAllKeys();
                iLayoutMgr.Reset();
                return;
                }
            }
        }

    // Update the modifier state:
    //
    if (iField[EModifierKeys] && iField[EModifierKeys]->IsInReport(firstByte))
        {
        UpdateModifiers(EModifierKeys, aPayload);
        }

    // Finally, look for key events in all fields in this report:
    //
    for (i = 0; i < KNumInputFieldTypes; ++i)
        {
        if (iField[i] && iField[i]->IsInReport(firstByte))
            {
            ProcessKeys(i, aPayload);
            }
        }
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::IsRollover
//
// IsRollover() checks to see if the keyboard is in a rollover state
// (where too many keys are pressed to be able to give a sensible
// report).
//
// Rollover can only happen for fields with usage page 0x07.
//
// Returns ETrue if the keyboard is in a rollover state
//----------------------------------------------------------------------------
//
TBool CHidKeyboardDriver::IsRollover(TInt aFieldIndex, const TDesC8& aReport) const
    {
    TBool rollover = ETrue;

    TReportTranslator report(aReport, iField[aFieldIndex]);

    for (TInt i = 0; rollover && (i < report.Count()); ++i)
        {
        const TInt KRolloverError = 1;
        TInt usage;

        if (KErrNone == report.GetUsageId(usage, i) && usage
                != KRolloverError)
            {
            rollover = EFalse;
            }
        }

    return rollover;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::ProcessKeys
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::ProcessKeys(TInt aFieldIndex, const TDesC8& aReport)
    {
        TRACE_FUNC
    ((_L("[HID]\tCHidKeyboardDriver::ProcessKeys()")));

    // Get the appropriate field and key press array
    const CField* field = iField[aFieldIndex];
    RArray<TInt>& keysPressed = iKeys[aFieldIndex];

    TInt bufferSize = keysPressed.Count();
    TReportTranslator report(aReport, field);

        // 1. For each key that appeared in the last report but does
        //    not appear in this report, send a key UP event.

        TRACE_FUNC
    ((_L("[HID]\t  Testing for keys up")));

    TInt i;
    for (i = 0; i < bufferSize; ++i)
        {
        TInt key = keysPressed[i];

        if (key)
            {
            TInt pressed = EFalse;

            if ((report.GetValue(pressed, key) == KErrNone) && (!pressed))
                {
                    TRACE_INFO( (_L("[HID]\t  Key up: %d [0x%04x]"), key, key));
                KeyUp(key, field->UsagePage());
                }
            }
        }

        // 2. For each key that appears in this report, but does
        //    not appear in the last report, send a key DOWN event.

        TRACE_FUNC
    ((_L("  Testing for keys down")));

    for (i = 0; i < report.Count(); ++i)
        {
        TInt key;

        if ((KErrNone == report.GetUsageId(key, i)) && (key != 0))
            {
            // Was the key present (pressed) in the last report?
            TBool match = EFalse;

            for (TInt j = 0; !match && (j < bufferSize); ++j)
                {
                match = (key == keysPressed[j]);
                }

            // If it's a newly-pressed key then send a KeyDown
            if (!match)
                {
                    TRACE_INFO( (_L("[HID]\t  Key down: %d [0x%02x]"), key, key));
                if (iInputHandlingReg->AllowedToHandleEvent(
                        EUsagePageKeyboard, key))
                    {
                    iInputHandlingReg->AddHandledEvent(
                            EUsagePageGenericDesktop, key);
                    KeyDown(key, field->UsagePage());
                    UpdateLockState(key);
                    }
                }
            }
        }

    // 3. Finally, record what keys are down:

    TInt index = 0;
    for (i = 0; (i < report.Count()) && (index < bufferSize); ++i)
        {
        TInt key;

        if (KErrNone == report.GetUsageId(key, i) && key != 0)
            {
            keysPressed[index++] = key;
            }
        }
    while (index < bufferSize)
        {
        keysPressed[index++] = 0;
        }
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::KeyEvent
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::KeyEvent(TBool aIsKeyDown, TInt aHidKey,
        TInt aUsagePage)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::KeyEvent(%d, %d, %d)"), aIsKeyDown,
                        aUsagePage, aHidKey));

    // Any type of key event will cancel a repeating key:
    //
    if (iRepeatTimer->IsActive())
        {
        iRepeatTimer->Cancel();
        }

    if (iRepeatEndTimer->IsActive())
        {
        iRepeatEndTimer->Cancel();
        }

    // Pass the key info to the keyboard layout translator:
    //
    TDecodedKeyInfo decodedKeys;
    TInt err = iLayoutMgr.KeyEvent(aIsKeyDown, aHidKey, aUsagePage,
            iModifiers, TLockKeys(iLockState & ECapsLock, iLockState
                    & ENumLock), decodedKeys);

    TTranslatedKey& key = decodedKeys.iEvent[0];
    if (err == KErrNone)
        {
        // Send the "key down" or "key up" event to the system:
        // Handle special application launch key combinations
        TRAPD( err, HandleApplicationLaunchKeysL(decodedKeys.iScanCode, aIsKeyDown, iModifiers));
        if (KErrNone != err)
            {
                TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::KeyEvent:Launch failed")));
            }

        //check if the key is multimedia key that needs to be sent as RawEvent and send it.
        // if key event is consumed don't send it anymore.
        if (!HandleKeyMapping(decodedKeys, aIsKeyDown, iModifiers))
            {
            if (decodedKeys.iScanCode != EStdKeyNull)
                {
                TKeyEvent event =
                        TKeyEventFromScanCode(decodedKeys.iScanCode);

                // remove other modifiers than autorepeat.
                if (event.iScanCode == EStdKeyDevice3)
                    {
                    event.iModifiers &= EModifierAutorepeatable;
                    }

                    //Number key events to phone app are handled differently.
                    TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::KeyEvent: ScanCode: %d %c"), decodedKeys.iScanCode, decodedKeys.iScanCode));
                TInt unikey = key.iUnicode;
                //Send key codes differently to idle app
                if ((IsDigitKey(decodedKeys.iScanCode) || IsSpecialHandleKey(
                        unikey) || decodedKeys.iScanCode == EStdKeyYes
                        || decodedKeys.iScanCode == EStdKeyBackspace
                        || decodedKeys.iScanCode == EStdKeyNo)
                        && IsPhoneAppTopMost())
                    {
                        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::KeyEvent: Send event %c to idle editor"), unikey));
                    HandleTelephoneAppKeys(decodedKeys.iScanCode, unikey,
                            aIsKeyDown);
                    }
                else
                    SendToWindowGroup(event, aIsKeyDown
                                                        ? EEventKeyDown
                                                           : EEventKeyUp);
                }

            // Send any "key press" events to the system, if applicable:
            //
            for (TInt i = 0; i < decodedKeys.iCount; ++i)
                {
                TTranslatedKey & key = decodedKeys.iEvent[i];

                SendKeyPress(key.iUnicode, key.iUsagePage, key.iScanCode,
                        key.iIsRepeatingKey);
                }
            }
        }
    else
        {
            TRACE_FUNC
        ((_L("[HID]\tKeyboard event translation failed!")));
        }
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::TKeyEventFromScanCode
//----------------------------------------------------------------------------
//
TKeyEvent CHidKeyboardDriver::TKeyEventFromScanCode(TInt aScanCode) const
    {
    TKeyEvent event;

    event.iScanCode = aScanCode;
    event.iModifiers = KeyEventModifiers();

    // The following are always zero for a key down or key up event:
    event.iCode = 0;
    event.iRepeats = 0;

    return event;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::IsDigitKey
//----------------------------------------------------------------------------
//
TBool CHidKeyboardDriver::IsDigitKey(TInt aScanCode)
    {
    TBool ret = EFalse;

    if (aScanCode >= '0' && aScanCode <= '9')
        {
        ret = ETrue;
        }

    return ret;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::IsDigit
//----------------------------------------------------------------------------
//
TBool CHidKeyboardDriver::IsSpecialHandleKey(TInt aUniCode)
    {
    TBool ret = EFalse;

    if (aUniCode == '+' || aUniCode == '*' || aUniCode == '#' || aUniCode
            == 'w' || aUniCode == 'p')
        {
        ret = ETrue;
        }

    return ret;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::HandleTelephoneAppKeys
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::HandleTelephoneAppKeys(TInt aScanCode,
        TInt aUniCode, TBool aIsKeyDown)
    {
    // Telephone view / idle view can't handle proper key events, but the key events must be sent as
    // raw events.
    TRawEvent rawEvent;
    TUint modifier = 0;
    TInt scancode = aScanCode;
    switch (aUniCode)
        {
        case '+':
        case EStdKeyNkpPlus:
            scancode = EStdKeyNkpPlus;
            modifier = EModifierLeftShift | EModifierShift;
            break;
        case '*':
            //case EStdKeyNkpAsterisk:
            scancode = EStdKeyNkpAsterisk;
            modifier = 0; //EModifierLeftShift | EModifierShift;
            break;
        case '#':
            //case EStdKeyHash:
            scancode = EStdKeyHash;
            modifier = EModifierLeftShift | EModifierShift;
            break;
        case 'w':
            modifier = EModifierLeftShift | EModifierShift;
            break;
        case 'p':
            modifier = EModifierLeftShift | EModifierShift;
            break;
        default:
            break;
        }
        TRACE_INFO( (_L("[HID]\tHIDKeyboard: HandleTelephoneAppKeys: Overridden Scancode: %d, Modifier: %d"), scancode, modifier));
        TRACE_INFO( (_L("[HID]\tHIDKeyboard: HandleTelephoneAppKeys: Original unicode: %d"), aUniCode));

    if (aIsKeyDown)
        rawEvent.Set(TRawEvent::EKeyDown, scancode);
    else
        rawEvent.Set(TRawEvent::EKeyUp, scancode); //This will never be called for +,* and #

    TEventModifier ab = (TEventModifier) -1; //all bits set to 1.
    TModifierState ba = (TModifierState) modifier;
        TRACE_INFO( (_L("[HID]\t   modifiers %d, modifierState: %d"), ab, (TUint)ba));

    iWsSession.SetModifierState(ab, ba);
    iWsSession.SimulateRawEvent(rawEvent);
    iWsSession.Flush();
        TRACE_FUNC
    ((_L("[HID]\tHIDKeyboard: HandleTelephoneAppKeys: Event sent!")));
    // Plus, Asterix and Hash keys need to send separate KeyUp event too.
    if (aUniCode == '+' || aUniCode == '*' || aUniCode == '#' || aUniCode
            == 'p' || aUniCode == 'w')
        {
        rawEvent.Set(TRawEvent::EKeyUp, scancode);
        iWsSession.SimulateRawEvent(rawEvent); //Key up event
        iWsSession.Flush();
            TRACE_FUNC
        ((_L("[HID]\tHIDKeyboard: HandleTelephoneAppKeys: Separate Key up event sent!")));
        }
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::CancelKeysForField
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::CancelKeysForField(TInt aFieldIndex)
    {
    //TRACE_FUNC((_L("[HID]\tCHidKeyboardDriver::CancelKeysForField(%d)"),
    if (iField[aFieldIndex])
        {
        for (TInt i = 0; i < iField[aFieldIndex]->Count(); ++i)
            {
            TInt key = iKeys[aFieldIndex][i];
            //TRACE_FUNC((_L("[HID]\t    Cancel key@%d=%02x (%d)"),
            //        i, key, key));
            if (key != 0)
                {
                KeyUp(key, iField[aFieldIndex]->UsagePage());
                iKeys[aFieldIndex][i] = 0;
                }
            }
        }
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::CancelAllKeys
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::CancelAllKeys()
    {
        TRACE_FUNC
    ((_L("[HID]\tCHidKeyboardDriver::CancelAllKeys()")));

    // Stop the key repeat timer:
    if (iRepeatTimer->IsActive())
        {
        iRepeatTimer->Cancel();
        }

    // Stop the key repeat safety timer:
    if (iRepeatEndTimer->IsActive())
        {
        iRepeatEndTimer->Cancel();
        }

    // Send key up events for any pressed keys:
    for (TInt i = 0; i < KNumInputFieldTypes; ++i)
        {
        CancelKeysForField(i);
        }

    // No modifier keys pressed:
    iModifiers = 0;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::UpdateModifiers
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::UpdateModifiers(TInt aFieldIndex,
        const TDesC8& aReport)
    {
        TRACE_FUNC
    ((_L("[HID]\tCHidKeyboardDriver::UpdateModifiers()")));
        TRACE_INFO( (_L("[HID]\t  old iModifiers = %02x"), iModifiers));

    const TInt KLeftControl = 224; // => bit 0 of modifier bitmask
    const TInt KRightGui = 231; // => bit 7

    TUint8 newModifiers = 0;

    // Translate the HID usage values into a boot protocol style
    // modifier bitmask:
    //
    TReportTranslator report(aReport, iField[aFieldIndex]);
    for (TInt i = KLeftControl; i <= KRightGui; i++)
        {
        TInt value;

        if (KErrNone == report.GetValue(value, i) && value)
            {
            newModifiers |= (1 << (i - KLeftControl));
            }
        }

    // Finally, record the new state:
    //
    iModifiers = newModifiers;

        TRACE_INFO( (_L("[HID]\t  new iModifiers = %02x"), iModifiers));
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::UpdateLockState
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::UpdateLockState(TInt aHidKey)
    {
    const TInt KHidCapsLock = 57;
    const TInt KHidScrollLock = 71;
    const TInt KHidNumLock = 83;

    TUint oldLockState = iLockState;

    switch (aHidKey)
        {
        case KHidNumLock:
            iLockState ^= ENumLock;
            break;
        case KHidCapsLock:
            iLockState ^= ECapsLock;
            break;
        case KHidScrollLock:
            iLockState ^= EScrollLock;
            break;
        }

    if (oldLockState != iLockState)
        {
            TRACE_INFO( (_L("[HID]\tChanged lock state: %d -> %d"),
                            oldLockState, iLockState));
        SetKeyboardLeds();
        }
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::KeyEventModifiers
//----------------------------------------------------------------------------
//
TUint32 CHidKeyboardDriver::KeyEventModifiers() const
    {
    // Modifier state:

    const TInt KNumModifierKeys = 8;

    const TUint32 KModifierMask[KNumModifierKeys] =
        {
        EModifierLeftCtrl | EModifierCtrl, // 0x  A0  (report bit 0)
                EModifierLeftShift | EModifierShift, // 0x 500
                EModifierLeftAlt | EModifierAlt, // 0x  14
                EModifierLeftFunc | EModifierFunc, // 0x2800
                EModifierRightCtrl | EModifierCtrl, // 0x  C0
                EModifierRightShift | EModifierShift, // 0x 600
                EModifierRightAlt | EModifierAlt, // 0x  18
                EModifierRightFunc | EModifierFunc
        // 0x3000  (report bit 7)
            };

    TUint32 result = 0;

    TUint bitPosn = 1;
    for (TInt i = 0; i < KNumModifierKeys; ++i)
        {
        if (iModifiers & bitPosn)
            {
            result |= KModifierMask[i];
            }
        bitPosn <<= 1;
        }

    // Lock state:

    if (iLockState & ECapsLock)
        {
        result |= EModifierCapsLock; // 0x 4000
        }

    if (iLockState & ENumLock)
        {
        result |= EModifierNumLock; // 0x 8000
        }

    if (iLockState & EScrollLock)
        {
        result |= EModifierScrollLock; // 0x10000
        }

    // Flag used by the FEP to identify events from the HID keyboard
    const TUint KHidKeyboardFepFlag = 0x00200000;
    result |= KHidKeyboardFepFlag;

    return result;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::SetKeyboardLeds
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::SetKeyboardLeds() const
    {
    // Not supported at present
    }

// ----------------------------------------------------------------------

//----------------------------------------------------------------------------
// CHidKeyboardDriver::OnKeyRepeat
// Key repeat call-back:
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::OnKeyRepeat()
    {
    // Send another key-press event:
    iLastKey.iRepeats = 1;
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::OnKeyRepeat() event [%d, %d, 0x%06x, %d]"),
                        iLastKey.iCode, iLastKey.iScanCode,
                        iLastKey.iModifiers, iLastKey.iRepeats));
    SendToWindowGroup(iLastKey, EEventKey);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelController::VolumePSChangeTimedoutCallback
// -----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::TimerFiredOnKeyRepeat(TAny* aThis)
    {
    return reinterpret_cast<CHidKeyboardDriver*> (aThis)->OnKeyRepeat();
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::SendKeyPress
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::SendKeyPress(TUint16 aUnicodeKey, TInt aUsagePage,
        TInt aScanCode, TBool aIsRepeatingKey)
    {

        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::SendKeyPress([%d], %d, %d)"),
                        aUnicodeKey, aScanCode, aIsRepeatingKey));

#ifdef _DEBUG
    //Please the compiler
    //TBuf<256> winGroupName;
    //iWsSession.GetWindowGroupNameFromIdentifier(iWsSession.GetFocusWindowGroup(), winGroupName);
    //TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver: Focused Window group:<%S>"), winGroupName));
#endif

    //Number entry to Telephone app is handled differently.
    if ((IsDigitKey(aScanCode) || IsSpecialHandleKey(aUnicodeKey) || aScanCode == EStdKeyBackspace )
            && IsPhoneAppTopMost())
        return;

    TKeyEvent event = TKeyEventFromScanCode(aScanCode);
    event.iCode = aUnicodeKey;

    if (aIsRepeatingKey)
        {
        event.iModifiers |= EModifierAutorepeatable;
        }

    // Cancel any current key repeat:
    if (iRepeatTimer->IsActive())
        {
        iRepeatTimer->Cancel();
        }

    // Send the key press event:

    const TInt KConsumerControlPage = 0x0c;

    // Number entry in telephony does not activate unless we send the key event through WindowServer.
    if (aUsagePage == KConsumerControlPage)

        {
        // Send media keys using RWsSession::SimulateKeyEvent() so
        // that the window server "capture key" function will
        // operate. Note that this means that media keys won't have
        // standard repeat key functionality because we can't set
        // the iRepeats member using SimulateKeyEvent().
        SendToWindowServer(event);
        }
    else
        {
        //If Chr-key, send '*' to open Special Character Dialog, but not if idle app (telephone).
        if (event.iCode == EKeyApplication2)
            {
                TRACE_FUNC
            ((_L("[HID]\tCHidKeyboardDriver::SendKeyPress, AltGr pressed")));
            if (IsPhoneAppTopMost()) //If idle app top-most, don't send '*'.
                return;
            event.iCode = '*';
            event.iScanCode = EStdKeyNkpAsterisk;
            event.iModifiers = 0;
            }
        //If Enter-key, check if AppMenu or Phone/Idle is topmost --> make it Selection key!
        if (event.iCode == EKeyEnter)
            {
            if (IsApplicationMenuTopMost() || IsPhoneAppTopMost())
                {
                    TRACE_FUNC
                ((_L("[HID]\tCHidKeyboardDriver::SendKeyPress, Change enter to selection")));
                event.iCode = EKeyDevice3;
                event.iScanCode = EStdKeyDevice3;
                }
            }

        // remove other modifiers than autorepeat.
        if (event.iScanCode == EStdKeyDevice3)
            {
            event.iModifiers &= EModifierAutorepeatable;
            }

        // Send all other keys using RWsSession::SendEventToWindowGroup():
        SendToWindowGroup(event, EEventKey);

        // Set up a key repeat timer, if necessary:
        if (aIsRepeatingKey)
            {
            iLastKey = event;
            TTimeIntervalMicroSeconds32 KDefaultDelay = KKeyRepeatDelay;
            TTimeIntervalMicroSeconds32 KDefaultRate = KKeyRepeatInterval;
            iRepeatTimer->Start(KDefaultDelay, KDefaultRate, TCallBack(
                    CHidKeyboardDriver::TimerFiredOnKeyRepeat, this));
            if (!iRepeatEndTimer->IsActive())
                {
                DBG(RDebug::Print(_L("[HID]\tCHidKeyboardDriver::SendKeyPress, start repeat ending timer")));
                iRepeatEndTimer->After(KRepeatEndTimeout);
                }
            }
        }
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::IsPhoneAppTopMost
// Check if Application Menu is the active application receiving the key presses
//----------------------------------------------------------------------------
//
TBool CHidKeyboardDriver::IsApplicationMenuTopMost()
    {
    TBool ret = EFalse;
    if (iAppMenuId == KErrNotFound) //was not found earlier in Constructor (WKB app launched at boot)
        iAppMenuId = AppMenuId(); //if in constructor the AppMenu was not active, then ask is it now
    TInt focusedId = iWsSession.GetFocusWindowGroup();
    if (focusedId == iAppMenuId)
        ret = ETrue;
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::IsApplicationMenuTopMost(): %d"), ret ));
    return ret;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::IsPhoneAppTopMost
//----------------------------------------------------------------------------
//
TBool CHidKeyboardDriver::IsPhoneAppTopMost()
    {
    TBool ret = EFalse;
    if (iPhoneAppId == KErrNotFound) //was not found earlier in Constructor (WKB app launched at boot)
        iPhoneAppId = PhoneAppId(); //if in constructor the AppMenu was not active, then ask is it now
    if (iIdleAppId == KErrNotFound)
        iIdleAppId = IdleAppId();

    TInt focusedId = iWsSession.GetFocusWindowGroup();
    if (focusedId == iPhoneAppId || focusedId == iIdleAppId) //KPhoneIdleViewUid || focusedId == KIdleAppUid)
        ret = ETrue;
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::IsPhoneAppTopMost(): %d"), ret ));
    return ret;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::AppMenuId
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::AppMenuId()
    {
    TApaTaskList taskList(iWsSession);
    TApaTask task = taskList.FindApp(TUid::Uid(KMenuAppUid));
    TInt Id = task.WgId();
    DBG(if (KErrNotFound == Id) RDebug::Print(_L("[HID]\tCHidKeyboardDriver::AppMenuId(): Menu not found!")));

    return Id;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::PhoneAppId
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::PhoneAppId()
    {

    TApaTaskList taskList(iWsSession);
    TApaTask task = taskList.FindApp(TUid::Uid(KPhoneAppUid));
    TInt Id = task.WgId();
    DBG(if (KErrNotFound == Id) RDebug::Print(_L("[HID]\tCHidKeyboardDriver::PhoneAppId(): Phone not found!")));

    return Id;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::IdleAppId
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::IdleAppId()
    {
    TApaTaskList taskList(iWsSession);
    TApaTask task = taskList.FindApp(TUid::Uid(KIdleAppUid));
    TInt Id = task.WgId();
    DBG(if (KErrNotFound == Id) RDebug::Print(_L("[HID]\tCHidKeyboardDriver::IdleAppId(): Idle not found!")));

    return Id;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::SendToWindowServer
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::SendToWindowServer(TKeyEvent aKeyEvent)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::SendToWindowServer, event: kc 0x%08x, sc 0x%08x, mod 0x%06x, rep %d]"),
                        aKeyEvent.iCode, aKeyEvent.iScanCode,
                        aKeyEvent.iModifiers, aKeyEvent.iRepeats));

    // Prevent the window server generating key repeats as we do this ourselves
    aKeyEvent.iModifiers &= ~EModifierAutorepeatable;
    iWsSession.SimulateKeyEvent(aKeyEvent);
    iWsSession.Flush();
        TRACE_FUNC
    ((_L("[HID]\tCHidKeyboardDriver::SendToWindowServer, Event sent!")));
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::SendToWindowGroup
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::SendToWindowGroup(const TKeyEvent& aKeyEvent,
        TEventCode aType)
    {

        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::SendToWindowGroup, Event: type %d, kc 0x%08x, sc 0x%08x, mod 0x%06x, rep %d]"),
                        aType, aKeyEvent.iCode, aKeyEvent.iScanCode,
                        aKeyEvent.iModifiers, aKeyEvent.iRepeats));

    // Using RWsSession::SendEventToWindowGroup() bypasses the normal
    // key handling in the window server, and so capture keys don't
    // work.  Therefore we check for the known captured keys in
    // WindowGroupForKeyEvent().

    TInt wGroup = WindowGroupForKeyEvent(aKeyEvent, aType);

    if (0 < wGroup)
        {
            TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::SendToWindowGroup: Send event to window group %d"), wGroup));

        TWsEvent event;

        event.SetType(aType);
        event.SetHandle(0);

        *(event.Key()) = aKeyEvent;

        // Remove shift modifiers from call key events because
        // the phone application ignores them otherwise
        //
        if (EStdKeyYes == aKeyEvent.iScanCode || EStdKeyNo
                == aKeyEvent.iScanCode)
            {
            event.Key()->iModifiers &= ~(EModifierLeftShift
                    | EModifierRightShift | EModifierShift);
            }

        // don't send ctrl and shift keys.
        if (EStdKeyLeftShift != aKeyEvent.iScanCode && EStdKeyRightShift
                != aKeyEvent.iScanCode && EStdKeyLeftCtrl
                != aKeyEvent.iScanCode && EStdKeyRightCtrl
                != aKeyEvent.iScanCode)
            {
            iWsSession.SendEventToWindowGroup(wGroup, event);
            iWsSession.Flush();
                TRACE_FUNC
            ((_L("[HID]\tCHidKeyboardDriver::SendToWindowGroup: Event sent!")));
            }
        else
            TRACE_FUNC
        ((_L("[HID]\tCHidKeyboardDriver::SendToWindowGroup: Shift or ctrl not sent!")));

        }

    // Keypresses should reset the backlight timer:
    User::ResetInactivityTime();
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::WindowGroupForKeyEvent
// Determine which window group a key event should be sent to.
// Except for special cases this is the window group with the focus.
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::WindowGroupForKeyEvent(const TKeyEvent& aKeyEvent,
        TEventCode aType)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::WindowGroupForKeyEvent: type %d, kc 0x%08x, sc 0x%08x, mod 0x%06x, rep %d]"),
                        aType, aKeyEvent.iCode, aKeyEvent.iScanCode,
                        aKeyEvent.iModifiers, aKeyEvent.iRepeats));
    _LIT(KBackDrop, "*EiksrvBackdrop*");

    if (EStdKeyApplication0 == aKeyEvent.iScanCode && (EEventKeyDown == aType
            || EEventKeyUp == aType))
        {
        // Application key up/down events go to the Eikon server
        // Use this old way for application key
        TInt result = iWsSession.FindWindowGroupIdentifier(0, KBackDrop); //This was in A2.x __EIKON_SERVER_NAME
        DBG(if (KErrNotFound == result) RDebug::Print(_L("[HID]\tCHidKeyboardDriver::WindowGroupForKeyEvent(): BackDropWindowGroup Name not found!")));
        return result;
        }

    if (EKeyDevice2 == aKeyEvent.iCode && EEventKey == aType)
        {
        // Power key press events go to SysAp
        TApaTaskList taskList( iWsSession );
        TApaTask task = taskList.FindApp( TUid::Uid( KSysApUid ) );
        TInt result = task.WgId();
        DBG(if (KErrNotFound == result) RDebug::Print(_L("[HID]\tCHidKeyboardDriver::WindowGroupForKeyEvent(): SysApWindowGroup Name not found!")))
        ;
        return result;
        }

    if (EStdKeyNo == aKeyEvent.iScanCode
            && (EEventKeyDown == aType || EEventKeyUp == aType))
        {
        // End key up/down events go to the phone app
        TApaTaskList taskList( iWsSession );
        TApaTask task = taskList.FindApp( TUid::Uid( KPhoneAppUid ) );
        TInt result = task.WgId();
        DBG(if (KErrNotFound == result) RDebug::Print(_L("[HID]\tCHidKeyboardDriver::WindowGroupForKeyEvent(): PhoneAppWindowGroup Name not found!")))
        ;
        return result;
        }

    // All other key events go to the window group with the keyboard focus
    return iWsSession.GetFocusWindowGroup();
    }

// ----------------------------------------------------------------------

//----------------------------------------------------------------------------
// CHidKeyboardDriver::ResetArrayToSize
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::ResetArrayToSize(RArray<TInt>& aArray, TInt aSize)
    {
    TInt err = KErrNone;

    aArray.Reset();

    for (TInt i = 0; !err && (i < aSize); ++i)
        {
        err = aArray.Append(0);
        }

    return err;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::CanHandleReportL
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::CanHandleReportL(CReportRoot* aReportRoot)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboard::CanHandleReport(0x%08x)"),
                        aReportRoot));
    iSupportedFieldCount = 0;
    // Look for keyboard reports:

    THidFieldSearch search;

    TKeyboardFinder finder;
    search.SearchL(aReportRoot, &finder);
    iField[EStandardKeys] = finder.StandardKeyField();
    iField[EModifierKeys] = finder.ModifierKeyField();
    iLedField = finder.LedField();

    TConsumerKeysFinder mmFinder;
    search.SearchL(aReportRoot, &mmFinder);
    iField[EMediaKeys] = mmFinder.ConsumerKeysField();

    TPowerKeysFinder pwrFinder;
    search.SearchL(aReportRoot, &pwrFinder);
    iField[EPowerKeys] = pwrFinder.PowerKeysField();

    for (TInt i = 0; i < KNumInputFieldTypes; ++i)
        {
        if (iField[i])
            {
            iSupportedFieldCount++;
            User::LeaveIfError(ResetArrayToSize(iKeys[i], iField[i]->Count()));
            }
        }

    TInt valid = KErrHidUnrecognised;

    // We only require standard and modifier key reports; the
    // LED and consumer keys are optional:

    if ((iField[EStandardKeys] != 0) && (iField[EModifierKeys] != 0))
        {
        valid = KErrNone;
        }
        TRACE_INFO( (
                        _L("CHidKeyboard::CanHandleReport() returning %d"), valid));

    return valid;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::HandleApplicationLaunchKeysL
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::HandleApplicationLaunchKeysL(TUint16 aScancodeKey,
        TBool aIsKeyDown, TUint8 aHIDModifiers)
    {
    // Note that aHIDModifier is not Symbian Modifier bits, but in below format: (from HID spec)
    /*    const TUint32 KModifierMask[KNumModifierKeys] =
     {
     EModifierLeftCtrl   | EModifierCtrl,   // 0x  A0  (report bit 0)
     EModifierLeftShift  | EModifierShift,  // 0x 500       
     EModifierLeftAlt    | EModifierAlt,    // 0x  14
     EModifierLeftFunc   | EModifierFunc,   // 0x2800
     EModifierRightCtrl  | EModifierCtrl,   // 0x  C0
     EModifierRightShift | EModifierShift,  // 0x 600
     EModifierRightAlt   | EModifierAlt,    // 0x  18
     EModifierRightFunc  | EModifierFunc    // 0x3000  (report bit 7)
     };
     */
    const TUint KHIDModifierAlt = 0x04;

        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::HandleApplicationLaunchKeys: scancode 0x%08x, HIDmodifiers 0x%08x"), aScancodeKey, aHIDModifiers));

    switch (aScancodeKey)
        {
        // Alt-m launches MultimediaMenu
        case 'm':
        case 'M':
            {
            if (aHIDModifiers & KHIDModifierAlt)
                {
                    TRACE_FUNC
                ((_L("Launch multimediamenu")));
                //LaunchApplicationL( KMultimediaMenuUid );
                TInt err = SendRawEvent(EStdKeyApplication6, aIsKeyDown, 0);
                if (KErrNone != err)
                    {
                        TRACE_FUNC
                    ((_L("[HID]\t    Event sending failed")));
                    }
                }
            break;
            }

            // Alt-n launches ActiveNotes
        case 'n':
        case 'N':
            {
            if (aHIDModifiers & KHIDModifierAlt)
                {
                    TRACE_FUNC
                ((_L("[HID]\t   Launch active notes")));
                LaunchApplicationL(KActiveNotesUid);
                }
            break;
            }
        case EStdKeyF1:
            {
            DBG(RDebug::Print(_L("[HID]\t   Launch Web service")));
            LaunchApplicationL(KServicesUid);
            }
            break;
        case EStdKeyF2:
            {
            DBG(RDebug::Print(_L("[HID]\t   Launch messages")));
            LaunchApplicationL(KMessagingUid);
            }
            break;
        default:
            break;
        }
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::LaunchApplicationL
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::LaunchApplicationL(TInt aAppUid)
    {
        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::LaunchApplication: UID 0x%08x"), aAppUid));

    TApaTaskList taskList(iWsSession);
    TUid uid = TUid::Uid(aAppUid);
    TApaTask task = taskList.FindApp(uid);
    if (task.Exists())
        {
        // Application is active, so just bring to foreground
        task.BringToForeground();
        }
    else
        {
        RApaLsSession appArcSession;
        // connect to AppArc server
        User::LeaveIfError(appArcSession.Connect());

        CleanupClosePushL(appArcSession);

        TThreadId threadID;

        // Launch the application with no params.
        User::LeaveIfError(appArcSession.StartDocument(KNullDesC, uid,
                threadID));

        CleanupStack::Pop(1); // appArcSession

        appArcSession.Close();
        }

    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::HandleNokiaMultimediaKeys
// Takes some Multimedia keyevents from HID and converts them to Nokia raw events format
// to simulate as if the key presses came from phone keypad (volume +/-, play, pause, frwd, rewind)
//
// This is temporary solution that shall be replaced by "proper" RemCon Bearer solution.
// Because the keycodes used here may change in future terminal!
// Note that aHIDModifier is not Symbian Modifier bits, but in below format: (from HID spec)
//    const TUint32 KModifierMask[KNumModifierKeys] =
//        {
//        EModifierLeftCtrl   | EModifierCtrl,   // 0x  A0  (report bit 0)
//        EModifierLeftShift  | EModifierShift,  // 0x 500
//        EModifierLeftAlt    | EModifierAlt,    // 0x  14
//        EModifierLeftFunc   | EModifierFunc,   // 0x2800
//        EModifierRightCtrl  | EModifierCtrl,   // 0x  C0
//        EModifierRightShift | EModifierShift,  // 0x 600
//        EModifierRightAlt   | EModifierAlt,    // 0x  18
//        EModifierRightFunc  | EModifierFunc    // 0x3000  (report bit 7)
//        };
//----------------------------------------------------------------------------
//

TBool CHidKeyboardDriver::HandleKeyMapping(TDecodedKeyInfo& aKey,
    TBool aIsKeyDown,
    TUint8 aHIDModifiers)
    {
    TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::HandleKeyMapping")));
    
    TBool ret = EFalse;
    
    switch (aKey.iScanCode)
        {
        case EStdKeyUpArrow:
            {
            ret = HandleKeyMappingUp(aKey, aIsKeyDown, aHIDModifiers);
            break;
            }
        case EStdKeyDownArrow:
            {
            ret = HandleKeyMappingDown(aKey, aIsKeyDown, aHIDModifiers);
            break;
            }
        case EStdKeyLeftArrow:
            {
            ret = HandleKeyMappingLeft(aKey, aIsKeyDown, aHIDModifiers);
            break;
            }
        case EStdKeyRightArrow:
            {
            ret = HandleKeyMappingRight(aKey, aIsKeyDown, aHIDModifiers);
            break;
            }
        case EStdKeyEscape:
            // fall through
        case EStdKeyF8:
            // fall through
        case EStdKeyApplication2:
            // fall through
        case EStdKeyApplication3:
            // fall through
        case EStdKeyF9:
            // break;
        case EStdKeyApplication4:
            // fall through
        case EStdKeyF7:
            // fall through
        case EStdKeyApplication5:
            // fall through
        case EStdKeyF12:
            // fall through
        case EStdKeyIncVolume:
            // fall through
        case EStdKeyF11:
            // fall through
        case EStdKeyDecVolume:
            // fall through
        case EStdKeyF10:
            {
            ret = HandleKeyMappingOther(aKey, aIsKeyDown, aHIDModifiers);
            break;
            }
        default:
            {
            // no actions for these keys
            break;
            }
        }
    
    return ret;
    }

// ----------------------------------------------------------------------
// CHidKeyboardDriver::HandleKeyMappingUp
// Handle key mapping gor UP + MODIFIER
// ----------------------------------------------------------------------
//
TBool CHidKeyboardDriver::HandleKeyMappingUp(TDecodedKeyInfo& aKey,
    TBool aIsKeyDown,
    TUint8 aHIDModifiers)
    {
    TRACE_INFO((_L("[HID]\tCHidKeyboardDriver::HandleKeyMappingUp")));
    
    TBool ret = EFalse;
    TInt scancode = 0;
    TUint modifier = 0;
    TBool isMmKey = EFalse;
    TMmKeyDown bitmapToReset = ENone;
    
    switch (aKey.iScanCode)
        {
        case EStdKeyUpArrow:
            {
            // Key up and send key was emulated
            if (!aIsKeyDown &&
                iNavKeyDown & ESend)
                {
                TRACE_INFO((_L("[HID]\tCTRL + UP >>> SEND KEY UP")));

                aKey.iScanCode = EStdKeyYes;
                TTranslatedKey& key = aKey.iEvent[0];
                key.iIsRepeatingKey = aIsKeyDown;
                key.iScanCode = EStdKeyYes;
                key.iUnicode = EKeyYes;
                
                iNavKeyDown = (iNavKeyDown & !ESend);
                }
            
            //CTRL = Send key
            else if (aIsKeyDown &&
                aHIDModifiers & (KHidModifierCtrl | KHidModifierCtrlRight))
                {
                TRACE_INFO((_L("[HID]\tCTRL + UP >>> SEND KEY DOWN")));
                
                aKey.iScanCode = EStdKeyYes;
                aKey.iCount = 1;
                TTranslatedKey& key = aKey.iEvent[0];
                key.iIsRepeatingKey = aIsKeyDown;
                key.iScanCode = EStdKeyYes;
                key.iUnicode = EKeyYes;
                
                iNavKeyDown = (iNavKeyDown | ESend);
                }
            
            //ALT = Stop
            else if (aHIDModifiers & (KHidModifierAlt | KHidModifierAltRight) ||
                iMmKeyDown & EStop)
                {
                TRACE_INFO((_L("[HID]\tALT + UP >>> STOP")));
                
                scancode = EStdKeyApplication3;
                isMmKey = ETrue;
                bitmapToReset = EStop;
                }

            //SHIFT = Volume up
            else if (aHIDModifiers & (KHidModifierShift | KHidModifierShiftRight) ||
                iMmKeyDown & EVolUp)
                {
                TRACE_INFO((_L("[HID]\tSHIFT + UP >>> VOLUME UP")));
                
                scancode = EStdKeyIncVolume;
                isMmKey = ETrue;
                bitmapToReset = EVolUp;
                }
            break;
            }
        default:
            {
            // no actions
            break;
            }
        }

    if (isMmKey)
        {
        if (bitmapToReset)
            {
            ResetBitmap(aIsKeyDown, bitmapToReset);
            }
        SendRawEvent(scancode, aIsKeyDown, modifier);
        ret = ETrue;
        }
    
    return ret;
    }

// ----------------------------------------------------------------------
// CHidKeyboardDriver::HandleKeyMappingDown
// Handle key mapping gor DOWN + MODIFIER
// ----------------------------------------------------------------------
//
TBool CHidKeyboardDriver::HandleKeyMappingDown(TDecodedKeyInfo& aKey,
    TBool aIsKeyDown,
    TUint8 aHIDModifiers)
    {
    TRACE_INFO((_L("[HID]\tCHidKeyboardDriver::HandleKeyMappingDown")));

    TBool ret = EFalse;
    TInt scancode = 0;
    TUint modifier = 0;
    TBool isMmKey = EFalse;
    TMmKeyDown bitmapToReset = ENone;
    
    switch (aKey.iScanCode)
        {
        case EStdKeyDownArrow:
            {
            // Key up and end key was emulated
            if (!aIsKeyDown &&
                iNavKeyDown & EEnd)
                {
                TRACE_INFO((_L("[HID]\tCTRL + DOWN >>> END KEY UP")));

                aKey.iScanCode = EStdKeyNo;
                TTranslatedKey& key = aKey.iEvent[0];
                key.iIsRepeatingKey = aIsKeyDown;
                key.iScanCode = EStdKeyNo;
                key.iUnicode = EKeyNo;
                
                iNavKeyDown = (iNavKeyDown & !EEnd);
                }
            
            //CTRL = End key
            else if (aIsKeyDown &&
                aHIDModifiers & (KHidModifierCtrl | KHidModifierCtrlRight))
                {
                TRACE_INFO((_L("[HID]\tCTRL + DOWN >>> END KEY DOWN")));
                
                aKey.iScanCode = EStdKeyNo;
                aKey.iCount = 1;
                TTranslatedKey& key = aKey.iEvent[0];
                key.iIsRepeatingKey = aIsKeyDown;
                key.iScanCode = EStdKeyNo;
                key.iUnicode = EKeyNo;
                
                iNavKeyDown = (iNavKeyDown | EEnd);
                }

            //ALT = Stop
            else if (aHIDModifiers & (KHidModifierAlt | KHidModifierAltRight) ||
                iMmKeyDown & EPlay)
                {
                TRACE_INFO((_L("[HID]\tALT + DOWN >>> PLAY / PAUSE")));
                
                scancode = EStdKeyApplication2;
                isMmKey = ETrue;
                bitmapToReset = EPlay;
                }

            //SHIFT = Volume down
            else if (aHIDModifiers & (KHidModifierShift | KHidModifierShiftRight) ||
                iMmKeyDown & EVolDown)
                {
                TRACE_INFO((_L("[HID]\tSHIFT + DOWN >>> VOLUME DOWN")));
                
                scancode = EStdKeyDecVolume;
                isMmKey = ETrue;
                bitmapToReset = EVolDown;
                }
            break;
            }
        default:
            {
            // no actions
            break;
            }
        }

    if (isMmKey)
        {
        if (bitmapToReset)
            {
            ResetBitmap(aIsKeyDown, bitmapToReset);
            }
        SendRawEvent(scancode, aIsKeyDown, modifier);
        ret = ETrue;
        }
    
    return ret;
    }

// ----------------------------------------------------------------------
// CHidKeyboardDriver::HandleKeyMappingLeft
// Handle key mapping gor LEFT + MODIFIER
// ----------------------------------------------------------------------
//
TBool CHidKeyboardDriver::HandleKeyMappingLeft(TDecodedKeyInfo& aKey,
    TBool aIsKeyDown,
    TUint8 aHIDModifiers)
    {
    TRACE_INFO((_L("[HID]\tCHidKeyboardDriver::HandleKeyMappingLeft")));

    TBool ret = EFalse;
    TInt scancode = 0;
    TUint modifier = 0;
    TBool isMmKey = EFalse;
    TMmKeyDown bitmapToReset = ENone;
    
    switch (aKey.iScanCode)
        {
        case EStdKeyLeftArrow:
            {
            // Key up and LSK was simulated
            if (!aIsKeyDown &&
                iNavKeyDown & ELsk)
                {
                TRACE_INFO((_L("[HID]\tCTRL + LEFT >>> LEFT SOFTKEY UP")));
                
                aKey.iScanCode = EStdKeyDevice0;
                TTranslatedKey& key = aKey.iEvent[0];
                key.iIsRepeatingKey = aIsKeyDown;
                key.iScanCode = EStdKeyDevice0;
                key.iUnicode = EKeyDevice0;
                
                iNavKeyDown = (iNavKeyDown & !ELsk); 
                }
            
            //CTRL = LSK
            else if (aIsKeyDown &&
                aHIDModifiers & (KHidModifierCtrl | KHidModifierCtrlRight))
                {
                TRACE_INFO((_L("[HID]\tCTRL + LEFT >>> LEFT SOFTKEY DOWN")));
                
                aKey.iScanCode = EStdKeyDevice0;
                aKey.iCount = 1;
                TTranslatedKey& key = aKey.iEvent[0];
                key.iIsRepeatingKey = aIsKeyDown;
                key.iScanCode = EStdKeyDevice0;
                key.iUnicode = EKeyDevice0;
                
                iNavKeyDown = (iNavKeyDown | ELsk);
                }

            //ALT (short) = Previous
            //ALT (long) = Backwards
            else if (aHIDModifiers & (KHidModifierAlt | KHidModifierAltRight) ||
                iMmKeyDown & EPrev)
                {
                TRACE_INFO((_L("[HID]\tALT + LEFT >>> PREVIOUS / REW")));
                
                scancode = EStdKeyApplication5;
                isMmKey = ETrue;
                bitmapToReset = ENext;
                }

            //SHIFT = Mute toggle
            else if (aHIDModifiers & (KHidModifierShift | KHidModifierShiftRight) ||
                iMmKeyDown & EPlay)
                {
                TRACE_INFO((_L("[HID]\tSHIFT + LEFT >>> MUTE / UNMUTE")));
                
                scancode = EStdKeyApplication2;
                isMmKey = ETrue;
                bitmapToReset = EPlay;
                }
            break;
            }
        default:
            {
            // no actions
            break;
            }
        }

    if (isMmKey)
        {
        if (bitmapToReset)
            {
            ResetBitmap(aIsKeyDown, bitmapToReset);
            }
        SendRawEvent(scancode, aIsKeyDown, modifier);
        ret = ETrue;
        }
    
    return ret;
    }

// ----------------------------------------------------------------------
// CHidKeyboardDriver::HandleKeyMappingRight
// Handle key mapping gor RIGHT + MODIFIER
// ----------------------------------------------------------------------
//
TBool CHidKeyboardDriver::HandleKeyMappingRight(TDecodedKeyInfo& aKey,
    TBool aIsKeyDown,
    TUint8 aHIDModifiers)
    {
    TRACE_INFO((_L("[HID]\tCHidKeyboardDriver::HandleKeyMappingRight")));

    TBool ret = EFalse;
    TInt scancode = 0;
    TUint modifier = 0;
    TBool isMmKey = EFalse;
    TMmKeyDown bitmapToReset = ENone;
    
    switch (aKey.iScanCode)
        {
        case EStdKeyRightArrow:
            {
            // Key up and RSK was simulated
            if (!aIsKeyDown &&
                iNavKeyDown & ERsk)
                {
                TRACE_INFO((_L("[HID]\tCTRL + LEFT >>> RIGHT SOFTKEY UP")));
                
                aKey.iScanCode = EStdKeyDevice1;
                TTranslatedKey& key = aKey.iEvent[0];
                key.iIsRepeatingKey = aIsKeyDown;
                key.iScanCode = EStdKeyDevice1;
                key.iUnicode = EKeyDevice1;
                
                iNavKeyDown = (iNavKeyDown & !ERsk); 
                }
            
            //CTRL = RSK
            else if (aIsKeyDown &&
                aHIDModifiers & (KHidModifierCtrl | KHidModifierCtrlRight))
                {
                TRACE_INFO((_L("[HID]\tCTRL + RIGHT >>> RIGHT SOFTKEY DOWN")));
                
                aKey.iScanCode = EStdKeyDevice1;
                aKey.iCount = 1;
                TTranslatedKey& key = aKey.iEvent[0];
                key.iIsRepeatingKey = aIsKeyDown;
                key.iScanCode = EStdKeyDevice1;
                key.iUnicode = EKeyDevice1;
                
                iNavKeyDown = (iNavKeyDown | ERsk);
                }

            //ALT (short) = Next
            //ALT (long)  = Forward
            else if (aHIDModifiers & (KHidModifierAlt | KHidModifierAltRight) ||
                iMmKeyDown & EPrev)
                {
                TRACE_INFO((_L("[HID]\tALT + RIGHT >>> NEXT / FF")));
                
                scancode = EStdKeyApplication4;
                isMmKey = ETrue;
                bitmapToReset = EPrev;
                }

            //SHIFT = Mute toggle (currently just toggle play / pause)
            else if (aHIDModifiers & (KHidModifierShift | KHidModifierShiftRight) ||
                iMmKeyDown & EPlay)
                {
                TRACE_INFO((_L("[HID]\tSHIFT + RIGHT >>> MUTE / UNMUTE")));
                
                scancode = EStdKeyApplication2;
                isMmKey = ETrue;
                bitmapToReset = EPlay;
                }
            break;
            }
        default:
            {
            // no actions
            break;
            }
        }

    if (isMmKey)
        {
        if (bitmapToReset)
            {
            ResetBitmap(aIsKeyDown, bitmapToReset);
            }
        SendRawEvent(scancode, aIsKeyDown, modifier);
        ret = ETrue;
        }
    
    return ret;
    }

// ----------------------------------------------------------------------
// CHidKeyboardDriver::HandleKeyMappingOther
// Handle key mapping gor OTHER + MODIFIER
// ----------------------------------------------------------------------
//
TBool CHidKeyboardDriver::HandleKeyMappingOther(TDecodedKeyInfo& aKey,
    TBool aIsKeyDown,
    TUint8 /*aHIDModifiers*/)
    {
    TRACE_INFO((_L("[HID]\tCHidKeyboardDriver::HandleKeyMappingOther")));

    TBool ret = EFalse;
    TInt scancode = 0;
    TUint modifier = 0;
    TBool isMmKey = EFalse;
    
    switch (aKey.iScanCode)
        {
        case EStdKeyEscape:
            {
            // ESC is released and keys were locked, eat the key press
            if (!aIsKeyDown &&
                iNavKeyDown & EEsc)
                {
                TRACE_INFO((_L("[HID]\tESC >>> DISBALE KEY LOCK UP")));
                
                ret = ETrue;
                iNavKeyDown = (iNavKeyDown & !EEsc);
                }
            
            // ESC when keylock enabled >>> Disabled key lock
/*            else if (aIsKeyDown &&
                iKeyLock.IsKeyLockEnabled())
                {
                TRACE_INFO((_L("[HID]\tESC >>> DISBALE KEY LOCK DOWN")));
                
                iKeyLock.DisableKeyLock();

                ret = ETrue;
                iNavKeyDown = (iNavKeyDown | EEsc);
                }
*/
            break;
            }
        case EStdKeyF8:
            // fall through
        case EStdKeyApplication2:
            {
            TRACE_INFO((_L("[HID]\tF8 / APPLICATION2 >>> PLAY / PAUSE")));
            
            // PLAY / PAUSE
            if (aKey.iScanCode != EStdKeyApplication2)
                {
                scancode = EStdKeyApplication2;
                isMmKey = ETrue;
                }
            break;
            }
        case EStdKeyApplication3:
            {
            TRACE_INFO((_L("[HID]\tAPPLICATION3 >>> STOP")));
            
            // STOP
            scancode = EStdKeyApplication3;
            isMmKey = ETrue;
            break;
            }
        case EStdKeyF9:
            // break;
        case EStdKeyApplication4:
            {
            TRACE_INFO((_L("[HID]\tF9 / APPLICATION4 >>> NEXT / FF")));
            
            // NEXT
            scancode = EStdKeyApplication4;
            isMmKey = ETrue;
            break;
            }
        case EStdKeyF7:
            // fall through
        case EStdKeyApplication5:
            {
            TRACE_INFO((_L("[HID]\tF7 / APPLICATION5 >>> PREVIOUS / REW")));
            
            // PREVIOUS
            scancode = EStdKeyApplication5;
            isMmKey = ETrue;
            break;
            }
        case EStdKeyF12:
            // fall through
        case EStdKeyIncVolume:
            {
            TRACE_INFO((_L("[HID]\tF12 / INCVOLUME >>> VOLUME UP")));
            
            // VOLUME UP
            scancode = EStdKeyIncVolume;
            isMmKey = ETrue;
            break;
            }
        case EStdKeyF11:
            // fall through
        case EStdKeyDecVolume:
            {
            TRACE_INFO((_L("[HID]\tF11 / DECVOLUME >>> VOLUME DOWN")));
            
            // VOLUME DOWN
            scancode = EStdKeyDecVolume;
            isMmKey = ETrue;
            break;
            }
        case EStdKeyF10:
            {
            TRACE_INFO((_L("[HID]\tF10 >>> MUTE")));
            
            // MUTE (currently just toggle play / pause)
            scancode = EStdKeyApplication2;
            isMmKey = ETrue;
            break;
            }
        default:
            {
            break;
            }
        }

    if (isMmKey)
        {
        SendRawEvent(scancode, aIsKeyDown, modifier);
        ret = ETrue;
        }
    
    return ret;
    }

void CHidKeyboardDriver::ResetBitmap(TBool aIsKeyDown,
        TMmKeyDown aBitmapToReset)
    {
    if (aIsKeyDown)
        {
        iMmKeyDown = (iMmKeyDown | aBitmapToReset);
        }
    else
        {
        iMmKeyDown = (iMmKeyDown & !aBitmapToReset);
        }
    }

// ----------------------------------------------------------------------
// CHidKeyboardDriver::SendRawEvent
// Send raw key event to window server
// ----------------------------------------------------------------------
TInt CHidKeyboardDriver::SendRawEvent(TInt aScancode, TBool aIsKeyDown,
        TUint aModifier)
    {
    TRawEvent rawEvent;
    TInt err = KErrNone;

        TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::SendRawEvent: scancode %d, modifier: %d, keydown: %d"), aScancode, aModifier, aIsKeyDown ));
    if (aScancode != 0)
        {
        if (aIsKeyDown)
            rawEvent.Set(TRawEvent::EKeyDown, aScancode);
        else
            rawEvent.Set(TRawEvent::EKeyUp, aScancode); //This will never be called for +,* and #

        TEventModifier eventmodifier = (TEventModifier) -1; //all bits set to 1.
        TModifierState modifierstate = (TModifierState) aModifier;
            TRACE_INFO( (_L("[HID]\tCHidKeyboardDriver::SendRawEvents: modifiers %d, modifierState: %d"), aModifier, (TUint)modifierstate));

        err = iWsSession.SetModifierState(eventmodifier, modifierstate);
        iWsSession.SimulateRawEvent(rawEvent);
        iWsSession.Flush();
        }
    return err;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::SupportedFieldCount
//----------------------------------------------------------------------------
//
TInt CHidKeyboardDriver::SupportedFieldCount()
    {
    return iSupportedFieldCount;
    }

//----------------------------------------------------------------------------
// CHidKeyboardDriver::SetInputHandlingReg
//----------------------------------------------------------------------------
//
void CHidKeyboardDriver::SetInputHandlingReg(
        CHidInputDataHandlingReg* aHandlingReg)
    {
    iInputHandlingReg = aHandlingReg;
    }

// ----------------------------------------------------------------------
// CHidKeyboardDriver::TimerExpired
// Callback function from CTimeOutTimer
// ----------------------------------------------------------------------
void CHidKeyboardDriver::TimerExpired()
    {
    DBG(RDebug::Print(_L("[HID]\tCHidKeyboardDriver::TimerExpired, cancel all key presses")));
    CancelAllKeys();
    }

// ----------------------------------------------------------------------
// End of file
