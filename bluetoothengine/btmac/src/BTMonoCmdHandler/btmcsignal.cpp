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
* Description:  General Active Object offering asynchronous service
*
*/

#include "atcodec.h"
#include "btmcphonestatus.h"
#include "btmcsignal.h"
#include "debug.h"
#include <mmtsy_names.h>
#include <e32math.h>

const TInt8 KMaxPhoneSignal = 7;
const TInt8 KMaxHFPSignal = 5;

// -----------------------------------------------------------------------------
// CBtmcSignal::NewL
// -----------------------------------------------------------------------------
CBtmcSignal* CBtmcSignal::NewL(CBtmcPhoneStatus& aParent)
    {
    CBtmcSignal* self = CBtmcSignal::NewLC(aParent);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcSignal::NewLC
// -----------------------------------------------------------------------------
CBtmcSignal* CBtmcSignal::NewLC(CBtmcPhoneStatus& aParent)
    {
    CBtmcSignal* self = new (ELeave) CBtmcSignal(aParent);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcSignal::~CBtmcSignal
// -----------------------------------------------------------------------------
CBtmcSignal::~CBtmcSignal()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iPhone.Close();
    iServer.UnloadPhoneModule(KMmTsyModuleName);
    iServer.Close();
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcSignal::GoActive
// -------------------------------------------------------------------------------
void CBtmcSignal::GoActive()
    {
    if (!IsActive())
        {
        iPhone.NotifySignalStrengthChange(iStatus, iDbm, iBars);
        SetActive();
        }
    TRACE_FUNC
    }


// -------------------------------------------------------------------------------
// CBtmcSignal::RunL
// -------------------------------------------------------------------------------
void CBtmcSignal::RunL()
    {
    TRACE_FUNC_ENTRY
    TInt err = iStatus.Int();
    TRACE_INFO((_L("err %d"), err))
    if (!err)
        {
        ConvertToHFPScale(iBars);
        }
    else
        {
        iBars = 0;
        }        
    if (iPreviousBars != iBars)
        {
        iParent.HandleNetworkStrengthChangeL(iBars);
        iPreviousBars = iBars;
        }
    if (err != KErrServerTerminated)
        {
        GoActive();
        }
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcSignal::DoCancel
// -------------------------------------------------------------------------------
void CBtmcSignal::DoCancel()
    {
    iPhone.CancelAsyncRequest(EMobilePhoneNotifySignalStrengthChange);
    TRACE_FUNC
    }


// -------------------------------------------------------------------------------
// CBtmcSignal::RunError
// -------------------------------------------------------------------------------
TInt CBtmcSignal::RunError(TInt /*aErr*/)
    {
    iPhone.Close();
    iServer.UnloadPhoneModule(KMmTsyModuleName);
    iServer.Close();
    TRACE_FUNC
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtmcSignal::CBtmcSignal
// -----------------------------------------------------------------------------
CBtmcSignal::CBtmcSignal(CBtmcPhoneStatus& aParent)
    : CActive(EPriorityNormal), iParent(aParent), iPreviousBars(0)
    {
    CActiveScheduler::Add(this);
    }

void CBtmcSignal::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iServer.Connect());
    iServer.LoadPhoneModule(KMmTsyModuleName);
    LEAVE_IF_ERROR(iPhone.Open(iServer, KMmTsyPhoneName));
    iPhone.GetSignalStrength(iStatus, iDbm, iBars);
    SetActive();
    TRACE_FUNC_EXIT
    }

void CBtmcSignal::ConvertToHFPScale(TInt8 &aSignal)
    {
    TReal result;
    Math::Round( result, TReal(TReal(KMaxHFPSignal)/TReal(KMaxPhoneSignal) *TReal(aSignal)),0);
    aSignal = TInt8(result);
    }

TInt CBtmcSignal::GetSignalStrength()
    {
    return iBars;
    }

TInt CBtmcSignal::GetRssiStrength()
	{
	return iDbm;
	}
// End of File
