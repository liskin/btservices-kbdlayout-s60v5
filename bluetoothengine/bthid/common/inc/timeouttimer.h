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

#ifndef __TIMEOUTTIMER_H__
#define __TIMEOUTTIMER_H__
#include <e32base.h>
// ----------------------------------------------------------------------
// Panics:
// Category:
_LIT(KTimeOutTimerPanic, "TimeOutTimer");

// Codes:
enum TTimeOutTimerPanic
    {
    ETimerFailed = 1
    };

// ----------------------------------------------------------------------

class MTimeOutNotifier;

/*!
 This class will notify an object after a specified timeout.
 */
class CTimeOutTimer : public CTimer
    {
public:
    /*!
     Create a CTimeOutTimer object
     @param aPriority priority to use for this timer
     @param aTimeOutNotify object to notify of timeout event
     @result A pointer to the created instance of CTimeOutTimer
     */
    static CTimeOutTimer* NewL(const TInt aPriority,
            MTimeOutNotifier& aTimeOutNotify);

    /*!
     Create a CTimeOutTimer object
     @param aPriority priority to use for this timer
     @param aTimeOutNotify object to notify of timeout event
     @result A pointer to the created instance of CTimeOutTimer
     */
    static CTimeOutTimer* NewLC(const TInt aPriority,
            MTimeOutNotifier& aTimeOutNotify);

    /*!
     Destroy the object and release all memory objects
     */
    ~CTimeOutTimer();

protected:
    // From CTimer
    /*!
     Invoked when a timeout occurs
     */
    virtual void RunL();

private:
    /*!
     Perform the first phase of two phase construction 
     @param aPriority priority to use for this timer
     @param aTimeOutNotify An observer to notify
     */
    CTimeOutTimer(const TInt aPriority, MTimeOutNotifier& aTimeOutNotify);

    /*!
     Perform the second phase construction of a CTimeOutTimer 
     */
    void ConstructL();

private:
    // Member variables

    /*! The observer for this objects events */
    MTimeOutNotifier& iNotify;
    };

#endif // __TIMEOUTTIMER_H__
