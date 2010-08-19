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

#include "scpbrcommandhandler.h"

#include <mpbutil.h>
#include "debug.h"
#include "atmisccmdpluginconsts.h"

CSCPBRCommandHandler::CSCPBRCommandHandler( 
		MATMiscCmdPlugin* aCallback, 
		TAtCommandParser& aATCmdParser, 
		RMobilePhone& aPhone) 
		: CATCmdAsyncBase( aCallback, aATCmdParser, aPhone )
	{
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
	}

CSCPBRCommandHandler::~CSCPBRCommandHandler()
	{
    TRACE_FUNC_ENTRY
    Cancel();
    iPhoneBookStore.Close();
    iCustomApi.Close();
    iReply.Close();
    delete iPhoneBookBuffer;
    iEntries.Close();
    iContactsBuf.Close();
    TRACE_FUNC_EXIT
	}

void CSCPBRCommandHandler::ConstructL()
	{
    TRACE_FUNC_ENTRY
    TInt err = iPhoneBookStore.Open( iPhone, KETelIccAdnPhoneBook);
    if( err != KErrNone )  // if the phonebookstore can't be opened or error occurs when try to open it.
        {
        iState = ESCPBRStateNotSupported;
        TRACE_FUNC_EXIT
        return;
        }
    err = iCustomApi.Open( iPhone );
    if( err != KErrNone )  // If the custom api can't be opened.
        {
        iPhoneBookStore.Close();
        iState = ESCPBRStateNotSupported;
        TRACE_FUNC_EXIT
        return;
        }
    iReply.CreateL( KBufBlockSize );
	iPhoneBookBuffer = new ( ELeave ) CPhoneBookBuffer();	
    TRACE_FUNC_EXIT
	}

CSCPBRCommandHandler* CSCPBRCommandHandler::NewL( 
		MATMiscCmdPlugin* aCallback, 
		TAtCommandParser& aATCmdParser, 
		RMobilePhone& aPhone )
	{
    TRACE_FUNC_ENTRY
	CSCPBRCommandHandler * self = new ( ELeave ) CSCPBRCommandHandler( aCallback, aATCmdParser, aPhone );
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
    TRACE_FUNC_EXIT
	return self;
	}

void CSCPBRCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/ )
	{
    TRACE_FUNC_ENTRY
    if( iState == ESCPBRStateIdle )  // Not supported or have unfinished task.
        {
        iReply.Zero();  
        TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
        switch( cmdHandlerType )
            {
            case TAtCommandParser::ECmdHandlerTypeTest: // command: AT^SCPBR=?
                {
                if( iTotalEntries == 0 )
                    {
                    iState = ESCPBRStateGetPhoneBookInfo;
                    RMobilePhoneBookStore::TMobilePhoneBookInfoV1Pckg pkg( iPhoneBookInfo );
                    iPhoneBookStore.GetInfo( iStatus, pkg );
                    SetActive();
                    }
                else if ( iMLength ==0 )
                    {
                    iState = ESCPBRStateGet3GPBInfo;
                    iCustomApi.Get3GPBInfo( iStatus, i3GPBInfo );
                    SetActive();
                    }
                else
                    {
                    iReply.Format( KSCPBRSupportedEntriesIndexList, 
                            iTotalEntries, iNLength, iTLength, iMLength );
                    iCallback->CreateReplyAndComplete( EReplyTypeOk, iReply );
                    }
                break;
                }
            case TAtCommandParser::ECmdHandlerTypeSet:  // command: AT^SCPBR=<index1>[,<index2>]
                {
                if( ParseParameters() == KErrNone )
                    {
                    iEntries.Reset();
                    TInt amountOfEntries = iIndex2 - iIndex1 + 1;  // the amount of entries.
                    Trace( _L8("The amount of entries: %d "), amountOfEntries );
                    iContactsBuf.Zero();
                    TInt bufSize;
                    if( amountOfEntries >= KSCPBRMaxEntryCount )
                        {
                        bufSize= KSCPBRMaxEntryCount * KSCPBRDefaultSizeOfOneEntry;
                        amountOfEntries = KSCPBRMaxEntryCount;
                        }
                    else
                        {
                        bufSize = amountOfEntries * KSCPBRDefaultSizeOfOneEntry;
                        }
                    
                    if( bufSize >= iContactsBuf.MaxLength() )
                        {
                        TInt err = iContactsBuf.ReAlloc( bufSize );
                        Trace( _L8("The error code of the realloc: %d "), err );
                        if( err != KErrNone )
                            {
                            iState = ESCPBRStateIdle;
                            iCallback->CreateReplyAndComplete( EReplyTypeError );
                            break;
                            }
                        }
                    iState = ESCPBRStateRead;
                    iPhoneBookStore.Read( iStatus,  iIndex1, amountOfEntries, iContactsBuf );
                    SetActive();
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
        }
    else
        {
        iCallback->CreateReplyAndComplete( EReplyTypeError );
        } 
       TRACE_FUNC_EXIT
	}

TInt CSCPBRCommandHandler::ParseParameters()
    {
    TRACE_FUNC_ENTRY
    iIndex1 = 0; 
    iIndex2 = 0;
    TInt index3;
    TInt ret1 = iATCmdParser.NextIntParam( iIndex1 );
    TInt ret2 = iATCmdParser.NextIntParam( iIndex2 );
    TInt ret3 = iATCmdParser.NextIntParam( index3 );
    
    if( ret1 != KErrNone 
            || ( ret2 != KErrNone && ret2 != KErrNotFound )
            || ret3 != KErrNotFound )
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    if( ret2 == KErrNotFound )
        {
        iIndex2 = iIndex1;
        }
    if (iIndex2 < iIndex1)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    TRACE_FUNC_EXIT
    return KErrNone;
    }

void CSCPBRCommandHandler::RunL()
	{
    TRACE_FUNC_ENTRY
    iReply.Zero();
    TInt result = iStatus.Int();
    if( result == KErrNone )
        {
        switch( iState )
            {
            case ESCPBRStateRead:
                {
                ExtractEntriesL();
                FormatReplyL();
                iState = ESCPBRStateIdle;
                iCallback->CreateReplyAndComplete(EReplyTypeOk, iReply);
                break;
                }
            case ESCPBRStateGetPhoneBookInfo:
                {
                iTotalEntries = iPhoneBookInfo.iTotalEntries;
                iNLength = iPhoneBookInfo.iMaxNumLength;
                iTLength = iPhoneBookInfo.iMaxTextLength;
                if (iMLength == 0)
                    {
                    iState = ESCPBRStateGet3GPBInfo;
                    iCustomApi.Get3GPBInfo( iStatus, i3GPBInfo );
                    SetActive();
                    }
                else 
                    {
                    iReply.Format( KSCPBRSupportedEntriesIndexList, 
                            iTotalEntries, iNLength, iTLength, iMLength );
                    iState = ESCPBRStateIdle;
                    iCallback->CreateReplyAndComplete( EReplyTypeOk, iReply );
                    }
                break;
                }
            case ESCPBRStateGet3GPBInfo:
                {
                iMLength = i3GPBInfo.iMaxLenEmail;
                iReply.Format( KSCPBRSupportedEntriesIndexList, 
                        iTotalEntries, iNLength, iTLength, iMLength );
                iState = ESCPBRStateIdle;
                iCallback->CreateReplyAndComplete( EReplyTypeOk, iReply );
                break;
                }
            default: 
                {
                iState = ESCPBRStateIdle;
                iCallback->CreateReplyAndComplete( EReplyTypeError );
                break;
                }
            }
        }
    else
        {
        iState = ESCPBRStateIdle;
        iCallback->CreateCMEReplyAndComplete( result );
        }
    TRACE_FUNC_EXIT
	}

TInt CSCPBRCommandHandler::RunError(TInt aError)
    {
    TRACE_FUNC_ENTRY
    iState = ESCPBRStateIdle;
    iCallback->CreateReplyAndComplete( EReplyTypeError );
    TRACE_FUNC_EXIT
    return KErrNone;
    }

void CSCPBRCommandHandler::ExtractEntriesL()
    {
    TRACE_FUNC_ENTRY
    iPhoneBookBuffer->Set( &iContactsBuf );
    iPhoneBookBuffer->StartRead();
    TUint8 fieldTag;
    CPhoneBookBuffer::TPhBkTagType dataType;
    TInt numCount = 0;
	// clear entry arry for use.
    iEntries.Reset();
    User::LeaveIfError(iPhoneBookBuffer->GetTagAndType( fieldTag, dataType ));
    if( fieldTag != RMobilePhoneBookStore::ETagPBNewEntry )
        {
        // Buffer corrupt
        User::Leave(KErrCorrupt);
        }
    TInt ret = KErrNone;
    TInt index = -1; // at least one entry
    while (ret != KErrNotFound) // if reach the buffer end.
        {
        switch (fieldTag)
            {
            case RMobilePhoneBookStore::ETagPBNewEntry:
                {
                TPhoneBookEntry entry;
                iEntries.AppendL(entry);
                ++index;
                numCount = 0; // set the number count of new entry to 0. 
                break;
                }
            case RMobilePhoneBookStore::ETagPBAdnIndex:
                {
                TUint16 simIndex = 0;
                if (dataType != CPhoneBookBuffer::EPhBkTypeInt16)
                    {
                    User::Leave(KErrCorrupt);
                    }
                User::LeaveIfError( iPhoneBookBuffer->GetValue(simIndex) );

                iEntries[index].iIndex = simIndex;
                break;
                }
            case RMobilePhoneBookStore::ETagPBText:
                {
                TPtrC16 text;
                if (dataType != CPhoneBookBuffer::EPhBkTypeDes16)
                    {
                    User::Leave(KErrCorrupt);
                    }
                User::LeaveIfError( iPhoneBookBuffer->GetValue(text) );
                CopyToPhonebookEntryField(iEntries[index].iName, text);
                break;
                }
            case RMobilePhoneBookStore::ETagPBNumber:
                {
                TPtrC16 number;
                if (dataType != CPhoneBookBuffer::EPhBkTypeDes16)
                    {
                    User::Leave(KErrCorrupt);
                    }
                User::LeaveIfError( iPhoneBookBuffer->GetValue(number) );
                ++numCount;
                switch (numCount)
                    {
                    case 1: // The first number
                        CopyToPhonebookEntryField(iEntries[index].iNumber1, number);
                        break;
                    case 2: // The Second number
                        CopyToPhonebookEntryField(iEntries[index].iNumber2, number);
                        break;
                    case 3: // the Third number
                        CopyToPhonebookEntryField(iEntries[index].iNumber3, number);
                        break;
                    case 4: // the fourth number.
                        CopyToPhonebookEntryField(iEntries[index].iNumber4, number);
                        break;
                    }
                break;
                }
            case RMobilePhoneBookStore::ETagPBEmailAddress:
                {
                TPtrC16 email;
                if (dataType != CPhoneBookBuffer::EPhBkTypeDes16)
                    {
                    User::Leave(KErrCorrupt);
                    }
                User::LeaveIfError( iPhoneBookBuffer->GetValue(email) );
                CopyToPhonebookEntryField(iEntries[index].iEmail, email);
                break;
                }
            default:
                {
                iPhoneBookBuffer->SkipValue(dataType);
                break;
                }
            }
        ret = iPhoneBookBuffer->GetTagAndType(fieldTag, dataType);
        }
 
    TRACE_FUNC_EXIT
    }

void CSCPBRCommandHandler::CopyToPhonebookEntryField(TDes8& aDest, const TDesC16& aSrc)
    {
    TRACE_FUNC_ENTRY
    TInt maxLength = aDest.MaxLength();
    if ( aSrc.Length() <= maxLength )
        {
        aDest.Copy(aSrc);
        }
    else
        {
        aDest.Copy( aSrc.Left(maxLength) );
        }
    TRACE_FUNC_EXIT    
    }

void CSCPBRCommandHandler::FormatReplyL()
    {
    TRACE_FUNC_ENTRY

    TInt count = iEntries.Count();
    for (TInt i = 0; i < count; ++i)
        {
        AppendEntryL(iEntries[i]);
        }

    TRACE_FUNC_EXIT
    }

void CSCPBRCommandHandler::AppendEntryL(const TPhoneBookEntry& aEntry)
    {
    TRACE_FUNC_ENTRY
    TBuf8<KSCPBRDefaultSizeOfOneEntry> entryBuffer;
    aEntry.Externalize(entryBuffer);
    
    TInt length = entryBuffer.Length() + KSCPBR().Length(); 
    if (length + iReply.Length() > iReply.MaxLength())
        {
        iReply.ReAllocL(iReply.MaxLength()+KBufBlockSize);
        }
    
    iReply.Append(KSCPBR);
    iReply.Append(entryBuffer);
    TRACE_FUNC_EXIT
    }

void CSCPBRCommandHandler::DoCancel()
	{
    TRACE_FUNC_ENTRY
    switch( iState )
        {
        case ESCPBRStateRead:
            {
            iPhoneBookStore.CancelAsyncRequest( EMobilePhoneStoreRead );
            break;
            }
        case ESCPBRStateGetPhoneBookInfo:
            {
            iPhoneBookStore.CancelAsyncRequest( EMobilePhoneStoreGetInfo );
            break;
            }
        case ESCPBRStateGet3GPBInfo:
            {
            iPhoneBookStore.CancelAsyncRequest( EGet3GPBInfoIPC );
            break;
            }
        default:
            break;
        }
    iState = ESCPBRStateIdle;
    TRACE_FUNC_EXIT
	}


void CSCPBRCommandHandler::TPhoneBookEntry::Externalize(TDes8& aDes) const
    {
    TRACE_FUNC_ENTRY
    
    aDes.Format(KSCPBRReplyOneEntry, iIndex, &iNumber1, &iNumber2, &iNumber3, &iNumber4, &iName, &iEmail);
    
    TRACE_FUNC_EXIT
    }
