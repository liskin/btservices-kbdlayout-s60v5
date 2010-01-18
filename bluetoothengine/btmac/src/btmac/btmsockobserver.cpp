/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  socket event observer default implementation
*
*/


// INCLUDE FILES
#include "btmsockobserver.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

void MBtmSockObserver::RfcommConnectCompletedL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::RfcommAcceptCompletedL(TInt /*aErr*/, TUint /*aService*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::RfcommShutdownCompletedL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::RfcommSendCompletedL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::RfcommReceiveCompletedL(TInt /*aErr*/, const TDesC8& /*aData*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::RfcommIoctlCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::RfcommLinkInActiveModeL()
    {
    TRACE_FUNC
    }

void MBtmSockObserver::RfcommLinkInSniffModeL()
    {
    TRACE_FUNC
    }

void MBtmSockObserver::SyncLinkSetupCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::SyncLinkDisconnectCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::SyncLinkAcceptCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void MBtmSockObserver::SyncLinkSendCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }
    
void MBtmSockObserver::SyncLinkReceiveCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

