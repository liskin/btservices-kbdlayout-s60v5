/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 * Description :
 *
 */

#include <sysutil.h>

#include "cgmrcommandhandler.h"

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCGMRCommandHandler* CCGMRCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCGMRCommandHandler* self = new (ELeave) CCGMRCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCGMRCommandHandler::CCGMRCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdSyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCGMRCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);
    TRACE_FUNC_EXIT
    }

CCGMRCommandHandler::~CCGMRCommandHandler()
    {
    TRACE_FUNC_ENTRY
    iReply.Close();
    TRACE_FUNC_EXIT
    }

void CCGMRCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY
    
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOk );
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeBase):
            {
            TInt replyType = EReplyTypeOk;
            if (iReply.Length() == 0)
                {
                replyType = GetSoftwareVersion();
                }
            
            if ( (EReplyTypeOk != replyType) )
                {
                iCallback->CreateCMEReplyAndComplete(KErrUnknown);
                }
            else
                {
                iCallback->CreateReplyAndComplete( EReplyTypeOk, iReply);
                }
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

TInt CCGMRCommandHandler::GetSoftwareVersion()
    {
    TRACE_FUNC_ENTRY
    iReply.Append( KCRLF );

    // Get software version (e.g. 010.009)
    TBuf<KSysUtilVersionTextLength> swVersion;   
    TInt errorCode = SysUtil::GetSWVersion( swVersion );

    if (KErrNone == errorCode)
        {
        TLex8 lex(swVersion.Collapse());
        lex.SkipCharacters();
        TPtrC8 token = lex.MarkedToken(); 
        iReply.Append(token);
        
        iReply.Append( KCRLF );
        TRACE_FUNC_EXIT
        return EReplyTypeOk;
        }
    else
        {
        TRACE_FUNC_EXIT
        return EReplyTypeError;
        }
    }



