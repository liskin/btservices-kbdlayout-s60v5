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

#include "cgmmcommandhandler.h"

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCGMMCommandHandler* CCGMMCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCGMMCommandHandler* self = new (ELeave) CCGMMCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCGMMCommandHandler::CCGMMCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdSyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCGMMCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);
    TRACE_FUNC_EXIT
    }

CCGMMCommandHandler::~CCGMMCommandHandler()
    {
    TRACE_FUNC_ENTRY
    iReply.Close();
    TRACE_FUNC_EXIT
    }

void CCGMMCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
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
                    _LIT8( KSpace, " " );
                
                    iReply.Append( KCRLF );
                    iReply.Append( iManufacturer );
                    iReply.Append( KSpace );
                    iReply.Append( iModel );    
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

void CCGMMCommandHandler::SetManufacturer(const TDesC8& aManufacturer)
    {
    TRACE_FUNC_ENTRY
    iManufacturer.Zero();
    if (aManufacturer.Length() <= CTelephony::KPhoneManufacturerIdSize)
        {
        iManufacturer.Copy(aManufacturer);
        }
    else
        {
        iManufacturer.Copy(aManufacturer.Left(CTelephony::KPhoneManufacturerIdSize));
        }   
    TRACE_FUNC_EXIT
    }

void CCGMMCommandHandler::SetModelID(const TDesC8& aModelID)
    {
    TRACE_FUNC_ENTRY
    iModel.Zero();
    if (aModelID.Length() <= CTelephony::KPhoneModelIdSize)
        {
        iModel.Copy(aModelID);
        }
    else
        {
        iModel.Copy(aModelID.Left(CTelephony::KPhoneModelIdSize));
        }    
    TRACE_FUNC_EXIT
    }


void CCGMMCommandHandler::SetTelephonyError(TInt aTelError)
    {
    TRACE_FUNC_ENTRY
    iTelError = aTelError;
    TRACE_FUNC_EXIT
    }


