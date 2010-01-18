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


#include "btmccallactive.h"
#include "debug.h"

// -----------------------------------------------------------------------------
// CBtmcCallActive::NewL
// -----------------------------------------------------------------------------
CBtmcCallActive* CBtmcCallActive::NewL(
    MBtmcActiveObserver& aObserver, 
    CActive::TPriority aPriority,
    TInt aServiceId,
    RMobileLine& aLine,
    const TName& aName)
    {
    CBtmcCallActive* self = CBtmcCallActive::NewLC(aObserver, aPriority, aServiceId, aLine, aName);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcCallActive::NewLC
// -----------------------------------------------------------------------------
CBtmcCallActive* CBtmcCallActive::NewLC(
    MBtmcActiveObserver& aObserver, 
    CActive::TPriority aPriority, 
    TInt aServiceId,
    RMobileLine& aLine,
    const TName& aName)
    {
    CBtmcCallActive* self = new (ELeave) CBtmcCallActive(aObserver, aPriority, aServiceId, aLine, aName);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcCallActive::~CBtmcCallActive
// -----------------------------------------------------------------------------
CBtmcCallActive::~CBtmcCallActive()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iCall.Close();
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcCallActive::GoActive
// -------------------------------------------------------------------------------
void CBtmcCallActive::GoActive()
    {
    TRACE_ASSERT(!IsActive(), KErrGeneral);
    iCall.NotifyMobileCallStatusChange(iStatus, iCallStatus);
    SetActive();
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtmcCallActive::CallName
//
// Gets call name.
// -----------------------------------------------------------------------------
//
const TDesC& CBtmcCallActive::CallName() const
    {
    return iName;
    }

// -----------------------------------------------------------------------------
// CBtmcCallActive::RemotePartyNumber
//
// Get caller's phone number.
// -----------------------------------------------------------------------------
//
const TDesC& CBtmcCallActive::RemotePartyNumber() const
    {
    return iRemoteNumber;
    }

// -----------------------------------------------------------------------------
// CBtmcCallActive::CallStatus
//
// -----------------------------------------------------------------------------
//
RMobileCall::TMobileCallStatus CBtmcCallActive::CallStatus() const
    {
    return iCallStatus;
    }

// -------------------------------------------------------------------------------
// CBtmcCallActive::RunL
// -------------------------------------------------------------------------------
void CBtmcCallActive::RunL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("ao status %d"), iStatus.Int()))
    TRACE_INFO((_L("name '%S' new status %d, old status %d"), &iName, iCallStatus, iPrevCallStatus))
    
    if(iCallStatus != iPrevCallStatus)
        {
        iPrevCallStatus = iCallStatus;
    	if( iCallStatus != RMobileCall::EStatusIdle &&
    		!iEmergency )
    		{
    		RMobileCall::TMobileCallInfoV1 myInfo;
    		GetCallInfo( myInfo );
    		if ( myInfo.iValid & RMobileCall::KCallEmergency &&
    			 myInfo.iEmergency )
    			{
    			TRACE_INFO((_L("Emergency call object detected")))
            	iEmergency = ETrue;
            	}
    		}

        Observer().RequestCompletedL(*this, iStatus.Int());
        }
    else if( iCallStatus == RMobileCall::EStatusIdle )
        {
        Observer().RequestCompletedL(*this, iStatus.Int());    
        }
    else
        {
        GoActive();
        }
        
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcCallActive::DoCancel
// -------------------------------------------------------------------------------
void CBtmcCallActive::DoCancel()
    {
    iCall.CancelAsyncRequest(EMobileCallNotifyMobileCallStatusChange);
    TRACE_FUNC
    }

// -------------------------------------------------------------------------------
// CBtmcCallActive::RunError
// -------------------------------------------------------------------------------
TInt CBtmcCallActive::RunError(TInt /*aErr*/)
    {
    Cancel();
    iCall.Close();
    TRACE_FUNC
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtmcCallActive::CBtmcCallActive
// -----------------------------------------------------------------------------
CBtmcCallActive::CBtmcCallActive(
    MBtmcActiveObserver& aObserver,
    CActive::TPriority aPriority,
    TInt aServiceId,
    RMobileLine& aLine,
    const TName& aName)
    : CBtmcActive(aObserver, aPriority, aServiceId), iLine(aLine)
    {
    iName.Copy(aName);
    }

void CBtmcCallActive::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iCall.OpenExistingCall(iLine, iName))
    LEAVE_IF_ERROR(iCall.GetMobileCallStatus(iCallStatus))
    iPrevCallStatus = iCallStatus;
    TRACE_INFO((_L("name '%S' status %d"), &iName, iCallStatus))

    RMobileCall::TMobileCallInfoV1 info;
    RMobileCall::TMobileCallInfoV1Pckg package(info);
    LEAVE_IF_ERROR(iCall.GetMobileCallInfo(package))
    iCall.GetMobileCallInfo(package);
    if ((info.iValid & RMobileCall::KCallRemoteParty) && 
        info.iRemoteParty.iRemoteIdStatus == RMobileCall::ERemoteIdentityAvailable)
        {
        iRemoteNumber.Copy(info.iRemoteParty.iRemoteNumber.iTelNumber);
        }
	else if (info.iValid & RMobileCall::KCallDialledParty)
		{
		//number for outgoing call
		iRemoteNumber.Copy(info.iDialledParty.iTelNumber);
		}

    TRACE_INFO((_L("remote party '%S' "), &iRemoteNumber))
 
    TRACE_FUNC_EXIT
    }

// -----------------------------------------------------------------------------
// CBTMonoCallActive::GetCallInfo
// -----------------------------------------------------------------------------
void CBtmcCallActive::GetCallInfo(RMobileCall::TMobileCallInfoV1& info)
    {
    RMobileCall::TMobileCallInfoV1Pckg package(info);
    iCall.GetMobileCallInfo(package);
    }

// -----------------------------------------------------------------------------
// CBTMonoCallActive::IsEmergencyCall
// -----------------------------------------------------------------------------
TBool CBtmcCallActive::IsEmergencyCall() const
		{
		return iEmergency;
		}

// End of File
