/*
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
*       This class implements RemCon bearer pulgin interface.
*
*/

#include "bmbcmdlistener.h"
#include <btengprivatepskeys.h>
#include "debug.h"
#include "bmbpanic.h"

CBmbCmdListener* CBmbCmdListener::NewL(CBmbPlugin& aParent)
    {
    CBmbCmdListener* self = new(ELeave) CBmbCmdListener(aParent);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

CBmbCmdListener::~CBmbCmdListener()
    {
   	Cancel();
   	iATCmdProperty.Close();
   	iATRespProperty.Close();
    }

// ---------------------------------------------------------------------------
// Send the response via PS, and starts listening the next command
// to be handled by this bearer.
// ---------------------------------------------------------------------------
//
void CBmbCmdListener::HandlingDataCompleted( const TDesC8& aResp )
    {
    // No better error handling than ignoring it.
    (void) iATRespProperty.Set( aResp );
    TRACE_FUNC
    }

CBmbCmdListener::CBmbCmdListener(CBmbPlugin& aParent) 
    : CActive(CActive::EPriorityStandard), iParent(aParent)
    {
    CActiveScheduler::Add(this);
    }

void CBmbCmdListener::ConstructL()
    {
	TRACE_FUNC
    LEAVE_IF_ERROR(iATCmdProperty.Attach(KPSUidBluetoothEnginePrivateCategory, KBTHfpATCommand));
	LEAVE_IF_ERROR(iATRespProperty.Attach(KPSUidBluetoothEnginePrivateCategory, KBTHfpATResponse));
	Subscribe();
    }

void CBmbCmdListener::RunL()
    {
    TRACE_FUNC
	TInt err = iStatus.Int();
    Subscribe();
    // Error could be received from Subscribe when the PS
    // key is deleted due to powering BT off. In
    // this case, we just re-subscribe.
	if(err == KErrNone)
		{
	    iAtCmdBuf.Zero();
    	err = iATCmdProperty.Get(iAtCmdBuf);
    	if ( !err && iAtCmdBuf.Length() > 0 )
    	    {
    	    // An AT command to be processed by iParent.
    	    // At command handling completion, iParent will call this object
    	    // to send out response via HandlingDataCompleted().
    	    iParent.DataFromRemote(iAtCmdBuf);
    	    }
	    }
	TRACE_ERROR((_L8("listener Status %d"), err))
    }

void CBmbCmdListener::DoCancel()
    {
    iATCmdProperty.Cancel();
    }

void CBmbCmdListener::Subscribe()
    {
    __ASSERT_ALWAYS( !IsActive(), Panic(EBmbPanicCmdListenerBadState) );
    iATCmdProperty.Subscribe(iStatus);
    SetActive();
    TRACE_FUNC
    }
