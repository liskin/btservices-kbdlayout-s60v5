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


#ifndef __SHUTDOWN_H
#define __SHUTDOWN_H

#include <e32std.h>

// ----------------------------------------------------------------------

class CLayoutMgrShutdown : public CActive
    {
public:
    static CLayoutMgrShutdown* NewL();
    virtual ~CLayoutMgrShutdown();

    void Start();

private:
    CLayoutMgrShutdown();
    void ConstructL();

protected:
    // from CActive:
    virtual void DoCancel();
    virtual void RunL();

private:
    static const TInt KShutdownInterval;

    RTimer iTimer;
    };

// ----------------------------------------------------------------------

#endif

