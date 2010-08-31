/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Session class for handling commands from clients.
*
*/

#ifndef BTNOTIFSESSION_H
#define BTNOTIFSESSION_H

#include "btnotifserver.h"

/**
 *  CBTNotifSession maintains a session with a client.
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifSession ) : public CSession2
    {

public:

    /**
     * Two-phased constructor.
     */
    static CBTNotifSession* NewL();

    /**
    * Destructor.
    */
    virtual ~CBTNotifSession();

// from base class CSession2

    /**
     * From CSession2.
     * Receives a message from a client.
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the client request.
     */
    virtual void ServiceL( const RMessage2& aMessage );

    /**
     * From CSession2.
     * Completes construction of the session.
     *
     * @since Symbian^4
     */
    virtual void CreateL();

private:

    CBTNotifSession();

    void ConstructL();

    /**
     * Returns a handle to our server.
     *
     * @since Symbian^4
     * @param Pointer to our server.
     */
    inline CBTNotifServer* Server() const
        { return (CBTNotifServer*) CSession2::Server(); }

private: // data

    BTUNITTESTHOOK

    };

#endif // BTNOTIFSESSION_H
