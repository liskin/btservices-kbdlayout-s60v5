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
#include "btmcprotocol.h"
#include "btmcnumber.h"
#include <mmtsy_names.h>
#include "debug.h"

// -----------------------------------------------------------------------------
// CBtmcNumber::NewL
// -----------------------------------------------------------------------------
CBtmcNumber* CBtmcNumber::NewL(
    MBtmcActiveObserver& aObserver,
    CBtmcProtocol& aProtocol, 
    CActive::TPriority aPriority, 
    TInt aServiceId)
    {
    CBtmcNumber* self = new (ELeave) CBtmcNumber(aObserver, aProtocol, aPriority, aServiceId);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcNumber::~CBtmcNumber
// -----------------------------------------------------------------------------
CBtmcNumber::~CBtmcNumber()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iStore.Close();
    iPhone.Close();
    iServer.UnloadPhoneModule(KMmTsyModuleName);
    iServer.Close();
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcNumber::GoActive
// -------------------------------------------------------------------------------
void CBtmcNumber::GoActive()
    {
    TRACE_ASSERT(!IsActive(), KErrGeneral);
    iEntry.iIndex = 1;
    iStore.Read(iStatus, iPckg);
    SetActive();
    TRACE_FUNC
    }

// -------------------------------------------------------------------------------
// CBtmcNumber::RunL
// -------------------------------------------------------------------------------
void CBtmcNumber::RunL()
    {
    TRACE_FUNC_ENTRY
    
    RATResultPtrArray resarr;
    ATObjArrayCleanupResetAndDestroyPushL(resarr);
    CATResult* okerr = NULL;
    if (iStatus == KErrNone)
        {
        TBuf8<RMobileONStore::KOwnNumberTextSize> nameBuf;
        nameBuf.Copy(iEntry.iNumber.iTelNumber);
        
        TRACE_INFO((_L8("ao status %d, phonebook returned %S"), iStatus.Int(), &nameBuf))
        RATParamArray params;
        CleanupClosePushL(params);
        LEAVE_IF_ERROR(params.Append(TATParam()))
        LEAVE_IF_ERROR(params.Append(TATParam(nameBuf, EATDQStringParam)))  
         
        TBTMonoATPhoneNumberType numType;
        if (nameBuf.Length() == 0)
            {
            numType = EBTMonoATPhoneNumberUnavailable;
            }
        else if(nameBuf.Locate('+') == 0)
            {
            numType = EBTMonoATPhoneNumberInternational;
            }
        else
            {
            numType = EBTMonoATPhoneNumberNational;
            }
        LEAVE_IF_ERROR(params.Append(TATParam(numType)))
        LEAVE_IF_ERROR(params.Append(TATParam()))
        LEAVE_IF_ERROR(params.Append(TATParam(4)))
        CATResult* code = CATResult::NewL(EATCNUM, EATActionResult, &params);
        CleanupStack::PushL(code);
        resarr.AppendL(code);
        CleanupStack::Pop(code);
        CleanupStack::PopAndDestroy(&params);
        okerr = CATResult::NewL(EATOK);
        }
    else
        {
        okerr = CATResult::NewL(EATERROR);
        }
    CleanupStack::PushL(okerr);
    resarr.AppendL(okerr);
    CleanupStack::Pop(okerr);
    iProtocol.SendResponseL(resarr);
    CleanupStack::PopAndDestroy(&resarr);
    Observer().RequestCompletedL(*this, iStatus.Int());    
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcNumber::DoCancel
// -------------------------------------------------------------------------------
void CBtmcNumber::DoCancel()
    {
    iStore.CancelAsyncRequest(EMobilePhoneStoreRead);
    TRACE_FUNC
    }


// -------------------------------------------------------------------------------
// CBtmcNumber::RunError
// -------------------------------------------------------------------------------
TInt CBtmcNumber::RunError(TInt /*aErr*/)
    {
    TRACE_FUNC
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtmcNumber::CBtmcNumber
// -----------------------------------------------------------------------------
CBtmcNumber::CBtmcNumber(
    MBtmcActiveObserver& aObserver, 
    CBtmcProtocol& aProtocol, 
    CActive::TPriority aPriority, 
    TInt aServiceId)
    : CBtmcActive(aObserver, aPriority, aServiceId), iProtocol(aProtocol), iPckg(iEntry)
    {
    }

void CBtmcNumber::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iServer.Connect());
    iServer.LoadPhoneModule(KMmTsyModuleName);
    LEAVE_IF_ERROR(iPhone.Open(iServer, KMmTsyPhoneName));
     LEAVE_IF_ERROR(iStore.Open(iPhone));//, KETelIccMsisdnPhoneBook));
    TRACE_FUNC_EXIT
    }

// End of File
