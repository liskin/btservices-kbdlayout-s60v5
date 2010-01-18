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


#ifndef _TIMEOUTNOTIFIER_H_
#define _TIMEOUTNOTIFIER_H_

/*!
 This class specifies the function to be called when a timeout occurs.
 Used in conjunction with CTimeOutTimer class
 */
class MTimeOutNotifier
    {
public:
    /*!
     The function to be called when a timeout occurs.
     */
    virtual void TimerExpired() = 0;
    };

#endif
