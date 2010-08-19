// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "btmpagescanparametersmanager.h"
#include <ctsydomainpskeys.h>
#include <bt_subscribe.h>
#include <bt_subscribe_partner.h>
#include <bttypespartner.h>
#include "debug.h"

// in the current configuration an incoming call ringing limit is 30 seconds
// if for some reason we don't receive the call state change
// we disable fast connection page scan settings in 35 seconds
const TInt KFastConnectionTime              = 35000000;
// a timeout for resetting page scan settings
// if we don't receive a confirmation we try to reset again
const TInt KRestoreStandardParametersTime   = 1000000;

const TInt KCallStateListenerService            = 8;
const TInt KPageScanParametersListenerService   = 9;
const TInt KFastConnectionService               = 10;
const TInt KRestoreStandardParametersService    = 11;

CBtmPageScanParametersManager::CBtmPageScanParametersManager()
    :  iIsBeingDeactivated(EFalse)
    {
    }

CBtmPageScanParametersManager::~CBtmPageScanParametersManager()
    {
    TRACE_FUNC_ENTRY
    delete iTimerActive;
    iTimer.Close();
    if (iCallStateKey.Handle())
        {
        iCallStateKey.Cancel();
        }
    delete iPageScanParametersListener;
    iCallStateKey.Close();
    if (iPageScanParametersKey.Handle())
        {
        iPageScanParametersKey.Cancel();
        }
    delete iCallStateListener;
    iPageScanParametersKey.Close();
    TRACE_FUNC_EXIT
    }

void CBtmPageScanParametersManager::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iCallStateKey.Attach(KPSUidCtsyCallInformation, KCTsyCallState));
    iCallStateListener = CBtmActive::NewL(*this, CActive::EPriorityStandard,
                                          KCallStateListenerService);

    LEAVE_IF_ERROR(iPageScanParametersKey.Attach(KPropertyUidBluetoothCategory,
                                                 KPropertyKeyBluetoothGetPageScanParameters));
    iPageScanParametersListener = CBtmActive::NewL(*this, CActive::EPriorityStandard,
                                                   KPageScanParametersListenerService);

    iTimerActive = CBtmActive::NewL(*this, CActive::EPriorityStandard, KFastConnectionService);
    LEAVE_IF_ERROR(iTimer.CreateLocal());
    TRACE_FUNC_EXIT
    }

CBtmPageScanParametersManager* CBtmPageScanParametersManager::NewL()
    {
    TRACE_STATIC_FUNC_ENTRY
    CBtmPageScanParametersManager* self = new( ELeave ) CBtmPageScanParametersManager();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CBtmPageScanParametersManager::StartTimer(TInt aService, TInt aTimeout)
    {
    if (!iTimerActive->IsActive())
        {
        TRACE_FUNC_ENTRY
        iTimer.After(iTimerActive->iStatus, aTimeout);
        iTimerActive->SetRequestId(aService);
        iTimerActive->GoActive();
        TRACE_FUNC_EXIT
        }
    else
        {
        TRACE_WARNING(_L("WARNING, page scan manager timer is already active"))
        }
    }

void CBtmPageScanParametersManager::StopTimer()
    {
    if (iTimerActive->IsActive())
        {
        TRACE_FUNC_ENTRY
        iTimerActive->Cancel();
        TRACE_FUNC_EXIT
        }
    }

void CBtmPageScanParametersManager::RequestCompletedL(CBtmActive& aActive)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("request id %d status %d"), aActive.RequestId(), aActive.iStatus.Int()))
    if (aActive.iStatus.Int() == KErrNone)
        {
        switch (aActive.RequestId())
            {
            case KCallStateListenerService:
                {
                iCallStateKey.Subscribe(aActive.iStatus);
                aActive.GoActive();

                TInt callState;
                TInt ret = iCallStateKey.Get(callState);
                if (ret == KErrNone)
                    {
                    TRACE_INFO((_L("call state received %d"), callState))
                    if (callState == EPSCTsyCallStateRinging)
                        {
                        // an incoming call ringing => switch to fast page scanning
                        TRACE_INFO((_L("switching to fast page scanning")))
                        (void) RProperty::Set(KPropertyUidBluetoothCategory,
                                              KPropertyKeyBluetoothSetPageScanParameters,
                                              EFastConnectionPageScanParameters);

                        // we will disable fast page scanning when the timer gets expired
                        StartTimer(KFastConnectionService, KFastConnectionTime);
                        }
                    else if (iTimerActive->IsActive() && iTimerActive->RequestId() == KFastConnectionService)
                        {
                        // the incoming call is not ringing anymore => switch to standard page scanning
                        TRACE_INFO((_L("switching to standard page scanning")))
                        DisableFastConnection();
                        }
                    }
                break;
                }

            case KPageScanParametersListenerService:
                {
                iPageScanParametersKey.Subscribe(aActive.iStatus);
                aActive.GoActive();

                TInt pageScanParameters;
                TInt ret = iPageScanParametersKey.Get(pageScanParameters);
                if (ret == KErrNone)
                    {
                    TRACE_INFO((_L("page scan parameters received %d"), pageScanParameters))
                    if (iTimerActive->IsActive() && iTimerActive->RequestId() == KRestoreStandardParametersService
                        && pageScanParameters == EStandardPageScanParameters)
                        {
                        TRACE_INFO((_L("standard page scan parameters enabled")))
                        StopTimer();
                        if (iIsBeingDeactivated)
                            {
                            TRACE_INFO((_L("page scan parameters service is stopped")))
                            iPageScanParametersKey.Cancel();
                            }
                        }
                    }
                break;
                }

            case KFastConnectionService:
            case KRestoreStandardParametersService:
                {
                // try to reset the page scan setting because it hasn't been confirmed yet
                TRACE_INFO((_L("timer expired, switching to standard page scanning")))
                DisableFastConnection();
                break;
                }

            default:
                {
                TRACE_WARNING(_L("WARNING, unknown service"))
                break;
                }
            }
        }
    TRACE_FUNC_EXIT
    }

void CBtmPageScanParametersManager::CancelRequest(CBtmActive& aActive)
    {
    TRACE_FUNC_ENTRY
    switch (aActive.RequestId())
        {
        case KFastConnectionService:
        case KRestoreStandardParametersService:
            {
            iTimer.Cancel();
            break;
            }
        default:
            break;
        }
    TRACE_FUNC_EXIT
    }

void CBtmPageScanParametersManager::DisableFastConnection()
    {
    TRACE_FUNC_ENTRY
    StopTimer();

    // switch to standard page scanning
    (void) RProperty::Set(KPropertyUidBluetoothCategory,
                          KPropertyKeyBluetoothSetPageScanParameters,
                          EStandardPageScanParameters);

    // we will disable fast page scanning when the timer gets expired
    StartTimer(KRestoreStandardParametersService, KRestoreStandardParametersTime);
    TRACE_FUNC_EXIT
    }

void CBtmPageScanParametersManager::Activate()
    {
    TRACE_FUNC_ENTRY
    iIsBeingDeactivated = EFalse;

    if (!iCallStateListener->IsActive())
        {
        iCallStateKey.Subscribe(iCallStateListener->iStatus);
        iCallStateListener->GoActive();
        }
    else
        {
        TRACE_WARNING(_L("WARNING, call state is already being listened"))
        }

    if (!iPageScanParametersListener->IsActive())
        {
        iPageScanParametersKey.Subscribe(iPageScanParametersListener->iStatus);
        iPageScanParametersListener->GoActive();
        }
    else
        {
        TRACE_WARNING(_L("WARNING, call state is already being listened"))
        }

    TInt callState;
    TInt err = RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
    if(err == KErrNone && callState == EPSCTsyCallStateRinging)
        {
        // an incoming call already ringing => switch to fast page scanning
        TRACE_INFO((_L("call is already ringing, switching to fast page scanning")))
        (void) RProperty::Set(KPropertyUidBluetoothCategory,
                              KPropertyKeyBluetoothSetPageScanParameters,
                              EFastConnectionPageScanParameters);

        // we will disable fast page scanning when the timer gets expired
        StartTimer(KFastConnectionService, KFastConnectionTime);
        }
    TRACE_FUNC_EXIT
    }

void CBtmPageScanParametersManager::Deactivate()
    {
    TRACE_FUNC_ENTRY
    iIsBeingDeactivated = ETrue;

    // stop call state listening
    iCallStateKey.Cancel();

    if (iTimerActive->IsActive())
        {
        // if fast page scanning is active, disable it
        // if we're already switching to standard page scanning, do nothing
        if (iTimerActive->RequestId() == KFastConnectionService)
            {
            TRACE_INFO((_L("deactivation started, switching to standard page scanning")))
            DisableFastConnection();
            }
        }
    else
        {
        // if no timer is active we just stop page scan parameters listening
        iPageScanParametersKey.Cancel();
        }
    TRACE_FUNC_EXIT
    }
