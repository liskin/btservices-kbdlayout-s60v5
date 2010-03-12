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
#include "btmcoperator.h"
#include "btmcprotocol.h"
#include <mmtsy_names.h>
#include "debug.h"

// -----------------------------------------------------------------------------
// CBtmcOperator::NewL
// -----------------------------------------------------------------------------
CBtmcOperator* CBtmcOperator::NewL(            
    MBtmcActiveObserver& aObserver,
    CBtmcProtocol& aProtocol, 
    CActive::TPriority aPriority, 
    TInt aServiceId)
    {
    CBtmcOperator* self = new (ELeave) CBtmcOperator(aObserver, aProtocol, aPriority, aServiceId);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcOperator::~CBtmcOperator
// -----------------------------------------------------------------------------
CBtmcOperator::~CBtmcOperator()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iCustomApi.Close();
    iPhone.Close();
    iServer.UnloadPhoneModule(KMmTsyModuleName);
    iServer.Close();
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcOperator::GoActive
// -------------------------------------------------------------------------------
void CBtmcOperator::GoActive()
    {
    TRACE_ASSERT(!IsActive(), KErrGeneral);
    iInfo.iType = RMmCustomAPI::EOperatorNameNitzShort;
    iCustomApi.GetOperatorName(iStatus, iInfo);
    SetActive();
    TRACE_FUNC
    }
    
// -------------------------------------------------------------------------------
// CBtmcOperator::GetName
// -------------------------------------------------------------------------------
const TDesC& CBtmcOperator::GetName() const
    {
    TRACE_FUNC
    return iInfo.iName;
    }

// -------------------------------------------------------------------------------
// CBtmcOperator::RunL
// -------------------------------------------------------------------------------
void CBtmcOperator::RunL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("ao status %d, operator '%S'"), iStatus.Int(), &(iInfo.iName)))
    RATResultPtrArray resarr;
    ATObjArrayCleanupResetAndDestroyPushL(resarr);    

    RATParamArray params;
    CleanupClosePushL(params);
    LEAVE_IF_ERROR(params.Append(TATParam(0)));

    if (iStatus == KErrNone)
        {
        LEAVE_IF_ERROR(params.Append(TATParam(0)));
        TBuf8<KMaxOperatorNameLength> buf8;
        buf8.Copy(iInfo.iName); // need Unicode convert?
        LEAVE_IF_ERROR(params.Append(TATParam(buf8, EATDQStringParam)));
        }
    else
        {
        LEAVE_IF_ERROR(params.Append(TATParam()));
        LEAVE_IF_ERROR(params.Append(TATParam()));
        }

    CATResult* code = CATResult::NewL(EATCOPS, EATReadResult, &params);
    CleanupStack::PushL(code);
    resarr.AppendL(code);
    CleanupStack::Pop(code);
    CleanupStack::PopAndDestroy(&params);

    CATResult* ok = CATResult::NewL(EATOK);
    CleanupStack::PushL(ok);
    resarr.AppendL(ok);
    CleanupStack::Pop(ok);
    iProtocol.SendResponseL(resarr);
    CleanupStack::PopAndDestroy(&resarr);
    //in this case this function call actually can't leave
    Observer().RequestCompletedL(*this, iStatus.Int());
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcOperator::DoCancel
// -------------------------------------------------------------------------------
void CBtmcOperator::DoCancel()
    {
    TRACE_FUNC
    }

// -------------------------------------------------------------------------------
// CBtmcOperator::RunError
// -------------------------------------------------------------------------------
TInt CBtmcOperator::RunError(TInt aErr)
    {
    TRACE_FUNC
    //this is called here so that the command handling tries to continue
    //in this case this function call actually can't leave, ignoring it
    TInt err;
    TRAP( err, Observer().RequestCompletedL(*this, aErr); )
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtmcOperator::CBtmcOperator
// -----------------------------------------------------------------------------
CBtmcOperator::CBtmcOperator(
    MBtmcActiveObserver& aObserver, 
    CBtmcProtocol& aProtocol, 
    CActive::TPriority aPriority, 
    TInt aServiceId)
    : CBtmcActive(aObserver, aPriority, aServiceId), iProtocol(aProtocol)
    {
    }

void CBtmcOperator::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iServer.Connect());
    iServer.LoadPhoneModule(KMmTsyModuleName);
    LEAVE_IF_ERROR(iPhone.Open(iServer, KMmTsyPhoneName));
    LEAVE_IF_ERROR(iCustomApi.Open(iPhone));
    TRACE_FUNC_EXIT
    }

// End of File
