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


#ifndef __GENERICSERVER_H__
#define __GENERICSERVER_H__

#include <e32std.h>
#include <e32base.h>

// ----------------------------------------------------------------------

/*
 *  Information passed to the server to allow it to signal the client.
 */
struct TStartupRequest
    {
    inline TStartupRequest();
    inline TStartupRequest(TRequestStatus* status, TThreadId id);

    /** Request status to be signalled when the server has started */
    TRequestStatus* iStatus;
    /** ID of the client's thread */
    TThreadId iId;
    };

const TInt KStartupTextLength = sizeof(TStartupRequest) / sizeof(TText);

// ----------------------------------------------------------------------

/*
 *  Base class for all servers.
 */
class CGenericServer : public CPolicyServer

    {
public:
    /**
     *  Extract the client startup information from the command line passed
     *  to the server process.
     *  @param startup Structure to receive the client information.
     *  @return TInt KErrNone if successful.
     */
    static TInt GetStartupFromCommandLine(TStartupRequest& startup);

protected:
    // Constructor
    CGenericServer(TInt aPriority);
    // Signal the client that server startup has finished
    static void SignalStartup(TStartupRequest* startup, TInt aReason);
    };

// ----------------------------------------------------------------------

inline TStartupRequest::TStartupRequest() :
    iStatus(NULL), iId(0)
    {
    }

inline TStartupRequest::TStartupRequest(TRequestStatus* status, TThreadId id) :
    iStatus(status), iId(id)
    {
    }

#endif  // __GENERICSERVER_H__
