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


#ifndef __LISTENEROBSERVER_H__
#define __LISTENEROBSERVER_H__

/*!
 This class specifies the listener observer interface
 Used in conjunction with CSocketListener class
 */

class MListenerObserver
    {
public:

    /*!
     Informs the observer that a connection has been accepted by a listener into
     the blank socket provided.
     @param aPort BT L2CAP channel the listener is working on
     @param aErrorCode KErrNone, or system error code
     */
    virtual void SocketAccepted(TUint aPort, TInt aErrorCode) = 0;

    };

#endif // __LISTENEROBSERVER_H__
