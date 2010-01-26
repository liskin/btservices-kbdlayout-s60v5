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
* Description:  call cmd handling. 
*
*/


// INCLUDE FILES

#include "atcodec.h"
#include "btengprivatepskeys.h"
#include "btmccallinghandler.h"
#include "btmcprotocol.h"
#include "debug.h"

const TInt KCallingResponse = 30;

// -----------------------------------------------------------------------------
// CBtmcCallingHandler::NewL
// -----------------------------------------------------------------------------
CBtmcCallingHandler* CBtmcCallingHandler::NewL(CBtmcProtocol& aProtocol) 
    {
    CBtmcCallingHandler* self = new(ELeave) CBtmcCallingHandler(aProtocol);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcCallingHandler::~CBtmcCallingHandler
// -----------------------------------------------------------------------------
CBtmcCallingHandler::~CBtmcCallingHandler()
    {
    TRACE_FUNC_ENTRY
    delete iActive;
    iRespProperty.Close();
    iCmdProperty.Close();
    TRACE_FUNC_EXIT
    }


void CBtmcCallingHandler::HandleCallingCmdL(const CATCommand& aCmd)
    {
    TRACE_FUNC_ENTRY
    TRACE_ASSERT(!iActive->IsActive(), KErrInUse)
    // Delegate the command to btmonobearer for processing
    TInt err = iCmdProperty.Set(aCmd.Des());
    if (err)
    	{
    	TRACE_INFO((_L8("PS set returned %d"), err))
        CATResult* nok = CATResult::NewLC(EATERROR);
        iProtocol.SendResponseL(*nok);
        CleanupStack::PopAndDestroy(nok);
        iProtocol.CmdHandlingCompletedL();
    	}
    else
        {
        // wait for the response from btmonobearer
        iCmdId = aCmd.Id();
        iRespProperty.Subscribe(iActive->iStatus);
        iActive->GoActive();
        }
    TRACE_FUNC_EXIT
    }

TBool CBtmcCallingHandler::ActiveCmdHandling() const
    {
    return iActive->IsActive();
    }

TBool CBtmcCallingHandler::ActiveChldHandling() const
    {
    return ActiveCmdHandling() && ( iCmdId == EATCHLD );
    }

void CBtmcCallingHandler::RequestCompletedL(CBtmcActive& aActive, TInt aErr)
    {
    switch (aActive.ServiceId())
        {
        case KCallingResponse:
            {
            TInt result = KErrNone;
            if (!aErr)
                {
                TBuf8<KMaxATSize> buf;
                aErr = iRespProperty.Get(buf);
                if (!aErr && buf.Length() >= sizeof(TInt))
                    {
                    const TUint8* ptr = buf.Ptr();
                    result = *((const TInt*)ptr);
                    }
                }
            TRACE_INFO((_L("resp %d"), result))
            TATId atid = EATOK;
            if (aErr || result)
                {
                atid = EATERROR;
                if (iCmdId == EATBVRA)
                    {
                    iProtocol.VoiceRecognitionError();
                    }
                }
            CATResult* nok = CATResult::NewLC(atid);
            iProtocol.SendResponseL(*nok);
            CleanupStack::PopAndDestroy(nok);
            iProtocol.CmdHandlingCompletedL();
            break;
            }
        default:
            break;
        }
    }

void CBtmcCallingHandler::CancelRequest(TInt aServiceId)
    {
    switch (aServiceId)
        {
        case KCallingResponse:
            {
            iRespProperty.Cancel();
            break;    
            }
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBtmcCallingHandler::CBtmcCallingHandler
// -----------------------------------------------------------------------------
CBtmcCallingHandler::CBtmcCallingHandler(CBtmcProtocol& aProtocol)
    : iProtocol(aProtocol)
    {
    }

// -----------------------------------------------------------------------------
// CBtmcCallingHandler::ConstructL
// -----------------------------------------------------------------------------
void CBtmcCallingHandler::ConstructL()
    {
    TRACE_FUNC
    LEAVE_IF_ERROR( iCmdProperty.Attach(KPSUidBluetoothEnginePrivateCategory, KBTHfpATCommand) );
    LEAVE_IF_ERROR( iRespProperty.Attach(KPSUidBluetoothEnginePrivateCategory, KBTHfpATResponse) );
    iActive = CBtmcActive::NewL(*this, CActive::EPriorityStandard, KCallingResponse);
    TRACE_FUNC_EXIT  
    }
    
// End of file
