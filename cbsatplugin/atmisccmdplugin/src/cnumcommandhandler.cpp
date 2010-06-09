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
// This file contains the implementation of the AT+CNUM command
// 
//

#include "cnumcommandhandler.h"

#include <mmretrieve.h>

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCNUMCommandHandler* CCNUMCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser,
                                                RMobilePhone& aPhone, RTelServer& aTelServer)
    {
    TRACE_FUNC_ENTRY
    CCNUMCommandHandler* self = new (ELeave) CCNUMCommandHandler(aCallback, aATCmdParser, aPhone, aTelServer );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCNUMCommandHandler::CCNUMCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser,
                                         RMobilePhone& aPhone, RTelServer& aTelServer) :
        CATCmdAsyncBase(aCallback, aATCmdParser, aPhone),
        iTelServer(aTelServer),
        iOwnStoreInfoPckg(iOwnStoreInfo),
        iOwnNumberEntryPckg(iOwnNumberEntry)
    {
    }

void CCNUMCommandHandler::ConstructL()
    {
    iReplyBuffer.CreateL(KDefaultCmdBufLength);
    }

CCNUMCommandHandler::~CCNUMCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iReplyBuffer.Close();
    iOwnNumberStore.Close();
    TRACE_FUNC_EXIT
    }

void CCNUMCommandHandler::HandleCommand( const TDesC8& /*aCmd*/,
                                   RBuf8& /*aReply*/,
                                   TBool /*aReplyNeeded */)
    {
    TRACE_FUNC_ENTRY
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeBase):
            {
            // Handle AT+CNUM
            // step 1 check if the phone support Own Number Store
            //  and open the store if supported
           
            iReplyBuffer.Zero();
            // only open a subsession to OwnNumberStore when necessary
            if(iOwnNumberStore.SubSessionHandle() == 0)
                {
                TName tsyName;
                TInt ret = iTelServer.GetTsyName(0, tsyName);
                if(ret != KErrNone)
                    {
                    Trace(KDebugPrintD, "GetTsyName returned error ", ret);
                    iCallback->CreateReplyAndComplete(EReplyTypeError);
                    break;
                    }
                
                TBool isSupported = EFalse;
                // TODO: isSupported should be checked
                ret = iTelServer.IsSupportedByModule(tsyName, KETelFuncMobileOwnNumberStore, isSupported);
                if(ret != KErrNone)
                    {
                    Trace(KDebugPrintD, "IsSupportedByModule returned error ", ret);
                    iCallback->CreateReplyAndComplete(EReplyTypeError);
                    break;
                    }
                
                Trace(KDebugPrintD, "before open OwnNumberstore", 0);
                ret = iOwnNumberStore.Open(iPhone);
                if(ret != KErrNone)
                    {
                    Trace(KDebugPrintD, "Open OwnNumberStore returned error ", ret);
                    iCallback->CreateReplyAndComplete(EReplyTypeError);
                    break;
                    }
                }
            // step 2 Get number of own number entries stored.
            iOwnNumberStore.GetInfo(iStatus, iOwnStoreInfoPckg);
            iPendingEvent = EMobilePhoneStoreGetInfo;
            SetActive();
             
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            // Handle AT+CNUM=?
            iCallback->CreateReplyAndComplete( EReplyTypeOk );
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

void CCNUMCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    if(iStatus.Int() != KErrNone)
        {
        Trace(KDebugPrintD, "Pending event id: ", iPendingEvent);
        iCallback->CreateCMEReplyAndComplete(iStatus.Int());
       }
    else if(iPendingEvent == EMobilePhoneStoreGetInfo)
        { // step 3 check the number of entries
        Trace(KDebugPrintD, "Number of Owner's numbers: ", iOwnStoreInfo.iUsedEntries);
        if(iOwnStoreInfo.iUsedEntries < 1)
            {
            // complete with OK if no owner's number
            iCallback->CreateReplyAndComplete(EReplyTypeOk);
            }
        else
            {
            // step 4 Read the first owner's number entry
            iOwnNumberEntry.iIndex = 0;
            iOwnNumberStore.Read(iStatus,iOwnNumberEntryPckg);
            iPendingEvent = EMobilePhoneStoreRead;
            SetActive();
            }
        }
    else
        {
        ASSERT(iPendingEvent == EMobilePhoneStoreRead);
        // step 5 append a phone number entry to the reply buffer
        if(iOwnNumberEntry.iNumber.iTelNumber.Length() != 0)
            {
            // 129 is the type of address octet in interger format
            _LIT8(KATNumReply, "\r\n+CNUM: ,\"%S\",129");
            TBuf8<RMobilePhone::KMaxMobileTelNumberSize > telNumber8;
            telNumber8.Copy(iOwnNumberEntry.iNumber.iTelNumber);
            iReplyBuffer.AppendFormat(KATNumReply, &telNumber8);
            }
        // check if there is any more owner's number
        if(iOwnNumberEntry.iIndex < iOwnStoreInfo.iUsedEntries)
            { // step 6 read next entry 
            ++iOwnNumberEntry.iIndex;
            Trace(KDebugPrintD, "Next number id", iOwnNumberEntry.iIndex);
            iOwnNumberStore.Read(iStatus,iOwnNumberEntryPckg);
            iPendingEvent = EMobilePhoneStoreRead;
            SetActive();
            }
        else
            { // final step: all entries are retrieved. send them back to the AT server
            iReplyBuffer.Append(KOKCRLF);
            iCallback->CreateReplyAndComplete(EReplyTypeOther, iReplyBuffer);
            }
        }

    TRACE_FUNC_EXIT
    }

void CCNUMCommandHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iCallback->CreateReplyAndComplete(EReplyTypeError);
    TRACE_FUNC_EXIT
    }
