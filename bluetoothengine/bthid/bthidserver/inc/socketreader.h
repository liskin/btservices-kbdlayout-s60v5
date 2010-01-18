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


#ifndef __SOCKETSREADER_H__
#define __SOCKETSREADER_H__

#include <es_sock.h>

class MSocketObserver;

/*!
 This class handles reading data from a socket.
 Any data read is simply passed back to the observer.
 */
class CSocketReader : public CActive
    {
public:
    /*!
     Create a CSocketReader object
     @param aSocketID ID given to the socket. Used when reporting back.
     @param aObserver an observer for status reporting
     @param aInitialBufSize the initial size to use for the read buffer
     @result A pointer to the created instance of CSocketReader
     */
    static CSocketReader* NewL(TUint aSocketID, MSocketObserver& aObserver,
            TInt aInitialBufSize);

    /*!
     Create a CSocketReader object
     @param aSocketID ID given to the socket. Used when reporting back.
     @param aObserver an observer for status reporting
     @param aInitialBufSize the initial size to use for the read buffer
     @result A pointer to the created instance of CSocketReader
     */
    static CSocketReader* NewLC(TUint aSocketID, MSocketObserver& aObserver,
            TInt aInitialBufSize);

    /*!
     Destroy the object and release all memory objects
     */
    ~CSocketReader();

    /*!
     Initiate reading from a socket
     @param aSocket socket to read from
     @param aMTU the bluetooth maximum transmission unit for the given socket.
     */
    void StartReadingL(RSocket* aSocket, TInt aMTU);

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
    CSocketReader(TUint aSocketID, MSocketObserver& aObserver);

    /*!
     Perform the second phase construction of a CSocketReader 
     @param aInitialBufSize the initial size to use for the read buffer
     */
    void ConstructL(TInt aInitialBufSize);

    /*!
     Initiate a read from the socket.
     */
    void IssueRead();

private:
    // Member variables
    /*! The ID of the socket being read from */
    TUint iSocketID;

    /*! An observer for reporting data to */
    MSocketObserver& iObserver;

    /*! Socket to read data from */
    RSocket* iSocket;

    /*! Buffer for receiving data */
    HBufC8* iBuffer;

    /*! Pointer to the buffer to be used when reading */
    TPtr8 iBufferPtr;
    };

#endif // __SOCKETSREADER_H__
