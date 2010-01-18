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


#include "pluginstub.h"

enum TTestCase
    {
    EEventAT,
    EEventExtClose,
    };
    
const TInt KFirstTimeout = 15000000;  
const TInt KRestTimeout = 2000000;  

CAtExtPluginStub* CAtExtPluginStub::NewL()
    {
    CAtExtPluginStub* self = new (ELeave) CAtExtPluginStub();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CAtExtPluginStub::~CAtExtPluginStub()
	{
	delete iActive;
	iTimer.Close();
	}

void CAtExtPluginStub::HandleCommand( const TDesC8& aCmd, RBuf8& aReply )
	{
    HandleCommandCompleted(KErrNone);
    }

void CAtExtPluginStub::RequestCompletedL(CGenericActive& aActive)
    {
    TInt err = aActive.RequestStatus().Int();
    switch (aActive.RequestId())
        {
        case EEventAT:
            {
            _LIT8(KAT, "\r\n+AT : 0\r\n");
            SendUnsolicitedResult(KAT);
            StartTimer(KRestTimeout, EEventExtClose);
            break;
            }
        case EEventExtClose:
            {
            delete this;
            }
        }
    }

void CAtExtPluginStub::CancelRequest(CGenericActive& /*aActive*/)
    {
    iTimer.Cancel();
    }

TInt CAtExtPluginStub::HandleRunError(TInt /*aErr*/)
    {
    return KErrNone;
    }

CAtExtPluginStub::CAtExtPluginStub() : CATExtPluginBase()
    {
    }
     
void CAtExtPluginStub::ConstructL()
    {
    iActive = CGenericActive::NewL(*this, CActive::EPriorityStandard, EEventAT);
    iTimer.CreateLocal();
    StartTimer(KFirstTimeout, EEventAT);
   	}


void CAtExtPluginStub::StartTimer(TInt aTimeout, TInt aId)
    {
    iActive->SetRequestId(aId);
    iTimer.After(iActive->RequestStatus(), aTimeout);
    iActive->GoActive();
   	}
    
//  End of File  
