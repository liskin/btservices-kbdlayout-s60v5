/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Main handler for incoming requests
*
*/

#include "atmisccmdplugin.h"

#include "debug.h"


CATCmdAsyncBase::CATCmdAsyncBase(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CActive(CActive::EPriorityStandard),
    iATCmdParser(aATCmdParser),
    iPhone(aPhone)
    {
    TRACE_FUNC_ENTRY
    iCallback = aCallback;
    CActiveScheduler::Add(this);
    TRACE_FUNC_EXIT
    }

