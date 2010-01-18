/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The base active object declaration
*
*/


#ifndef C_BASRVACTIVENOTIFIER_H
#define C_BASRVACTIVENOTIFIER_H

//  INCLUDES
#include <e32base.h>
#include <btnotif.h>
#include "basrvactive.h"

class TBTDevAddr;

/**
 * An abstract Active Object.
 *
 * This class provides TRequestStatus for an async operation but it doesn't know the
 * detail of the request. CBasrvActive could be used to implement timers, P&S subscribes
 * that are "simple" enough operations.
 *
 * @since S60 v3.1
 */
class CBasrvActiveNotifier : public CBasrvActive
    {
public:
    static CBasrvActiveNotifier* NewL(MBasrvActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId, const TBTDevAddr& aAddr, TBTGenericInfoNoteType aNote);

    virtual ~CBasrvActiveNotifier();

private:
    void GoActive();

private:
    void ConstructL();

    void DoCancel();

protected:
    CBasrvActiveNotifier(MBasrvActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId, const TBTDevAddr& iAddr, TBTGenericInfoNoteType aNote);

    MBasrvActiveObserver& Observer();

private:
    RNotifier iNotifier;
    TBTGenericInfoNotiferParamsPckg iPckg;
    TBuf8<sizeof(TInt)> iNoResult;
    };

#endif  // C_BASRVACTIVE_H
