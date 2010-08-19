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

#include "cgmicommandhandler.h"

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCGMICommandHandler* CCGMICommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCGMICommandHandler* self = new (ELeave) CCGMICommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCGMICommandHandler::CCGMICommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdSyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCGMICommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);
    TRACE_FUNC_EXIT
    }

CCGMICommandHandler::~CCGMICommandHandler()
    {
    TRACE_FUNC_ENTRY
    iReply.Close();
    TRACE_FUNC_EXIT
    }

void CCGMICommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
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
                    iReply.Append( iManufacturer );            
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

void CCGMICommandHandler::SetManufacturer(const TDesC8& aManufacturer)
    {
    TRACE_FUNC_ENTRY
    ASSERT( aManufacturer.Length() <= CTelephony::KPhoneModelIdSize );
    iManufacturer.Zero();
    iManufacturer.Copy(aManufacturer);
    TRACE_FUNC_EXIT
    }


void CCGMICommandHandler::SetTelephonyError(TInt aTelError)
    {
    TRACE_FUNC_ENTRY
    iTelError = aTelError;
    TRACE_FUNC_EXIT
    }


