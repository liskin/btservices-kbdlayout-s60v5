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


#ifndef __SOCKINITNOTIFIER_H__
#define __SOCKINITNOTIFIER_H__

/*!
 This class specifies the socket initiator notifier interface
 Used in conjunction with CSocketInitiator class
 */

class MSockInitNotifier
    {
public:

    /*!
     Reports that control and interrupt sockets have been established.
     */
    virtual void SocketsConnected() = 0;

    /*!
     Reports that a failure occurred connecting the sockets.
     @param aStatus system error code.
     */
    virtual void SocketsConnFailed(TInt aStatus) = 0;
    };

#endif // __SOCKINITNOTIFIER_H__
