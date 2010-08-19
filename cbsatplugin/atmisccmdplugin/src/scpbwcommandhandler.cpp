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

#include "scpbwcommandhandler.h"
#include <mpbutil.h> 
#include <exterror.h>
#include <etelmmerr.h>

#include "atmisccmdpluginconsts.h"
#include "debug.h"

const TInt KMaxContactEntrySize = 512;
const TInt KMaxTextLength = 64;
const TInt KMaxNumberLength = 64;
const TInt KMaxEmailLength = 64;

CSCPBWCommandHandler* CSCPBWCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CSCPBWCommandHandler* self = new (ELeave) CSCPBWCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CSCPBWCommandHandler::CSCPBWCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdAsyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CSCPBWCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    
    TInt err = iPhoneBookStore.Open(iPhone, KETelIccAdnPhoneBook);
    if (err != KErrNone)
        {
        iState = ESCPBWStateSimStoreNotSupported;
        TRACE_FUNC_EXIT
        return;
        }
    err = iMmCustomAPI.Open(iPhone);
    if (err != KErrNone)
        {
        iPhoneBookStore.Close();
        iState = ESCPBWStateSimStoreNotSupported;
        TRACE_FUNC_EXIT
        return;
        }
    iPhoneBookBuffer = new (ELeave) CPhoneBookBuffer;
    
    iReply.CreateL(KDefaultCmdBufLength);
    iNum1.CreateL(KMaxNumberLength);
    iNum2.CreateL(KMaxNumberLength);
    iNum3.CreateL(KMaxNumberLength);
    iNum4.CreateL(KMaxNumberLength);
    iText.CreateL(KMaxTextLength);
    iEmail.CreateL(KMaxEmailLength);
    iPbData.CreateL(KMaxContactEntrySize);
    
    TRACE_FUNC_EXIT
    }

CSCPBWCommandHandler::~CSCPBWCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    delete iPhoneBookBuffer;
    iPhoneBookStore.Close();
    iMmCustomAPI.Close();
    iPbData.Close();
    iReply.Close();
    iNum1.Close();
    iNum2.Close();
    iNum3.Close();
    iNum4.Close();
    iText.Close();
    iEmail.Close();
    TRACE_FUNC_EXIT
    }

void CSCPBWCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY
    
    if (iState != ESCPBWStateIdle)
        {
        // Reply "ERROR" if handler is not in idle
        iCallback->CreateReplyAndComplete(EReplyTypeError);
        TRACE_FUNC_EXIT
        return;
        }
    TInt err = KErrNone;
    
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            if (iMaxEntries == 0)
                {
                RMobilePhoneBookStore::TMobilePhoneBookInfoV1Pckg pckg(iPhoneBookStoreInfo);
                iPhoneBookStore.GetInfo(iStatus, pckg);
                iState = ESCPBWStateGetPhonebookInfo;
                SetActive();
                }
            else if (iEmailLength == 0)
                {
                iState = ESCPBWStateGet3GPBInfo;
                iMmCustomAPI.Get3GPBInfo(iStatus, i3GPBInfo);
                SetActive();
                }
            else
                {
                // Phonebook info has been obtained
                iReply.Zero();
                iReply.Format(KSCPBWSupportedCmdsList, iMaxEntries, iNumLength, iTextLength, iEmailLength);
                iCallback->CreateReplyAndComplete( EReplyTypeOk, iReply );
                }
            }
            break;
        case (TAtCommandParser::ECmdHandlerTypeSet): 
            {
            ResetParameters();
            TBool isDeleteRequest = EFalse;
            err = ParseParameters(isDeleteRequest);
            Trace(_L("Parse completed, err = %d"), err);
            if (isDeleteRequest)
                {
                // Delete entry at iIndex
                iState = ESCPBWStateDelete;
                iPhoneBookStore.Delete(iStatus, iIndex);
                SetActive();
                }
            else if (err == KErrNone)
                {
                // Create an entry
                err = CreateContactEntry();
                if (err == KErrNone)
                    {
                    iPhoneBookStore.Write(iStatus, iPbData, iIndex);
                    iState = ESCPBWStateWrite;
                    SetActive();
                    }
                else
                    {
                    iCallback->CreateReplyAndComplete( EReplyTypeError );
                    }
                }
            else
                {
                iCallback->CreateReplyAndComplete( EReplyTypeError );
                }
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete( EReplyTypeError );
            break;
            }
        }
    
    TRACE_FUNC_EXIT
    }

void CSCPBWCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    
    iReply.Zero();
    TInt err = iStatus.Int();
    Trace(_L("State = %d, err = %d"), iState, err);
    
    if (err == KErrNone)
        {
        switch (iState)
            {
            case ESCPBWStateGetPhonebookInfo:
                {
                Trace(_L("Get info successful."));
                iNumLength = iPhoneBookStoreInfo.iMaxNumLength;
                iTextLength = iPhoneBookStoreInfo.iMaxTextLength;
                iMaxEntries = iPhoneBookStoreInfo.iTotalEntries;
                if (iEmailLength == 0)
                    {
                    iMmCustomAPI.Get3GPBInfo(iStatus, i3GPBInfo);
                    iState = ESCPBWStateGet3GPBInfo;
                    SetActive();
                    }
                else
                    {
                    iReply.Format(KSCPBWSupportedCmdsList, iMaxEntries, iNumLength, iTextLength, iEmailLength);
                    iCallback->CreateReplyAndComplete(EReplyTypeOk, iReply);
                    iState = ESCPBWStateIdle;
                    }
                }
                break;
            case ESCPBWStateGet3GPBInfo:
                {
                Trace(_L("Get 3GPB info successful."));
                iEmailLength = i3GPBInfo.iMaxLenEmail;
                iReply.Format(KSCPBWSupportedCmdsList, iMaxEntries, iNumLength, iTextLength, iEmailLength);
                iCallback->CreateReplyAndComplete(EReplyTypeOk, iReply);
                iState = ESCPBWStateIdle;
                }
                break;
            case ESCPBWStateWrite:
                {
                Trace(_L("Write successful. Index = %d"), iIndex);
                iCallback->CreateReplyAndComplete(EReplyTypeOk);
                iState = ESCPBWStateIdle;
                }
                break;
            case ESCPBWStateDelete:
                {
                Trace(_L("Delete successful."));
                iCallback->CreateReplyAndComplete(EReplyTypeOk);
                iState = ESCPBWStateIdle;
                }
                break;
            default:
                {
                iState = ESCPBWStateIdle;
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                break;
                }
            }
        }
    else
        {
        iState = ESCPBWStateIdle; 
        iCallback->CreateCMEReplyAndComplete(err);
        }
    
    TRACE_FUNC_EXIT
    }

void CSCPBWCommandHandler::DoCancel() 
    {
    TRACE_FUNC_ENTRY
    
    switch (iState)
        {
        case ESCPBWStateGetPhonebookInfo:
            {
            iPhoneBookStore.CancelAsyncRequest(EMobilePhoneStoreGetInfo);
            }
            break;
        case ESCPBWStateGet3GPBInfo:
            {
            iMmCustomAPI.CancelAsyncRequest(EGet3GPBInfoIPC);
            }
            break;
        case ESCPBWStateDelete:
            {
            iPhoneBookStore.CancelAsyncRequest(EMobilePhoneStoreDelete);
            }
            break;
        case ESCPBWStateWrite:
            {
            iPhoneBookStore.CancelAsyncRequest(EMobilePhoneStoreWrite);
            }
            break;
        }
    iState = ESCPBWStateIdle;
    
    TRACE_FUNC_EXIT
    }

TInt CSCPBWCommandHandler::ParseParameters( TBool& aIsDeleteRequest )
    {
    TRACE_FUNC_ENTRY
    
    TInt ret = KErrNone;
    // Paese index
    ret = iATCmdParser.NextIntParam(iIndex); 
    Trace(_L("Parse index err: %d"), ret);
    Trace(_L("index: %d"), iIndex);
    if (ret != KErrNone && ret != KErrNotFound)
        {
        // Bad index
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    TPtrC8 ptrc;
    // Parse num1
    ptrc.Set(iATCmdParser.NextParam());
    if (ptrc.Length() != 0)
        {
        Trace(_L("Parse num1 OK: %S"), &ptrc);
        SetBuffer(iNum1, ptrc);
        }
    else if (ret == KErrNone )
        {
        // Only index given
        Trace(_L("Only index given."));
        aIsDeleteRequest = ETrue;
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    else 
        {
        // no num1 found
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    ret = iATCmdParser.NextIntParam(iType1);
    if (ret == KErrNotFound)
        {
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    else if (ret != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    // Parse num2
    ptrc.Set(iATCmdParser.NextParam());
    if (ptrc.Length() != 0)
        {
        Trace(_L("Parse num2 OK: %S"), &ptrc);
        SetBuffer(iNum2, ptrc);
        }
    else 
        {
        // no num2 found
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    ret = iATCmdParser.NextIntParam(iType2);
    if (ret == KErrNotFound)
        {
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    else if (ret != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    // Parse num3
    ptrc.Set(iATCmdParser.NextParam());
    if (ptrc.Length() != 0)
        {
        Trace(_L("Parse num3 OK: %S"), &ptrc);
        SetBuffer(iNum3, ptrc);
        }
    else
        {
        // no num3 found
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    ret = iATCmdParser.NextIntParam(iType3);
    if (ret == KErrNotFound)
        {
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    else if (ret != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    // Parse num4
    ptrc.Set(iATCmdParser.NextParam());
    if (ptrc.Length() != 0)
        {
        Trace(_L("Parse num4 OK: %S"), &ptrc);
        SetBuffer(iNum4, ptrc);
        }
    else
        {
        // no num4 found
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    ret = iATCmdParser.NextIntParam(iType4);
    if (ret == KErrNotFound)
        {
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    else if (ret != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    // Parse text
    ptrc.Set(iATCmdParser.NextParam());
    if (ptrc.Length() != 0)
        {
        Trace(_L("Parse text OK: %S"), &ptrc);
        SetBuffer(iText, ptrc);
        }
    else
        {
        // no text found
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    ret = iATCmdParser.NextIntParam(iCoding);
    if (ret == KErrNotFound)
        {
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    else if (ret != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    // Parse email
    ptrc.Set(iATCmdParser.NextParam());
    if (ptrc.Length() != 0)
        {
        Trace(_L("Parse email OK: %S"), &ptrc);
        SetBuffer(iEmail, ptrc);
        }
    else
        {
        // no email found
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    
    if (iATCmdParser.NextParam().Length() != 0)
        {
        // too many parameters
        Trace(_L("Too many parameters."));
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    TRACE_FUNC_EXIT
    return KErrNone;
    }

TInt CSCPBWCommandHandler::CreateContactEntry()
    {
    TRACE_FUNC_ENTRY
    
    TInt err = KErrNone;
    iPhoneBookBuffer->Set(&iPbData);
    // Add new entry tag
    err = iPhoneBookBuffer->AddNewEntryTag();
    Trace(_L("New entry tag added, err = %d"), err);
    if (err != KErrNone)
        {
        TRACE_FUNC_EXIT
        return err;
        }
    // Put index into the entry
    err = iPhoneBookBuffer->PutTagAndValue(RMobilePhoneBookStore::ETagPBAdnIndex, (TUint16)iIndex);
    Trace(_L("Index added, err = %d"), err);
    if (err != KErrNone)
        {
        TRACE_FUNC_EXIT
        return err;
        }
    // Put text into the entry
    if (iText.Length() != 0)
        {
        err = iPhoneBookBuffer->PutTagAndValue(RMobilePhoneBookStore::ETagPBText, iText);
        Trace(_L("Text added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        }
    // Put num1 into the entry
    if (iNum1.Length() != 0)
        {
        err = iPhoneBookBuffer->PutTagAndValue(RMobilePhoneBookStore::ETagPBNumber, iNum1);
        Trace(_L("Number 1 added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        }
    // Put num2 into the entry
    if (iNum2.Length() != 0)
        {
        // Add anr tag 
        err = iPhoneBookBuffer->AddNewNumberTag();
        Trace(_L("New number tag added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        err = iPhoneBookBuffer->PutTagAndValue(RMobilePhoneBookStore::ETagPBNumber, iNum2);
        Trace(_L("Number 2 added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        }
    // Put num3 into the entry
    if (iNum3.Length() != 0)
        {
        // Add anr tag 
        err = iPhoneBookBuffer->AddNewNumberTag();
        Trace(_L("New number tag added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        err = iPhoneBookBuffer->PutTagAndValue(RMobilePhoneBookStore::ETagPBNumber, iNum3);
        Trace(_L("Number 3 added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        }
    // Put num4 into the entry
    if (iNum4.Length() != 0)
        {
        // Add anr tag 
        err = iPhoneBookBuffer->AddNewNumberTag();
        Trace(_L("New number tag added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        err = iPhoneBookBuffer->PutTagAndValue(RMobilePhoneBookStore::ETagPBNumber, iNum4);
        Trace(_L("Number 4 added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        }    
    // Put email address into the entry
    if (iEmail.Length() != 0)
        {
        err = iPhoneBookBuffer->PutTagAndValue(RMobilePhoneBookStore::ETagPBEmailAddress, iEmail);
        Trace(_L("Email added, err = %d"), err);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        }
    
    TRACE_FUNC_EXIT
    return KErrNone;
    }

void CSCPBWCommandHandler::ResetParameters()
    {
    TRACE_FUNC_ENTRY
    
    iPbData.Zero();
    iIndex = -1;
    iNum1.Zero();
    iType1 = 0x91; // International & ISDN
    iNum2.Zero();
    iType2 = 0x91;
    iNum3.Zero();
    iType3 = 0x91;
    iNum4.Zero();
    iType4 = 0x91;
    iText.Zero();
    iCoding = 0; // GSM 7 bit
    iEmail.Zero();
    
    TRACE_FUNC_EXIT
    }

void CSCPBWCommandHandler::SetBuffer(TDes& aDest, const TDesC8& aSource)
    {
    TRACE_FUNC_ENTRY
    TInt maxLength = aDest.MaxLength();
    if (aSource.Length() <= maxLength)
        {
        aDest.Copy(aSource);
        }
    else
        {
        aDest.Copy(aSource.Left(maxLength));
        }
    TRACE_FUNC_EXIT
    }


