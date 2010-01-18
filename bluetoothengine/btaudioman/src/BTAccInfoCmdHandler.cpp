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
* Description:  Loaded by Accessory Server to retrieve information of an accessory
*
*/


// INCLUDE FILES
#include <AccPolCommonNameValuePairs.h>

#include "BTAccInfoCmdHandler.h"
#include "debug.h"

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTAccInfoCmdHandler::CBTAccInfoCmdHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTAccInfoCmdHandler::CBTAccInfoCmdHandler()
    {
    
    }

// -----------------------------------------------------------------------------
// CBTAccInfoCmdHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTAccInfoCmdHandler* CBTAccInfoCmdHandler::NewL()
    {
    return new (ELeave) CBTAccInfoCmdHandler();
    }

// Destructor
CBTAccInfoCmdHandler::~CBTAccInfoCmdHandler()
    {
    TRACE_FUNC_ENTRY
    delete iRequester;
    TRACE_FUNC_EXIT
    }
    
// -----------------------------------------------------------------------------
// CBTAccInfoCmdHandler::ProcessCommandL
// -----------------------------------------------------------------------------
//
void CBTAccInfoCmdHandler::ProcessCommandL(
                                  const TProcessCmdId aCommand,
                                  const TASYCmdParams& aCmdParams)
    {
    TRACE_FUNC;
    TRACE_INFO((_L(" aCommand %d"), aCommand));
    switch (aCommand)
        {
        case ECmdProcessCommandInit:
            {
            TAccValueTypeTBool boolInitResponse;            
            boolInitResponse.iValue = ETrue;
            ProcessResponseL( boolInitResponse );            
            break;
            }
        case ECmdGetSupportedBTProfiles :
            {
            if (iRequester)
                {
                TASYBTAccInfo info;
                TASYBTAccInfoPckgC btInfoPckg(info);
                ProcessResponseL(btInfoPckg, KErrInUse);
                break;
                }
            iRequester = CBTAccInfoRequester::NewL(*this);
            TBTDevAddr addr(aCmdParams().iCmdValue);
        	TRACE_INFO_SEG(
        	    {TBuf<12> buf; addr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);}
        	    );
            iRequester->GetBTAccInfoL(addr);
            break;
            }
        default:
            break;
        }
    }
    
// -----------------------------------------------------------------------------
// CBTAccInfoCmdHandler::GetBTAccInfoCompleted
// -----------------------------------------------------------------------------
//
void CBTAccInfoCmdHandler::GetBTAccInfoCompletedL(TInt aErr, const TASYBTAccInfo& aAccInfo)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L(" aErr %d"), aErr));
    TASYBTAccInfoPckgC btInfoPckg(aAccInfo);
    ProcessResponseL(btInfoPckg, aErr);
    delete iRequester;
    iRequester = NULL;
    TRACE_FUNC_EXIT
    }


//  End of File
