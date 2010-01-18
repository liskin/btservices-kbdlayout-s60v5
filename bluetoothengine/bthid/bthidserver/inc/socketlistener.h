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
* Description:  
 *
*/


#ifndef __SOCKETLISTENER_H__
#define __SOCKETLISTENER_H__

#include <e32base.h>
#include <es_sock.h>

#include <btmanclient.h>

class MListenerObserver;

/*!
 This class listens for incoming connection attempts on a BT L2CAP channel
 */

class CSocketListener : public CActive
    {
public:

    /*!
     Create a CSocketListener object
     @param aSocketServ the socket server session
     @param aPort port to listen on
     @param aObserver an observer of this object
     @result A pointer to the created instance of CSocketListener
     */
    static CSocketListener* NewL(RSocketServ& aSocketServ, TUint aPort,
            MListenerObserver& aObserver, TBool aAuthorisation);

    /*!
     Create a CSocketListener object
     @param aSocketServ the socket server session
     @param aPort port to listen on
     @param aObserver an observer of this object
     @result A pointer to the created instance of CSocketListener
     */
    static CSocketListener* NewLC(RSocketServ& aSocketServ, TUint aPort,
            MListenerObserver& aObserver, TBool aAuthorisation);

    /*!
     Destroy the object and release all memory objects
     */
    ~CSocketListener();

    /*!
     Accept the next connection into a blank socket
     @param aSocket blank socket to accept into
     @return KErrNone if successful, or one of the system-wide error codes.
     */
    TInt AcceptConnection(RSocket& aSocket);

protected:
    // from CActive

    /*!
     Cancel any outstanding requests
     */
    void DoCancel();

    /*!
     Respond to an event
     */
    void RunL();

private:

    /*!
     Perform the first phase of two phase construction 
     @param aSocketServ the socket server session
     @param aPort port to listen on
     @param aObserver an observer of this object
     */
    CSocketListener(RSocketServ& aSocketServ, TUint aPort,
            MListenerObserver& aObserver, TBool aAuthorisation);

    /*!
     Perform the second phase of two phase construction 
     */
    void ConstructL();

private:
    // Member variables

    /*! The socket server session */
    RSocketServ& iSocketServer;

    /*! Socket port to listen on */
    TUint iPort;

    /*! An observer for status reporting */
    MListenerObserver& iObserver;

    /*! Listening Socket */
    RSocket iListeningSocket;

    /*! The socket to accept into. Not owned */
    RSocket* iAcceptSocket;

    /*! Whether to use authorisation in incoming connections when constructed */
    TBool iAuthorisationAskedFlag;

    };

#endif // __SOCKETLISTENER_H__
