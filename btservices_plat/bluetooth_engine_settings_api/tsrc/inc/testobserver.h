/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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



#ifndef TESTOBSERVER_H
#define TESTOBSERVER_H

#include <e32base.h>

#define TLFUNCLOG (TUint8*) __FUNCTION__

class CBtTestLogger;

/**
*  Testing supervisor
*/
class MBTTestObserver
    {
    public:

    virtual void TestCompleted( TInt aErr, const TUint8* aFunc, const TDesC& aArg ) = 0;

    virtual CBtTestLogger& Logger() = 0;

    };

#endif      // TESTOBSERVER_H

// End of File
