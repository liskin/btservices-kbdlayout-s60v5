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


#ifndef __SOCKETOBSERVER_H__
#define __SOCKETOBSERVER_H__

/*!
 This class specifies the socket observer interface
 Used in conjunction with CSocketReader & CSocketWriter classes
 */
class MSocketObserver
    {
public:
    /*!
     Report a communication error.
     @param aSocketID socket being observed.
     @param aConnectionLost True if the error is a failure to read.
     @param aErrorCode associated error code.
     */
    virtual void HandleSocketError(TUint aSocketID, TBool aConnectionLost,
            TInt aErrorCode) = 0;

    /*!
     Data has been received on the socket and read into a buffer.
     @param aSocketID socket being observed.
     @param aBuffer the data buffer.
     @result ETrue to continue reading.
     @result EFalse to stop reading.
     */
    virtual TBool
            HandleDataReceived(TUint aSocketID, const TDesC8& aBuffer) = 0;

    /*!
     Data has been sent on the socket.
     @param aSocketID socket being observed
     */
    virtual void HandleWriteComplete(TUint aSocketID) = 0;

    };

#endif // __SOCKETOBSERVER_H__
