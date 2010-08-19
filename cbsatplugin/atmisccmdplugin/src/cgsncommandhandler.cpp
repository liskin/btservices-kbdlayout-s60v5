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

#include "cgsncommandhandler.h"

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCGSNCommandHandler* CCGSNCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCGSNCommandHandler* self = new (ELeave) CCGSNCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCGSNCommandHandler::CCGSNCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdSyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCGSNCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iTelError = KErrNone;
    iReply.CreateL(KDefaultCmdBufLength);
    TRACE_FUNC_EXIT
    }

CCGSNCommandHandler::~CCGSNCommandHandler()
    {
    TRACE_FUNC_ENTRY
    iReply.Close();
    TRACE_FUNC_EXIT
    }

void CCGSNCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
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
            if(iTelError == KErrNone)
                {
                if (iReply.Length() == 0)
                    {
                    iReply.Append( KCRLF );
                    iReply.Append( iSN );     
                    iReply.Append( KCRLF );
                    }
              
                iCallback->CreateReplyAndComplete( EReplyTypeOk, iReply);
                }
            else
                {
                iCallback->CreateCMEReplyAndComplete(iTelError);
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

void CCGSNCommandHandler::SetSerialNum(const TDesC8& aSerial)
    {
    TRACE_FUNC_ENTRY
    ASSERT( aSerial.Length() <= CTelephony::KPhoneSerialNumberSize );
    iSN.Zero();
    iSN.Copy(aSerial);
    TRACE_FUNC_EXIT
    }

void CCGSNCommandHandler::SetTelephonyError(TInt aTelError)
    {
    TRACE_FUNC_ENTRY
    iTelError = aTelError;
    TRACE_FUNC_EXIT
    }


