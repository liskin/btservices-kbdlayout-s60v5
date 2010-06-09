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

#include "cfuncommandhandler.h"

#include <MProfileEngine.h>
#include <starterclient.h>
#include <ssm/startupdomainpskeys.h>

#include <Profile.hrh>

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCFUNCommandHandler* CCFUNCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCFUNCommandHandler* self = new (ELeave) CCFUNCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCFUNCommandHandler::CCFUNCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdAsyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCFUNCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);
    iProfileEngine = CreateProfileEngineL();
    TRACE_FUNC_EXIT
    }

CCFUNCommandHandler::~CCFUNCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iProperty.Close();
    if (iProfileEngine != NULL)
        {
        iProfileEngine->Release();
        }
    TRACE_FUNC_EXIT
    }

void CCFUNCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY
    
    if (IsActive())
        {
        iCallback->CreateReplyAndComplete(EReplyTypeError);
        TRACE_FUNC_EXIT
        return;
        }
    
    TInt ret = KErrNone;
    iReply.Zero();
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOther, KCFUNSupportedCmdsList);
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeRead):
            {
            TInt profileId = iProfileEngine->ActiveProfileId();
            
            iReply.Append( KAtCFUN );
    
            if( profileId == EProfileOffLineId )
                {
                iReply.Append( '0' );
                }
            else
                {
                iReply.Append( '1' );
                }
            iReply.Append( KOKCRLF );
            iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply);
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeSet):
            {
            TInt func = 0;

            ret = iATCmdParser.NextIntParam(func);
            if (ret != KErrNone && ret != KErrNotFound)
                {
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                TRACE_FUNC_EXIT
                return;
                }
			iReset = 0; // default 0 - do not reset the MT before setting it to <fun> power level
            ret = iATCmdParser.NextIntParam(iReset);
            
            // second parameter is optional, but only 0 and 1 are valid if it is specified
            if (!(ret == KErrNone || ret == KErrNotFound) || !(iReset == 0 || iReset == 1))
                {
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                TRACE_FUNC_EXIT
                return;
                }
            
            ret = ActivateProfile(func);       
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            break;
            }
        }
    
    if (!IsActive())
        {
        if (ret != KErrNone)
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            }
        else
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply );
            }
        }
    TRACE_FUNC_EXIT
    }

void CCFUNCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    TInt systemState;
    
    TInt ret = iProperty.Get(systemState);
    if (ret != KErrNone)
        {
        if (systemState != iExpectedState)
            {
            iProperty.Subscribe(iStatus);
            SetActive();
            }
        else if (iReset == 1)
            {
            ret = RestartDevice();
            }
        }
    if (!IsActive())
        {
        if (ret != KErrNone)
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            }
        else
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply );
            }
        }
    TRACE_FUNC_EXIT
    }

void CCFUNCommandHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iProperty.Cancel();
    TRACE_FUNC_EXIT
    }

TInt CCFUNCommandHandler::ActivateProfile(TInt aFunc)
    {
    TRACE_FUNC_ENTRY
    TInt systemState;
    
    //Listen to the property KPSGlobalSystemState for profile change.
    TInt err = iProperty.Attach(KPSUidStartup, KPSGlobalSystemState);
    if (err != KErrNone)
        {
        TRACE_FUNC_EXIT
        return err;
        }

    err = iProperty.Get(systemState);
    if (err != KErrNone)
        {
        TRACE_FUNC_EXIT
        return err;
        }
    
    switch (aFunc)
        {
        case (0):
        case (4):
            {
            // check of KPSGlobalSystemState is not already set to ESwStateNormalRfOff,
            // issue the profile change request and start monitoring the property
            if (systemState != ESwStateNormalRfOff)
                {
                err = SetActiveProfile(KOfflineProfileId);
                
                
                if (err == KErrNone)
                    {
                    iExpectedState = ESwStateNormalRfOff;
                    iProperty.Subscribe(iStatus);
                    SetActive();
                    }           
                }
            break;
            }
        case (1):
            {
            // check of KPSGlobalSystemState is not already set to ESwStateNormalRfOn,
            // issue the profile change request and start monitoring the property
            if (systemState != ESwStateNormalRfOn)
                {
                err = SetActiveProfile(KGeneralProfileId);
                
                if (err == KErrNone)
                    {
                    iExpectedState = ESwStateNormalRfOn;
                    iProperty.Subscribe(iStatus);
                    SetActive();
                    }
                }
            break;
            }
        default:
            {
            err = KErrArgument;
            break;
            }
        }
    TRACE_FUNC_EXIT
    return err;
    }

TInt CCFUNCommandHandler::SetActiveProfile(TInt aProfileId)
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;

    if(iProfileEngine)
        {
        TRAP(err, iProfileEngine->SetActiveProfileL( aProfileId ));
        }
    TRACE_FUNC_EXIT
    return err;
    }

TInt CCFUNCommandHandler::RestartDevice()
    {
    TRACE_FUNC_ENTRY
    RStarterSession session;
    TInt err = session.Connect();

    if( err == KErrNone )
        {
        session.Reset(RStarterSession::EUnknownReset);
        session.Close();
        }
    TRACE_FUNC_EXIT
    return err;
    }
