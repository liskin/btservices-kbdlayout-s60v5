/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*                Server class is responsible for creating sessions and then handle
*                messages from the session class.
*
*/


// INCLUDE FILES
#include <e32property.h>
#include <btengprivatepskeys.h>

#include "BTAccServer.h"
#include "BTAccClientSrv.h"      // server name, panic code
#include "BTAccSecurityPolicy.h" 
#include "BTAccSession.h"          // create server's session
#include "debug.h"
#include "btmanclient.h"         // TBTDevAddrPckgBug
#include "basrvaccman.h"
#include "BTAccInfo.h"

/**  PubSub key read and write policies */
_LIT_SECURITY_POLICY_C1( KBTEngPSKeyReadPolicy, 
                         ECapabilityLocalServices);
_LIT_SECURITY_POLICY_C1( KBTEngPSKeyWritePolicy, 
                         ECapabilityLocalServices);

//  CONSTANTS
const TInt KShutdownDelay = 5000000;

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTAccServer::CBTAccServer
// C++ default constructor can NOT contain any code that
// might leave.
// -----------------------------------------------------------------------------
//
CBTAccServer::CBTAccServer(TInt aPriority) : CPolicyServer(aPriority, KBTAccSrvPolicy)
    {
    }

// Destructor
CBTAccServer::~CBTAccServer()
    {
    delete iAccMan;  
    delete iTimer;
    iSessions.Close();
    
    RProperty::Delete( KPSUidBluetoothEnginePrivateCategory, KBTATCodec );
    
    TRACE_FUNC
    }

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CBTAccServer* CBTAccServer::NewLC()
    {
    CBTAccServer* self=new(ELeave) CBTAccServer(EPriorityStandard);
    CleanupStack::PushL(self);
    self->ConstructL();
    self->StartL(KBTAudioManName);
    RThread().SetPriority(EPriorityRealTime); 
    return self;
    }

void CBTAccServer::StartShutdownTimerIfNoSessions()
	{
	if (iSessions.Count() == 0 && (!iTimer || !iTimer->IsActive()))
		{
		if (!iTimer)
		    {
		    TRAP_IGNORE(iTimer = CPeriodic::NewL(CActive::EPriorityStandard));
		    }
		
		if (iTimer)
		    {
		    iTimer->Start(KShutdownDelay, 0, TCallBack(CBTAccServer::TimerFired, this));
		    }
		
	    TRACE_FUNC	
		}
	}

void CBTAccServer::ClientOpened(CBTAccSession& aSession)
	{
	TRACE_FUNC
	
	//cancel the timer to prevent the server from shutting down
    CancelShutdownTimer();
	
	//add the session to the array of sessions
	(void)iSessions.Append(&aSession);
	}

void CBTAccServer::ClientClosed(CBTAccSession& aSession)
	{
	TRACE_FUNC_ENTRY
	
	//find and remove the given session from the array
	for (TUint i = 0; i < iSessions.Count(); ++i)
		{
		if (iSessions[i] == &aSession)
			{
			iSessions.Remove(i);
			break;
			}
		}
	
	StartShutdownTimerIfNoSessions();
	TRACE_FUNC_EXIT
	}

// ---------------------------------------------------------
// NewSessionL
// Create session(s) to client(s)
// ---------------------------------------------------------
//
CSession2* CBTAccServer::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
    {
    // check we're the right version
    TVersion srvVersion(KBTAccServerMajorVersionNumber,
                        KBTAccServerMinorVersionNumber,
                        KBTAccServerBuildVersionNumber);

    if (!User::QueryVersionSupported(srvVersion, aVersion))
        {
        User::Leave(KErrNotSupported);
        }

    CBTAccSession* session = CBTAccSession::NewL(*iAccMan);
    return session;
    }

void CBTAccServer::ConstructL()
    {
    iAccMan = CBasrvAccMan::NewL();
    iAccMan->LoadServicesL();
    
    User::LeaveIfError(RProperty::Define(KPSUidBluetoothEnginePrivateCategory,
                                         KBTATCodec, RProperty::EByteArray,
                                         KBTEngPSKeyReadPolicy,
                                         KBTEngPSKeyWritePolicy));
    }

void CBTAccServer::CancelShutdownTimer()
    {
    delete iTimer;
    iTimer = NULL;
    }

TInt CBTAccServer::TimerFired(TAny* /*aThis*/)
	{
	CActiveScheduler::Stop();
	return KErrNone;
	}

// ---------------------------------------------------------
// PanicClient
// RMessage::Panic() also completes the message.
// ---------------------------------------------------------
void PanicClient(const RMessage2& aMessage,TInt aPanic)
    {
    TRACE_ERROR((_L("[BTAccServer]\t PanicClient: Reason: %d"), aPanic))
    aMessage.Panic(KBTAudioManPanic,aPanic);
    }

// ---------------------------------------------------------
// PanicServer
// Panic our own thread
// ---------------------------------------------------------
void PanicServer(TInt aPanic)
    {
    TRACE_ERROR((_L("[BTAccServer]\t PanicServer: Reason: %d"), aPanic))
    User::Panic(KBTAudioManPanic, aPanic);
    }

// End of file
