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


#ifndef __GENERICCLIENT_H__
#define __GENERICCLIENT_H__

#ifndef DBG

#ifdef _DEBUG
#define DBG(a) a
#include <eikappui.h>
#else
#define DBG(a)
#endif

#endif

#include <e32std.h>

// ----------------------------------------------------------------------

/*
 *  Base class for client-side sessions for all servers.
 */
class RGenericSession : public RSessionBase
    {
protected:
    // Create a new session with the server, creating the server if necessary
    TInt StartSession(const TDesC& aServer, const TVersion& aVersion,
            const TDesC& aFilename, const TUidType& aUid,
            TInt aAsyncMessageSlots);

private:
    // Load and start the server
    TInt CreateServer(const TDesC& aServer, const TDesC& aFilename,
            const TUidType& aUid);
    };

#endif  // __GENERICCLIENT_H__
