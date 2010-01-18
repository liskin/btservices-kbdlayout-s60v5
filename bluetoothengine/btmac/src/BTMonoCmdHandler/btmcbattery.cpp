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
#include "btmcbattery.h"
#include "btmcphonestatus.h"
#include "debug.h"
#include <e32math.h>

const TInt8 KMaxPhoneStrength = 7;
const TInt8 KMaxHFPStrength = 5;

// -----------------------------------------------------------------------------
// CBtmcBattery::NewL
// -----------------------------------------------------------------------------
CBtmcBattery* CBtmcBattery::NewL(CBtmcPhoneStatus& aParent)
    {
    CBtmcBattery* self = CBtmcBattery::NewLC(aParent);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcBattery::NewLC
// -----------------------------------------------------------------------------
CBtmcBattery* CBtmcBattery::NewLC(CBtmcPhoneStatus& aParent)
    {
    CBtmcBattery* self = new (ELeave) CBtmcBattery(aParent);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcBattery::~CBtmcBattery
// -----------------------------------------------------------------------------
CBtmcBattery::~CBtmcBattery()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iProperty.Close();
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcBattery::GoActive
// -------------------------------------------------------------------------------
void CBtmcBattery::GoActive()
    {
    TRACE_ASSERT(!IsActive(), KErrGeneral);
    iProperty.Subscribe(iStatus);
    SetActive();
    TRACE_FUNC
    }


// -------------------------------------------------------------------------------
// CBtmcBattery::RunL
// -------------------------------------------------------------------------------
void CBtmcBattery::RunL()
    {
    TRACE_FUNC_ENTRY
    if (iStatus == KErrNone)
        {    
        TInt temp = 0;
        TInt err = iProperty.Get(temp);
        if (!err)
            {
            ConvertToHFPScale(temp);
            iCharge = temp;
            TRACE_INFO((_L("iStatus %d, battery strength %d"), iStatus.Int(), temp))
            iParent.HandleBatteryChangeL(temp);            
            }
        GoActive();
        }
    else
        {
        TRACE_ERROR((_L("ERROR %d"), iStatus.Int()))
        }        
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcBattery::DoCancel
// -------------------------------------------------------------------------------
void CBtmcBattery::DoCancel()
    {
    iProperty.Cancel();
    TRACE_FUNC
    }


// -------------------------------------------------------------------------------
// CBtmcBattery::RunError
// -------------------------------------------------------------------------------
TInt CBtmcBattery::RunError(TInt /*aErr*/)
    {
    iProperty.Close();
    TRACE_FUNC
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtmcBattery::CBtmcBattery
// -----------------------------------------------------------------------------
CBtmcBattery::CBtmcBattery(CBtmcPhoneStatus& aParent)
    : CActive(EPriorityNormal), iParent(aParent)
    {
    CActiveScheduler::Add(this);
    }

void CBtmcBattery::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iProperty.Attach(KPSUidHWRMPowerState, KHWRMBatteryLevel));
    LEAVE_IF_ERROR(iProperty.Get(iCharge));
    ConvertToHFPScale(iCharge);
    TRACE_FUNC_EXIT
    }

void CBtmcBattery::ConvertToHFPScale(TInt &aStrength)
    {
    TReal result;
    Math::Round( result, TReal(TReal(KMaxHFPStrength)/TReal(KMaxPhoneStrength) *TReal(aStrength)),0);
    aStrength = TInt8(result);    
    }

TInt CBtmcBattery::GetBatteryCharge()
    {
    return iCharge;
    }


// End of File
