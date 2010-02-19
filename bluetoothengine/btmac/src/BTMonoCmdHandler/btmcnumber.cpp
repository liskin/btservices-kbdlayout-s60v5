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
    GetMsisdnStoreInfo();
    TRACE_FUNC
    }

// -------------------------------------------------------------------------------
// CBtmcNumber::GetMsisdnStoreInfo
// -------------------------------------------------------------------------------
void CBtmcNumber::GetMsisdnStoreInfo()
    {
    TRACE_FUNC_ENTRY
    iState = EStateGettingMsisdnStoreInfo;
    iStore.GetInfo(iStatus, iOnStoreInfoPckg);
    SetActive();
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcNumber::GetMsisdnStoreEntry
// -------------------------------------------------------------------------------
void CBtmcNumber::GetMsisdnStoreEntry()
    {
    TRACE_FUNC_ENTRY
    iState = EStateGettingMsisdnStoreEntry;
    iEntry.iIndex = iUsedEntriesCount++;
    iStore.Read(iStatus, iPckg);
    SetActive();
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcNumber::RunL
// -------------------------------------------------------------------------------
void CBtmcNumber::RunL()
    {
    TRACE_FUNC_ENTRY
    
    switch (iState)
        {
        case EStateGettingMsisdnStoreEntry:
            {
            if (iStatus == KErrNone)
                {
                if(iEntry.iNumber.iTelNumber.Length() == 0)
                    {
                    GetMsisdnStoreEntry();
                    break;
                    }
              
                iCorrectNumbersCount++;

                RATResultPtrArray resarr;
                ATObjArrayCleanupResetAndDestroyPushL(resarr);
                
                TBuf8<RMobileONStore::KOwnNumberTextSize> telnumberBuf;
                telnumberBuf.Copy(iEntry.iNumber.iTelNumber);
                
                TRACE_INFO((_L8("ao status %d, phonebook returned %S"), iStatus.Int(), &telnumberBuf))
                RATParamArray params;
                CleanupClosePushL(params);
                LEAVE_IF_ERROR(params.Append(TATParam()))
                LEAVE_IF_ERROR(params.Append(TATParam(telnumberBuf, EATDQStringParam)))  
                 
                TBTMonoATPhoneNumberType numType;
                if(telnumberBuf.Locate('+') == 0)
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
                
                if(iCorrectNumbersCount == iOnStoreInfo.iUsedEntries)
                    {
                    CATResult* okerr = CATResult::NewL(EATOK);
                    CleanupStack::PushL(okerr);
                    resarr.AppendL(okerr);
                    CleanupStack::Pop(okerr);
                    iProtocol.SendResponseL(resarr);
                    CleanupStack::PopAndDestroy(&resarr);
                    Observer().RequestCompletedL(*this, iStatus.Int());  
                    }
                else
                    {
                    iProtocol.SendResponseL(resarr);
                    CleanupStack::PopAndDestroy(&resarr);
                    GetMsisdnStoreEntry();
                    }
                }
            else
                {
                CATResult* okerr = CATResult::NewL(EATERROR);
                CleanupStack::PushL(okerr);
                iProtocol.SendResponseL(*okerr);
                CleanupStack::PopAndDestroy(okerr);
                Observer().RequestCompletedL(*this, iStatus.Int());  
                }

            break;
            }
        case EStateGettingMsisdnStoreInfo:
            {
            if(iStatus==KErrNone && iOnStoreInfo.iUsedEntries>0 && 
                    (iOnStoreInfo.iCaps & RMobilePhoneStore::KCapsIndividualEntry))
                {
                GetMsisdnStoreEntry();
                }
            else
                {
                CATResult* okerr = CATResult::NewL(EATOK);
                CleanupStack::PushL(okerr);
                iProtocol.SendResponseL(*okerr);
                CleanupStack::PopAndDestroy(okerr);
                Observer().RequestCompletedL(*this, iStatus.Int());  
                }
            break;
            }
            
        default:
            break;        
        }
    
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
    : CBtmcActive(aObserver, aPriority, aServiceId), iProtocol(aProtocol), iPckg(iEntry),
    iOnStoreInfoPckg(iOnStoreInfo), iState(EStateNull), iUsedEntriesCount(1), iCorrectNumbersCount(0)
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
