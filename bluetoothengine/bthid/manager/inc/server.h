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

#ifndef __SERVER_H
#define __SERVER_H
#include <e32std.h>
#include <e32base.h>
#include "genericserver.h"
#define RMESSAGE RMessage2
// ----------------------------------------------------------------------
// Panics:
// Category:
_LIT(KLayoutServerPanic, "LayoutServer");

// Codes:
enum TLayoutServPanic
    {
    EBadRequest = 1, EBadDescriptor = 2, ENegativeSessionCount = 3
    };

// ----------------------------------------------------------------------

class CLayoutMgrShutdown;
class CLayoutLibrary;

class CLayoutServer : public CGenericServer
    {
public:
    static CLayoutServer* NewL();
    static CLayoutServer* NewLC();
    virtual ~CLayoutServer();

    static TInt ThreadFunction(TAny* aThreadArg);

    void SessionAdded();
    void SessionClosed();

    TInt Layout() const;
    TInt SetLayout(TInt aLayoutId);

    TInt InitialLayout() const;
    void SetInitialLayoutL(TInt aCountry, TInt aVendor, TInt aProduct);

    TBool FoundLayout() const;
    TBool IsNokiaSu8() const;
    static void PanicClient(const RMessage2& aMessage, TInt aReason);
    static void PanicServer(TInt aPanic);

private:
    CLayoutServer();
    void ConstructL();

    static void ThreadFunctionL(TStartupRequest* startup);

    // from CServer:
    virtual CSession2* NewSessionL(const TVersion& aVersion,
            const RMessage2& aMessage) const;

protected:
    // from CActive:
    virtual TInt RunError(TInt aErr);

private:
    TInt iSessionCount;

    CLayoutLibrary* iLayoutLibrary;
    CLayoutMgrShutdown* iShutdown;

    TInt iInitialLayout;

    TBool iIsNokiaSu8;
    TBool iFoundLayout;
    };

// ----------------------------------------------------------------------

#endif
