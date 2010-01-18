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


#ifndef __SESSION_H
#define __SESSION_H

#include <e32std.h>
#include <e32base.h>

// ----------------------------------------------------------------------

class CKeyboardDecoder;
class CLayoutLibrary;
class CLayoutServer;

class CLayoutSession : public CSession2

    {
public:
    static CLayoutSession* NewL();
    static CLayoutSession* NewLC();

    virtual ~CLayoutSession();

    // from CSession:
    virtual void ServiceL(const RMESSAGE& aMessage);

    void SetLayout(CLayoutLibrary* aLayoutLibrary);

private:
    CLayoutSession();
    void ConstructL();

    void DispatchMessageL(const RMESSAGE& aMessage);

    // hides base class version (not virtual):
    CLayoutServer* Server();

private:
    CKeyboardDecoder* iDecoder; // ownership
    CLayoutLibrary* iLayoutLibrary; // no ownership
    };

// ----------------------------------------------------------------------

#endif

