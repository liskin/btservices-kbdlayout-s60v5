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


#ifndef __MOUSE_H
#define __MOUSE_H

#include <e32std.h>
#include <w32std.h>

#include <e32msgqueue.h>
#include <e32cmn.h>
#include "pointmsgqueue.h"

#include "hidinterfaces.h"

class CField;
class CHidMouseDriver;

/*!
 HID mouse driver class
 */
class CHidMouseDriver : public CHidDriver //public CHidDriverPluginInterface
    {

private:
    /*! Initialisation states */
    enum TMouseDriverState
        {
        EUninitialised, /*!< Driver has not been initialised */
        EInitialised, /*!< Driver is initialised */
        EDisabled
        /*!< The driver will not respond to interrupt data */
        };

private:
    TMouseDriverState iDriverState;
    TInt iConnectionId;
    TInt iAppMenuId;

public:
    /*!
     Creates an instantiated CHidMouseDriver object.
     @param aHid The generic HID layer that requested the driver
     @param aFactory aFactory The factory that created the driver
     @result A pointer to the instantiated keyboard driver
     */
    static CHidMouseDriver* NewL(MDriverAccess* aHid);
    /*!
     Creates an instantiated CHidMouseDriver object and leaves it on the
     cleanup stack.
     @param aHid The generic HID layer that requested the driver
     @param aFactory aFactory The factory that created the driver
     @result A pointer to the instantiated keyboard driver
     */
    static CHidMouseDriver* NewLC(MDriverAccess* aHid);

    /*!
     Stops driver activity, deletes the key repeat and decode objects and closes
     the window server session before the driver is deleted
     */
    virtual ~CHidMouseDriver();

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

    virtual TInt SupportedFieldCount();

    /**
     * Set input handling registy 
     *
     * @since S60 v5.0     
     * @param aHandlingReg  a Input handling registry
     */
    virtual void SetInputHandlingReg(CHidInputDataHandlingReg* aHandlingReg);

    //redraw cursor
    void CursorRedraw();

private:
    // Constructor taking a pointer to the HID layer requesting the driver
    // instance
    CHidMouseDriver(MDriverAccess* aHid);

    void ConstructL();

private:
    // The types of keyboard input report fields that we handle:
    enum TKeyFieldType
        {
        EButtons = 0,
        EXY = 1,
        EWheel = 2,
        EMediaKeys = 3,
        EPowerKeys = 4,
        KNumInputFieldTypes
        };

private:
    // Called from within DataIn to handle interrupt and control channel data
    void InterruptData(const TDesC8& aPayload);

    // Handles the states of the XY (mouse up, down, left, right)
    void UpdateXY(TInt aFieldIndex, const TDesC8& aReport);

    // Handles the states of the Buttons (left & right button)
    void UpdateButtons(TInt aFieldIndex, const TDesC8& aReport);

    // Handles the states of the wheel
    void UpdateWheel(TInt aFieldIndex, const TDesC8& aReport);

    void MoveCursor(const TPoint& aPoint);
    TInt PostPointer(const TPoint& aPoint);
    TInt SendButtonEvent(TBool aButtonDown);

    void LaunchApplicationL(const TDesC& aName);
    //void LaunchApplicationL(TInt aAppUid);

private:
    // The Generic HID layer
    MDriverAccess *iGenericHid;

    // Pointers to the fields in the report descriptor containing the
    // various types of key:
    const CField* iField[KNumInputFieldTypes];

    //Supported field types count
    TInt iSupportedFieldCount;

    CHidInputDataHandlingReg* iInputHandlingReg;
    RMsgQueue<TPointBuffer> iPointBufQueue;
    TPointBuffer iPointerBuffer;
    TBool iButtonDown;
    TBool iButton2Down;

    // A window server session, so keypresses can be sent to applications:
    RWsSession iWsSession;
    };

// ----------------------------------------------------------------------

#endif
