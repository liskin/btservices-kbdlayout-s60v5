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


#ifndef __BTCONNECTIONOBSERVER_H__
#define __BTCONNECTIONOBSERVER_H__

/*!
 This class specifies the connection observer interface
 Used in conjunction with CBTHidConnection class
 */

class MBTConnectionObserver
    {
public:

    /*!
     Reports control data received from the HID device
     @param aConnID connection id
     @param aBuffer the data buffer
     */
    virtual void HandleControlData(TInt aConnID, const TDesC8& aBuffer) = 0;

    /*!
     Reports completion of a Set/Get command to the HID device for  commands that
     do not have a data response
     @param aConnID connection id
     @param aStatus command ack status code.
     */
    virtual void HandleCommandAck(TInt aConnID, TInt aStatus) = 0;

    /*!
     Reports interrupt data received from the HID device
     @param aConnID connection id
     @param aBuffer the data buffer
     */
    virtual void HandleInterruptData(TInt aConnID, const TDesC8& aBuffer) = 0;

    /*!
     Report a change in the connection status
     @param aConnID connection id
     @param aStatus KErrNone, or system error code.
     */
    virtual void FirstTimeConnectionComplete(TInt aConnID, TInt aStatus) = 0;

    /*!
     Reports that a connection has entered the link loss state
     @param aConnID connection id
     */
    virtual void LinkLost(TInt aConnID) = 0;

    /*!
     Reports that a connection has been re-established
     @param aConnID connection id
     */
    virtual void LinkRestored(TInt aConnID) = 0;

    /*!
     Reports that the Bluetooth Link has been intentionally disconnected
     @param aConnID connection id
     */
    virtual void Disconnected(TInt aConnID) = 0;

    /*!
     Reports that a connection has been unplugged
     @param aConnID connection id
     */
    virtual void Unplugged(TInt aConnID) = 0;
    };

#endif // __BTCONNECTIONOBSERVER_H__
