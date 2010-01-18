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


#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <e32std.h>
#include <w32std.h>
#include <e32event.h>

#include "hidinterfaces.h"
#include "hidkeys.h"
#include "layoutmgr.h"
#include "timeoutnotifier.h"
#include <e32msgqueue.h>
#include <e32cmn.h>
#include "pointmsgqueue.h"
#include "bthidsettings.h"

class CField;
class CHidKeyboardDriver;
class CTimeOutTimer;
/*!
 HID keyboard driver class
 */
class CHidKeyboardDriver : public CHidDriver, public MTimeOutNotifier
    {

private:
    /*! Initialisation states */
    enum TKeyboardDriverState
        {
        EUninitialised, /*!< Driver has not been initialised */
        EInitialised, /*!< Driver is initialised */
        EDisabled
        /*!< The driver will not respond to interrupt data */
        };

    // Flags indicating which locking keys are active
    enum TLockVals
        {
        ENumLock = 1, ECapsLock = 2, EScrollLock = 4
        };

    // The types of keyboard input report fields that we handle:
    enum TKeyFieldType
        {
        EStandardKeys = 0,
        EModifierKeys = 1,
        EMediaKeys = 2,
        EPowerKeys = 3,
        KNumInputFieldTypes
        };

    // The types of keyboard input report fields that we handle:
    enum TMouseFieldType
        {
        EMouseButtons = 0,
        EMouseXY = 1,
        EMouseWheel = 2,
        EMouseMediaKeys = 3,
        EMousePowerKeys = 4,
        KMouseInputFieldTypes
        };
public:
    // Constructors and destructor
    /*!
     Creates an instantiated CHidKeyboardDriver object.
     @param aHid The generic HID layer that requested the driver
     @param aFactory aFactory The factory that created the driver
     @result A pointer to the instantiated keyboard driver
     */
    static CHidKeyboardDriver* NewL(MDriverAccess* aHid);
    /*!
     Creates an instantiated CHidKeyboardDriver object and leaves it on the
     cleanup stack.
     @param aHid The generic HID layer that requested the driver
     @param aFactory aFactory The factory that created the driver
     @result A pointer to the instantiated keyboard driver
     */
    static CHidKeyboardDriver* NewLC(MDriverAccess* aHid);

    /*!
     Stops driver activity, deletes the key repeat and decode objects and closes
     the window server session before the driver is deleted
     */
    virtual ~CHidKeyboardDriver();

public:
    // new functions
    /*!
     Called by the Generic HID layer to see if the driver can is able to use
     reports from a newly-connected device
     @result ETrue The driver can handle the reports
     @result EFalse The driver cannot handle the reports
     */
    virtual TInt CanHandleReportL(CReportRoot* aReportRoot);

    /*!
     Called by the Generic HID layer when a device has been removed, prior to the
     driver being removed.  This allows the driver to notify any applications of
     disconnection, if required
     @param aReason The reason for device disconnection
     */
    virtual void Disconnected(TInt aReason);

    /*!
     Called by the Generic HID layer when data has been received from the device
     handled by this driver.
     @param aChannel The channel on which the data was received (as defined by the
     transport layer
     @param aPayload A pointer to the data buffer
     */
    virtual TInt DataIn(CHidTransport::THidChannelType aChannel,
            const TDesC8& aPayload);

    /*!
     Called by the transport layers to inform the generic HID of the success of
     the last Set... command.
     @param aConnectionId ID of the device
     @param aCmdAck Status of the last Set... command
     */
    virtual void CommandResult(TInt aCmdAck);

    /*!
     Called after a driver is sucessfully created by the Generic HID, when a
     device is connected
     */
    virtual void InitialiseL(TInt aConnectionId);

    /*!
     Resets the internal state of the driver (any pressed keys are released) and
     enables the driver
     */
    virtual void StartL(TInt aConnectionId);
    /*!
     Cancels all pressed keys and disables the driver (so it will not
     process interrupt data)
     */
    virtual void Stop();

    /**
     * Return count of supported fields
     *
     * @since S60 v5.0     
     * @return Number of supported fields.
     */
    virtual TInt SupportedFieldCount();

    /**
     * Set input handling registy 
     *
     * @since S60 v5.0     
     * @param aHandlingReg  a Input handling registry
     */
    virtual void SetInputHandlingReg(CHidInputDataHandlingReg* aHandlingReg);

    // ----------------------------------------
    // Repeat key timer interface:

    /*!
     Callback function for the key repeat timer to trigger a repeat key event.
     */
    TInt OnKeyRepeat();

    static TInt TimerFiredOnKeyRepeat(TAny* aThis);

private:
    // Constructors

    // Constructor taking a pointer to the HID layer requesting the driver
    // instance
    CHidKeyboardDriver(MDriverAccess* aHid);

    void ConstructL();

private:
    // Functions from base classes

    /**
     * From MTimeoutNotifier
     */
    void TimerExpired();
private:
    // New Functions

    // Called from within DataIn to handle interrupt and control channel data
    void InterruptData(const TDesC8& aPayload);

    // Send key down / key up events to the window server:
    void KeyEvent(TBool aIsKeyDown, TInt aHidKey, TInt aUsagePage);
    TKeyEvent TKeyEventFromScanCode(TInt aScanCode) const;
    inline void KeyUp(TInt aHidKey, TInt aUsagePage);
    inline void KeyDown(TInt aHidKey, TInt aUsagePage);

    // Handles the states of the modifier keys
    void UpdateModifiers(TInt aFieldIndex, const TDesC8& aReport);
    // Handles the states of the XY (mouse up, down, left, right)
    void UpdateXY(TInt aFieldIndex, const TDesC8& aReport);

    // Handles the states of the Buttons (left & right button)
    void UpdateButtons(TInt aFieldIndex, const TDesC8& aReport);

    // Handles the states of the wheel
    void UpdateWheel(TInt aFieldIndex, const TDesC8& aReport);

    // Handle key presses
    void ProcessKeys(TInt aFieldIndex, const TDesC8& aReport);

    // Determines whether too many keys have been pressed on the device
    TBool IsRollover(TInt aFieldIndex, const TDesC8& aReport) const;

    // Get current modifier state in the format used by TKeyEvent
    TUint32 KeyEventModifiers() const;

    // Resets the internal keypress states and sends keyup events if required
    void CancelKeysForField(TInt aFieldIndex);
    void CancelAllKeys();

    // Determines whether any lock (Caps,Num,scroll) keys are pressed
    void UpdateLockState(TInt aKey);

    // Sets the LEDs on the keyboard according to the keyboard state
    // (Not implemented)
    void SetKeyboardLeds() const;

    // Send a key event to the system
    void SendKeyPress(TUint16 aUnicode, TInt aUsagePage, TInt aScanCode,
            TBool aIsRepeatingKey);

    void SendToWindowGroup(const TKeyEvent& aKeyEvent, TEventCode aType);
    void SendToWindowServer(TKeyEvent aKeyEvent);

    // Determine which window group a key event should be sent to.
    // Except for special cases this is the window group with the focus.
    TInt WindowGroupForKeyEvent(const TKeyEvent& aKeyEvent, TEventCode aType);

    // Handles the sending of raw events to the phone application instead of key events.
    void HandleTelephoneAppKeys(TInt aScanCode, TInt aUniCode,
            TBool aIsKeyDown);

    // Checks if application launching key combination and launch a corresponding application
    void HandleApplicationLaunchKeysL(TUint16 aScancodeKey, TBool aIsKeyDown,
            TUint8 aModifiers);

    /**
     * CHidKeyboardDriver::LaunchApplicationL
     * @param aAppUid Application UID
     */
    void LaunchApplicationL(TInt aAppUid);

    // Checks if multimedia-key (play,stop,..) and sends to RemCon
    TBool HandleMultimediaKeys(TUint16 aScancodeKey, TBool aIsKeyDown,
            TUint8 aModifiers);
    TBool HandleMultimediaKeysForNokia(TUint16 aScancodeKey,
            TBool aIsKeyDown, TUint8 aModifiers);
    TBool HandleMultimediaKeysForStandard(TUint16 aScancodeKey,
            TBool aIsKeyDown, TUint8 aModifiers);

    TInt AppMenuId();
    TInt PhoneAppId();
    TInt IdleAppId();
    TBool IsApplicationMenuTopMost();
    TBool IsPhoneAppTopMost();

    // bitmap for Multimedia key states
    enum TMmKeyDown
        {
        EVolUp = 1,
        EVolDown = 2,
        EPlay = 4,
        EStop = 8,
        ENext = 16,
        EPrev = 32
        };

    void ResetBitmap(TBool aIsKeyDown, TMmKeyDown aBitmapToReset);

    /**
     * Send raw key event to window server
     * @param aScancode key scancode
     * @param aModifiers modifier
     * @return Error code
     */
    TInt SendRawEvent(TInt aScancode, TBool aIsKeyDown, TUint aModifier);

    // ----------------------------------------

    static TInt ResetArrayToSize(RArray<TInt>& aArray, TInt aSize);
    void MoveCursor(const TPoint& aPoint);
    TInt PostPointer(const TPoint& aPoint);
    TInt SendButtonEvent(TBool aButtonDown);

    TBool IsDigitKey(TInt aScanCode);
    TBool IsSpecialHandleKey(TInt aUniCode);

    void LaunchApplicationL(const TDesC& aName);
private:

    TKeyboardDriverState iDriverState;

    // Application UIDs
    TInt iConnectionId;
    TInt iAppMenuId;
    TInt iPhoneAppId;
    TInt iIdleAppId;

    // The Generic HID layer
    MDriverAccess *iGenericHid;

    // Arrays of keys currently pressed for each field:
    RArray<TInt> iKeys[KNumInputFieldTypes];

    // Pointers to the fields in the report descriptor containing the
    // various types of key:
    const CField* iField[KNumInputFieldTypes];

    // Pointers to the fields in the report descriptor containing the
    // various types of key:
    const CField* iMouseField[KMouseInputFieldTypes];
    // Pointer to the field in the report descriptor containing the LEDs:
    const CField* iLedField;

    // The current modifier state
    TUint8 iModifiers;
    // The current locking key states
    TUint8 iLockState;
    //Supported field types count
    TInt iSupportedFieldCount;

    // A window server session, so keypresses can be sent to applications:
    RWsSession iWsSession;

    // Timer object so the driver can generate repeat keypresses:
    CPeriodic *iRepeatTimer;

    // The last keypress event sent to the window server (allows key repeat)
    TKeyEvent iLastKey;

    // Layout manager session:
    RLayoutManager iLayoutMgr;

    // Settings handler class
    CBtHidSettings* iSettings;

    /*! The last keyboard layout selected by the user */
    THidKeyboardLayoutId iLastSelectedLayout;

    // Flag for Multimedia key state
    TUint8 iMmKeyDown;

    CHidInputDataHandlingReg* iInputHandlingReg;

    // This timer stops key repeating after defined time to prevent endless key repeats in error cases. Fix for EMKD-7FBB9H
    CTimeOutTimer* iRepeatEndTimer;

    TBool iComboDevice;

    RMsgQueue<TPointBuffer> iPointBufQueue;
    TPointBuffer iPointerBuffer;
    TBool iButtonDown;
    TBool iButton2Down;
    };

// ----------------------------------------------------------------------
// Helpers for sending key events:

inline void CHidKeyboardDriver::KeyUp(TInt aHidKey, TInt aUsagePage)
    {
    KeyEvent(EFalse, aHidKey, aUsagePage);
    }

inline void CHidKeyboardDriver::KeyDown(TInt aHidKey, TInt aUsagePage)
    {
    KeyEvent(ETrue, aHidKey, aUsagePage);
    }

// ----------------------------------------------------------------------

#endif
