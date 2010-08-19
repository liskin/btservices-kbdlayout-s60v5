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

#include "cmgdcommandhandler.h"
#include <mmtsy_names.h> 
#include <mmretrieve.h>
#include <exterror.h>

#include "atmisccmdpluginconsts.h"
#include "debug.h"

const TInt KDefaultListLength = 256;

CCMGDCommandHandler* CCMGDCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCMGDCommandHandler* self = new (ELeave) CCMGDCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCMGDCommandHandler::CCMGDCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdAsyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY

    TRACE_FUNC_EXIT
    }

void CCMGDCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    
    iReply.CreateL(KDefaultCmdBufLength);
    
    User::LeaveIfError( iMobileSmsMessaging.Open(iPhone) );

    TInt err = iMobileSmsStore.Open(iMobileSmsMessaging, KETelIccSmsStore);
    if (err != KErrNone)
        {
        iHandlerState = ECMGDStateSimStoreNotSupported;
        }
    
    iRetrieveSmsList = CRetrieveMobilePhoneSmsList::NewL(iMobileSmsStore, RMobileSmsStore::KETelMobileGsmSmsEntryV1);
    
    TRACE_FUNC_EXIT
    }

CCMGDCommandHandler::~CCMGDCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    delete iGsmSmsList;
    delete iRetrieveSmsList;
    iMobileSmsStore.Close();
    iMobileSmsMessaging.Close();
    iReply.Close();
    iDeleteList.Close();
    TRACE_FUNC_EXIT
    }

void CCMGDCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY
    
    if (iHandlerState == ECMGDStateSimStoreNotSupported)
        {
        // Reply "ERROR" if SIM store is not supported
        iCallback->CreateReplyAndComplete(EReplyTypeError);
        TRACE_FUNC_EXIT
        return;
        }
    
    TInt ret = KErrNone;
    
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            iRetrieveSmsList->Start(iStatus);
            iHandlerState = ECMGDStateRetrieveAllIndices;
            SetActive();
            }
            break;
        case (TAtCommandParser::ECmdHandlerTypeSet):
            {
            ret = ParseParameters();
            if (ret == KErrNone)
                {
                Trace(_L("index = %d"), iIndex);
                Trace(_L("delflag = %d"), iDelFlag);
                ret = DoHandleCommand();
                }
            else
                {
                ret = KErrArgument;
                }
            break;
            }
        default:
            {
            ret = KErrNotSupported;
            break;
            }
        }
    if (ret == KErrGsmSMSInvalidMemoryIndex)
        {
        iCallback->CreateCMSReplyAndComplete(ret);
        }
    else if (ret != KErrNone)
        {
        iCallback->CreateReplyAndComplete(EReplyTypeError);
        }

    TRACE_FUNC_EXIT
    }

void CCMGDCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    
    Trace(_L("Error = %d"), iStatus.Int());
    Trace(_L("State = %d"), iHandlerState);
    
    iReply.Zero();
    
    TInt err = iStatus.Int();
    if(err == KErrNone)
        {
        switch (iHandlerState)
            {
            case ECMGDStateRetrieveAllIndices:
                {
                GetMessageIndexListL();
                iDeleteList.Reset();
                break;
                }
            case ECMGDStateDeleteOneEntry:                
            case ECMGDStateDeleteAllEntries:
                {
                // do nothing
                break;
                }
            case ECMGDStateDeleteFilteredEntries:
                {
                FilterMessageListL();
                break;
                }
            default:
                {
                iHandlerState = ECMGDStateIdle;
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                TRACE_FUNC_EXIT
                return;
                }
            }
        
        if (iDeleteList.Count() != 0)
            {
            // delete the first entry in the list
            iMobileSmsStore.Delete(iStatus, iDeleteList[0]);
            iDeleteList.Remove(0);
            iHandlerState = ECMGDStateDeleteOneEntry;
            SetActive();                    
            }
        else
            {
            // deleting finished
            iCallback->CreateReplyAndComplete(EReplyTypeOk, iReply);
            }
        }
    else if (err == KErrNotFound)
        {
        // no message found in SIM
        switch (iHandlerState)
            {
            case ECMGDStateRetrieveAllIndices:
                {
                iHandlerState = ECMGDStateIdle;
                iReply.Format(KCMGDSupportedCmdsList, &KNullDesC8);
                iCallback->CreateReplyAndComplete(EReplyTypeOk, iReply);
                break;
                }
            case ECMGDStateDeleteFilteredEntries:
                {
                iMobileSmsStore.DeleteAll(iStatus);
                iHandlerState = ECMGDStateDeleteAllEntries;
                SetActive();
                break;
                }
            default:
                iHandlerState = ECMGDStateIdle;
                iCallback->CreateCMSReplyAndComplete(err);
                break;
            }
        }
    else 
        {
        iHandlerState = ECMGDStateIdle;
        iCallback->CreateCMSReplyAndComplete(err);
        }

    TRACE_FUNC_EXIT
    }

TInt CCMGDCommandHandler::RunError(TInt aError)
    {
    TRACE_FUNC_ENTRY
    
    iCallback->CreateCMSReplyAndComplete(aError);
    
    TRACE_FUNC_EXIT
    return KErrNone;
    }

void CCMGDCommandHandler::DoCancel() 
    {
    TRACE_FUNC_ENTRY
    
    switch (iHandlerState)
        {
        case ECMGDStateRetrieveAllIndices:
        case ECMGDStateDeleteFilteredEntries:
            {
            iRetrieveSmsList->Cancel();
            break;
            }
        case ECMGDStateDeleteOneEntry:
            {
            iMobileSmsStore.CancelAsyncRequest(EMobilePhoneStoreDelete);
            break;
            }
        case ECMGDStateDeleteAllEntries:
            {
            iMobileSmsStore.CancelAsyncRequest(EMobilePhoneStoreDeleteAll);
            break;
            }
        }

    TRACE_FUNC_EXIT
    }

TInt CCMGDCommandHandler::ParseParameters()
    {
    TRACE_FUNC_ENTRY
    
    TInt ret = KErrNone;
    TInt index = 0;
    TInt delflag = 0; // default value
    TInt otherParams = 0;
    
    // get index 
    TInt retIndex = iATCmdParser.NextIntParam(index);
    // get delflag
    TInt retDelflag = iATCmdParser.NextIntParam(delflag);
    // get other params
    TInt retOther = iATCmdParser.NextIntParam(otherParams);
    // Syntax error happens if
    // a)there is no param2 or param2 error 
    // b)there is no param1 but param2==0
    // c)there are too many params
    TBool noParam = (retIndex!=KErrNone && retDelflag!=KErrNone || retDelflag==KErrGeneral );
    TBool noParam1 = (retIndex!=KErrNone && retDelflag==KErrNone && delflag==0);
    TBool tooManyParams = (retOther!=KErrNotFound);
    
    if (noParam || noParam1 || tooManyParams)
        {
        ret = KErrArgument;
        }
    else
        {
        iIndex = index;
        iDelFlag = delflag;
        }

    TRACE_FUNC_EXIT
    return ret;
    }

TInt CCMGDCommandHandler::DoHandleCommand()
    {
    TRACE_FUNC_ENTRY
    
    TInt err = KErrNone;
    
    switch (iDelFlag)
        {
        case 0:
            // deletes the designated message with <index>.
            {
            if (iIndex == 0)
                {
                // The first position in SIM store is 1,
                // so zero is not in the supported range.
                err = KErrGsmSMSInvalidMemoryIndex;
                }
            else
                {
                iDeleteList.Reset();
                iMobileSmsStore.Delete(iStatus, iIndex);
                iHandlerState = ECMGDStateDeleteOneEntry;
                SetActive();
                }
            }
            break;
        case 1:
            // deletes all read message in first-choice memory and reserves unread message, 
            // sent message and unsent message.
            {
            iRetrieveSmsList->Start(iStatus);
            iHandlerState = ECMGDStateDeleteFilteredEntries;
            iFilterType = ECMGDFilterReadMessagesOnly;
            SetActive();
            }
            break;
        case 2:
            // deletes all read message and sent message in first-choice memory 
            // and reserves unread message and unsent message.
            {
            iRetrieveSmsList->Start(iStatus);
            iHandlerState = ECMGDStateDeleteFilteredEntries;
            iFilterType = ECMGDFilterReadAndSentMessages;
            SetActive();            
            }
            break;
        case 3:
            // deletes all read message, sent message and unsent message in first-choice memory 
            // and reserves unread message.
            {
            iRetrieveSmsList->Start(iStatus);
            iHandlerState = ECMGDStateDeleteFilteredEntries;
            iFilterType = ECMGDFilterReadSentAndUnsentMessages;
            SetActive();            
            }
            break;
        case 4:
            // deletes all messages in first-choice memory, including unread message.
            {
            iDeleteList.Reset();
            iMobileSmsStore.DeleteAll(iStatus);
            iHandlerState = ECMGDStateDeleteAllEntries;
            SetActive();
            }
            break;
            
        default:
            err = KErrNotSupported;
            break;
        }
    
    TRACE_FUNC_EXIT
    return err;
    }

void CCMGDCommandHandler::GetMessageIndexListL()
    {
    TRACE_FUNC_ENTRY
    
    iReply.Zero();
    delete iGsmSmsList;
    iGsmSmsList = NULL;
    
    RBuf8 list;
    list.CreateL(KDefaultListLength);
    CleanupClosePushL(list);

    iGsmSmsList = iRetrieveSmsList->RetrieveGsmListL();
    TInt entriesNum = iGsmSmsList->Enumerate();
    
    if (entriesNum > 0)
        {
        // Append 1st item
        RMobileSmsStore::TMobileGsmSmsEntryV1 entry;
        entry = iGsmSmsList->GetEntryL(0);
        list.AppendNum(entry.iIndex);
        
        for (TInt i=1; i<entriesNum; ++i)
            {
            entry = iGsmSmsList->GetEntryL(i);
            list.Append(',');
            list.AppendNum(entry.iIndex);
            }
        }
    iReply.Format(KCMGDSupportedCmdsList, &list);
    
    CleanupStack::PopAndDestroy(&list); //list
    
    TRACE_FUNC_EXIT
    }

/**
 * Filter message list with iFilterType
 * ECMGDFilterReadSentAndUnsentMessages: read, sent and unsent messages
 * ECMGDFilterReadAndSentMessages: read and sent messages
 * ECMGDFilterReadMessagesOnly: read messages
 */
void CCMGDCommandHandler::FilterMessageListL()
    {
    TRACE_FUNC_ENTRY
    
    delete iGsmSmsList;
    iGsmSmsList = NULL;
        
    iDeleteList.Reset();
    
    iGsmSmsList = iRetrieveSmsList->RetrieveGsmListL();
    TInt entriesNum = iGsmSmsList->Enumerate();
    
    for (TInt i=0; i<entriesNum; ++i)
        {
        RMobileSmsStore::TMobileGsmSmsEntryV1 entry = iGsmSmsList->GetEntryL(i);
        switch (iFilterType)
            {
            case ECMGDFilterReadSentAndUnsentMessages:
                {
                // add unsent message 
                if (entry.iMsgStatus == RMobileSmsStore::EStoredMessageUnsent)
                    {
                    iDeleteList.AppendL(entry.iIndex);
                    break;
                    }
                }
            case ECMGDFilterReadAndSentMessages:
                {
                // add sent message 
                if (entry.iMsgStatus == RMobileSmsStore::EStoredMessageSent)
                    {
                    iDeleteList.AppendL(entry.iIndex);
                    break;
                    }
                }
            case ECMGDFilterReadMessagesOnly:
                {
                // add read message 
                if (entry.iMsgStatus == RMobileSmsStore::EStoredMessageRead )
                    {
                    iDeleteList.AppendL(entry.iIndex);
                    break;
                    }
                }
            }
        }
    
    TRACE_FUNC_EXIT
    }
