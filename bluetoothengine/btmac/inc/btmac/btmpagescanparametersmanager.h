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

#ifndef BTMPAGESCANPARAMETERSMANAGER_H
#define BTMPAGESCANPARAMETERSMANAGER_H

#include <e32base.h>
#include <e32property.h>
#include "btmactive.h"

class CBtmPageScanParametersManager : public CBase, public MBtmActiveObserver
    {
public:
    static CBtmPageScanParametersManager* NewL();
    ~CBtmPageScanParametersManager();

    void Activate();
    void Deactivate();

private:  // From MBtmActiveObserver
    void RequestCompletedL(CBtmActive& aActive);
    void CancelRequest(CBtmActive& aActive);

private:
    void StartTimer(TInt aService, TInt aTimeout);
    void StopTimer();

    void DisableFastConnection();

protected:
    CBtmPageScanParametersManager();
    void ConstructL();

protected:
    RProperty       iCallStateKey;
    CBtmActive*     iCallStateListener;
    RProperty       iPageScanParametersKey;
    CBtmActive*     iPageScanParametersListener;
    RTimer          iTimer;
    CBtmActive*     iTimerActive;
    TBool           iIsBeingDeactivated;
    };

#endif // C_BTMSDISCONNECT_H
