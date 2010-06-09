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
* Description:  Main handler for incoming requests that don't require async operation.
*
*/

#include "atcmdsyncbase.h"

#include "debug.h"


CATCmdSyncBase::CATCmdSyncBase(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    iATCmdParser(aATCmdParser),
    iPhone(aPhone)
    {
    TRACE_FUNC_ENTRY
    iCallback = aCallback;
    TRACE_FUNC_EXIT
    }

void CATCmdSyncBase::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }
