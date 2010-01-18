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


#ifndef __SOCKETSWRITER_H__
#define __SOCKETSWRITER_H__

#include <es_sock.h>
#include "timeoutnotifier.h"

class CTimeOutTimer;
class MSocketObserver;

/*!
 This class handles writing data to a socket.
 */
class CSocketWriter : public CActive, public MTimeOutNotifier
    {
public:
    /*!
     Create a CSocketWriter object
     @param aSocketID ID given to the socket. Used when reporting back.
     @param aObserver an observer for status reporting
     @result A pointer to the created instance of CSocketWriter
     */
    static CSocketWriter* NewL(TUint aSocketID, MSocketObserver& aObserver);

    /*!
     Create a CSocketWriter object
     @param aSocketID ID given to the socket. Used when reporting back.
     @param aObserver an observer for status reporting
     @result A pointer to the created instance of CSocketWriter
     */
    static CSocketWriter* NewLC(TUint aSocketID, MSocketObserver& aObserver);

    /*!
     Destroy the object and release all memory objects
     */
    ~CSocketWriter();

    /*!
     Write the data to the socket
     @param aData the data to be written
     */
    void IssueWriteL(const TDesC8& aData);

    /*!
     Initialise the writer with a socket
     @param aSocket socket to write to.
     */
    void Initialise(RSocket* aSocket);

public:
    // From MTimeOutNotifier

    void TimerExpired();

protected:
    // from CActive
    /*!
     Cancel any outstanding operation
     */
    void DoCancel();

    /*!
     Called when operation complete
     */
    void RunL();

private:
    /*!
     Perform the first phase of two phase construction 
     @param aSocketID ID given to the socket. Used when reporting back.
     @param aObserver an observer for status reporting
     */
    CSocketWriter(TUint aSocketID, MSocketObserver& aObserver);

    /*!
     Perform the second phase construction of a CSocketWriter 
     */
    void ConstructL();

    /**
     *  Write the data from the current buffer to the socket.
     */
    void DoWrite();
    /**
     *  Store data to be written in the specified buffer.
     *  @param aData Data to be written.
     *  @param aBuffer Which buffer to use.
     */
    void StoreDataL(const TDesC8& aData, TInt aBuffer);

private:
    // Member variables

    /*! The maximum time allowed for a write to complete */
    static const TInt KTimeOut;

    /*! The ID of the RSocket we are working on */
    TUint iSocketID;

    /*! An observer for status reporting */
    MSocketObserver& iObserver;

    /*! The socket to write to */
    RSocket* iSocket;

    /*! Timer used to cancel a write after a predefined timeout */
    CTimeOutTimer* iTimer;

    /** Buffers to hold the data being written */
    HBufC8* iWriteBuffer[2];

    /** The buffer whose contents are currently being written */
    TInt iCurrentBuffer;
    };

#endif // __SOCKETSWRITER_H__
