// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//



/**
 @file
 @internalComponent
 @released
*/

#include <remcon/remconmediabrowsetargetbase.h>
#include <remconmediainformationtargetobserver.h>
#include <remconinterfaceselector.h>
#include <remconmediabrowsepanic.h>
#include <bluetooth/logger.h>
#include "mediabrowse.h"
#include "remconmediabrowsefault.h"
#include "remconqueuemessage.h"

const TInt KDatabaseUnawareUidCounter = 0;
#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_REMCONMEDIABROWSEAPI);
_LIT8(KLogFormat, "Operation Id = 0x%x, Data Lengh = %d");
#endif
//=========================================================================================
// Construction/Destruction
//=========================================================================================
CRemConMediaBrowseTargetBase::CRemConMediaBrowseTargetBase(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConDatabaseAwareMediaLibraryBrowseObserver& aMlObserver, 
		MRemConDatabaseAwareNowPlayingBrowseObserver& aNpObserver,
		TUint16 aMediaLibraryStateCookie)
	: CRemConInterfaceBase(TUid::Uid(KRemConMediaBrowseApiUid),
			KMaxLengthMediaBrowseMsg,
			aInterfaceSelector,
			ERemConClientTypeTarget),
	iInterfaceSelector(aInterfaceSelector),
	iRcdamlbo(&aMlObserver),
	iRcdanpbo(&aNpObserver),
	iMediaLibraryStateCookie(aMediaLibraryStateCookie),
	iAttributeIterator(iMediaAttributeIds),
	iNullIterator(iNullArray),
	iSearchInProgress(EFalse),
	iLastMlscUpdate(aMediaLibraryStateCookie)
	{
	}

CRemConMediaBrowseTargetBase::CRemConMediaBrowseTargetBase(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConDatabaseUnawareMediaLibraryBrowseObserver& aMlObserver, 
		MRemConDatabaseUnawareNowPlayingBrowseObserver& aNpObserver)
	: CRemConInterfaceBase(TUid::Uid(KRemConMediaBrowseApiUid),
			KMaxLengthMediaBrowseMsg,
			aInterfaceSelector,
			ERemConClientTypeTarget),
	iInterfaceSelector(aInterfaceSelector),
	iRcdumlbo(&aMlObserver),
	iRcdunpbo(&aNpObserver),
	iAttributeIterator(iMediaAttributeIds),
	iNullIterator(iNullArray),
	iSearchInProgress(EFalse)
	{
	}

CRemConMediaBrowseTargetBase::~CRemConMediaBrowseTargetBase()
	{
	iMediaAttributeIds.Close();
	iNullArray.Close();
	iOutBuf.Close();
	iSearchString.Close();
	iGetPathResponse->Close();
	iGiaResponse->Close();
	iGflResponse->Close();
	delete iGetPathResponse;
	delete iGiaResponse;
	delete iGflResponse;
	
	if (iNextMessageCallBack)
		{
		iNextMessageCallBack->Cancel();
		delete iNextMessageCallBack;
		}
	if (iNextItemCallBack)
		{
		iNextItemCallBack->Cancel();
		delete iNextItemCallBack;
		}
	
	iMsgQueue->Reset();
	delete iMsgQueue;
	}

void CRemConMediaBrowseTargetBase::BaseConstructL(TBool aSearchSupported)
	{
	iMsgQueue = new(ELeave)TRemConMessageQueue();

	iGetPathResponse = new(ELeave)RRemConGetPathResponse();
	iGiaResponse = new(ELeave)RRemConGetItemAttributesResponse();
	iGflResponse = new(ELeave)RRemConGetFolderItemsResponse();

	TCallBack cb(&NextMessageCb, this);
	iNextMessageCallBack = new(ELeave)CAsyncCallBack(cb, CActive::EPriorityStandard);
	
	TCallBack itemCallBack(&NextItemCallBack, this);
	iNextItemCallBack = new(ELeave)CAsyncCallBack(itemCallBack, CActive::EPriorityStandard);
	
	RRemConInterfaceFeatures features;
	User::LeaveIfError(features.Open());
	CleanupClosePushL(features);

	if(aSearchSupported)
		{
		features.AddOperationL(ESearchOperationId);
		iSearchSupported = ETrue;
		}
	
	if(DatabaseAware())
		{
		features.AddOperationL(EUIDPersistency);
		}
	
	//Mandate the following operationIds to be supported in the client
	features.AddOperationL(EGetFolderItemsOperationId);
	features.AddOperationL(EChangePathOperationId);
	features.AddOperationL(EGetItemAttributesOperationId);
	features.AddOperationL(ESetBrowsedPlayerOperationId);
	
	iOutBuf.CreateL(KMediaBrowseOutBufMaxLength);
	
	CRemConInterfaceBase::BaseConstructL(features, ETrue); // it's true, this interface is a bulk interface
	CleanupStack::PopAndDestroy(&features);
	}
	
//=========================================================================================
// RemCon interface stuff, called from interface selector
//=========================================================================================

/** 
@internalComponent
@released

Gets a pointer to a specific interface version.

@return A pointer to the interface, NULL if not supported.
*/
TAny* CRemConMediaBrowseTargetBase::GetInterfaceIf(TUid aUid)
	{
	TAny* ret = NULL;
	if ( aUid == TUid::Uid(KRemConInterfaceIf1) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConInterfaceIf*>(this)
			);
		}

	return ret;
	}

void CRemConMediaBrowseTargetBase::MrcibNewMessage(TUint aOperationId,
			const TDesC8& aData)
	{
	LOG_FUNC
	LOG2(KLogFormat, aOperationId, aData.Length());

	TMetadataTransferPDU currentOp = RAvrcpIPC::GetPDUIdFromIPCOperationId(aOperationId);
	switch(currentOp)
		{
	case EGetFolderItemsOperationId:
		AddToOperationQueue(EMbGetFolderItems, 
							EGetFolderItemsOperationId, aData);
		if (!iMsgQueue->IsEmpty() && !iInProgress && !iMsgQueue->Find(
			      TUid::Uid(KRemConMediaBrowseApiUid)
				 ,EGetFolderItemsOperationId))
			{
			iNextMessageCallBack->CallBack();
			}
		break;
	case EChangePathOperationId:
		ProcessChangePath(aData);
		break;
	case EGetItemAttributesOperationId:
		{
		AddToOperationQueue(EMbGetItemAttributes, 
							EGetItemAttributesOperationId, aData);
		if (!iMsgQueue->IsEmpty() && !iInProgress && !iMsgQueue->Find(
					      TUid::Uid(KRemConMediaBrowseApiUid)
						 ,EGetItemAttributesOperationId))
					{
					iNextMessageCallBack->CallBack();
					}
		break;
		}
	case ESearchOperationId:
		ProcessSearch(aData);
		break;
	case ESetBrowsedPlayerOperationId:
		ProcessGetPath(aData);
		break;	
	case EMediaLibraryStateCookieUpdateOperationId:
		ProcessMediaLibraryStateCookieUpdate(aData);
		break;
	default:
		__ASSERT_DEBUG(EFalse, MediaBrowseFault::Fault(EUnexpectedOperationId));
		break; 
		};
	}

//=========================================================================================
// Browse Interface functions, called from derived classes
//=========================================================================================
void CRemConMediaBrowseTargetBase::DoFolderListing(const TArray<TRemConItem>& aFolderListing, 
		TUint16 aMediaLibraryStateCookie, 
		TInt aResult)
	{
	if (aResult != KErrNone)
		{
		SendGetFolderItemsResponse(aResult, KNullDesC8);
		return;
		}	
	
	// If asserted here, means the client calls the interface FolderListing() 
	// more than once corresponding to the only once call 
	// MrcmbtoGetFolderListing()
	__ASSERT_DEBUG(iGflResponse->iItems.Count() == 0, MediaBrowsePanic::Panic(EFolderListingProvidedTwice));
	
	// Store clients state cookie to pass it back when requesting each item. 
	// This will ensure that we don't miss state change during the course of 
	// the non-atomic GetFolderListing operation
	iGflResponse->iUidCounter = aMediaLibraryStateCookie;
	
	// Store these UIDs, then ask for info about them
	if (iGflResponse->CopyItems(aFolderListing) != KErrNone)
		{
		SendGetFolderItemsResponse(KErrAvrcpAirInternalError, KNullDesC8);
		return;
		}
	
	iGflResponse->iCurrentListingSize = KGetFolderItemsResponseBaseSize;
	iGflResponse->iCurrentItem = -1;
	RequestNextItem();
	}

void CRemConMediaBrowseTargetBase::DoFolderUpResult(TUint aItemCount, TInt aResult)
	{
	SendChangePathResponse(aItemCount, aResult);
	}

void CRemConMediaBrowseTargetBase::DoFolderDownResult(TUint aItemCount, TInt aResult)
	{
	SendChangePathResponse(aItemCount, aResult);
	}

void CRemConMediaBrowseTargetBase::DoGetPathResult(TUint aItemCount, 
		TUint16 aMediaLibraryStateCookie, 
		TInt aResult)
	{
	if (aResult != KErrNone)
		{
		iGetPathResponse->Close();
		return SendError(EMbSetBrowsedPlayer,
				ESetBrowsedPlayerOperationId, aResult);
		}
	
	// Store the current UIDs counter.
	iMediaLibraryStateCookie = aMediaLibraryStateCookie;
	
	TInt status = KErrAvrcpAirBase - KErrAvrcpAirSuccess;
	iGetPathResponse->iStatus = status;
	iGetPathResponse->iUidCounter = aMediaLibraryStateCookie;
	iGetPathResponse->iNumberItems = aItemCount;
	
	RBuf8 responseBuf;
	TInt error = responseBuf.Create(iGetPathResponse->Size());
	if (error != KErrNone)
		{
		iGetPathResponse->Close();
		return SendError(EMbSetBrowsedPlayer, 
				ESetBrowsedPlayerOperationId, KErrAvrcpAirInternalError);
		}
	
	iGetPathResponse->iPduId = AvrcpBrowsing::ESetBrowsedPlayer;
	TRAP(error, iGetPathResponse->WriteL(responseBuf));
	if (error == KErrNone)
		{
		// Send the response back to the CT
		error = InterfaceSelector().SendBulkUnreliable(
				TUid::Uid(KRemConMediaBrowseApiUid),
				ESetBrowsedPlayerOperationId, responseBuf );
		}
	
	iGetPathResponse->Close();
	responseBuf.Close();
	}

void CRemConMediaBrowseTargetBase::DoSearchResult(TUint aNumberItemsFound,
		TUint16 aMediaLibraryStateCookie, 
		TInt aResult)
	{
	// GetFoldItems should in progress When this interface is called.
	__ASSERT_DEBUG(iSearchInProgress, MediaBrowsePanic::Panic(ESearchResultWithoutRequest));

	SendSearchResponse(aResult, aNumberItemsFound, aMediaLibraryStateCookie);
	}

void CRemConMediaBrowseTargetBase::DoMediaLibraryStateChange(TUint16 aMediaLibraryStateCookie)
	{
	if(DatabaseAware())
		{
		__ASSERT_DEBUG(aMediaLibraryStateCookie != KDatabaseUnawareUidCounter, MediaBrowsePanic::Panic(EZeroMediaLibraryStateCookie));
		
		// For database aware players we need to update if we have a pending update and
		// the new value is different to what we last 
		if(iMlscUpdatePending && (aMediaLibraryStateCookie != iLastMlscUpdate))
			{
			// Send update with new value
			SendMediaLibraryStateCookieUpdateResponse(aMediaLibraryStateCookie);
			}	
		
		// Always store the last value here.  When we are asked for an update
		// we will be provided with the value the update should be relative 
		// to, so we will compare to this. 
		iLastMlscUpdate = aMediaLibraryStateCookie;
		}
	else
		{
		__ASSERT_DEBUG(aMediaLibraryStateCookie == KDatabaseUnawareUidCounter, MediaBrowseFault::Fault(ENonZeroMediaLibraryStateCookie));
		
		if(iMlscUpdatePending)
			{
			SendMediaLibraryStateCookieUpdateResponse(aMediaLibraryStateCookie);
			}
		else
			{
			// For database aware clients the value can never change.  Indicate that
			// the client has informed us of the state change by incrementing our
			// update value so that we know there's been a change.  The value doesn't
			// matter, we just have to make it something other than zero.
			iLastMlscUpdate = 1;
			}
		}
	}

void CRemConMediaBrowseTargetBase::DoFolderItemResult(const TRemConItemUid& aFolderID,
		const TDesC8& aFolderName, 
		TFolderItemType aFolderType, 
		TFolderItemPlayable aPlayable,
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	// GetFolderItems should in progress When this interface is called.
	__ASSERT_DEBUG(iGetFolderListing || iGetItemAttributes, MediaBrowsePanic::Panic(EFolderItemResultWithoutRequest));

	if(iGetFolderListing)
		{
		ProcessFolderItemResult(aFolderID, aFolderName, aFolderType, aPlayable, aResult);
		}
	
	if (iGetItemAttributes)
		{
		ProcessGetItemAttributesResult(aAttributes, aResult);
		}
	}

void CRemConMediaBrowseTargetBase::DoMediaElementItemResult(const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	__ASSERT_DEBUG((iGetFolderListing || iGetItemAttributes), MediaBrowsePanic::Panic(EMediaElementItemResultWithoutRequest));

	if (iGetFolderListing)
		{
		ProcessMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes, aResult);
		}
	
	if (iGetItemAttributes)
		{
		ProcessGetItemAttributesResult(aAttributes, aResult);
		}		
	}

//=========================================================================================
// Utility functions, called internally
//=========================================================================================

void CRemConMediaBrowseTargetBase::ProcessMediaLibraryStateCookieUpdate(const TDesC8& aData)
	{
	// Try to read the incoming request
	RRemConUidsChangedRequest request;
	TRAPD(error, request.ReadL(aData));
	__ASSERT_DEBUG(error == KErrNone, MediaBrowseFault::Fault(EBadlyFormattedMediaLibraryStateCookieUpdate));
	static_cast<void>(error == error); // stops compiler warnings (assert above indicates design contract).

	if(request.iInitialUidCounter != iLastMlscUpdate)
		{
		// The client has updated the uid counter since the bearer
		// last asked.  Tell it the new value.  The Send..Response 
		// function deals with all the state management necessary,
		// including whether we are database aware or unaware
		SendMediaLibraryStateCookieUpdateResponse(iLastMlscUpdate);
		}
	else
		{
		// Bearer still up to date.  Remember that it's waiting for
		// an update.
		iMlscUpdatePending = ETrue;
		}
	}

void CRemConMediaBrowseTargetBase::ProcessGetItemAttributes(const TDesC8& aData)
	{
	TRemConFolderScope scope;
	TRemConItemUid item;
	TUint16 uidCounter;
	TInt err = ParseGetItemAttributesRequest(aData, scope, item, uidCounter);
	if (err != KErrNone)
	    {
	    SendGetItemAttributesResponse(err, KNullDesC8);
	    return;
	    }
	
	iAttributeIterator.Start();	
	iInProgress = ETrue;
	iGetItemAttributes = ETrue;	
	TInt result = KErrNone;
	if(scope == ENowPlayingFolder)
		{
		if (!DatabaseAware() && (0 == uidCounter))//Database UnAware
			{
			result = iRcdunpbo->MrcdunpboGetItem(item, iAttributeIterator);
			}
		else if (DatabaseAware() && (uidCounter > 0))
			{
			result = iRcdanpbo->MrcdanpboGetItem(item, iAttributeIterator, uidCounter);
			}
		else
			{
			result = KErrAvrcpAirInvalidParameter;
			}
		}
	else
		{
		if (!DatabaseAware() && (0 == uidCounter))//Database UnAware
			{
			result = iRcdumlbo->MrcdumlboGetItem(scope, item, iAttributeIterator);
			}
		else if (DatabaseAware() && (uidCounter > 0))
			{
			result = iRcdamlbo->MrcdamlboGetItem(scope, item, iAttributeIterator, 
					                     uidCounter);
			}
		else
			{
			result = KErrAvrcpAirInvalidParameter;
			}
		}
	
	// The call back function returns error.
	if (result != KErrNone)
		{
		SendGetItemAttributesResponse(result, KNullDesC8);
		}
	}

void CRemConMediaBrowseTargetBase::SendGetItemAttributesResponse(TInt aResult, const TDesC8& aData)
	{
	if(aResult != KErrNone)
		{
		SendError(EMbGetItemAttributes, EGetItemAttributesOperationId, aResult);
		}
	else
		{
		InterfaceSelector().SendBulkUnreliable(
				TUid::Uid(KRemConMediaBrowseApiUid),
				EGetItemAttributesOperationId, aData);
		}
	
	iInProgress = EFalse;
	iGetItemAttributes = EFalse;
	iMediaAttributeIds.Reset();
	iGiaResponse->Close();
	
	if (!iMsgQueue->IsEmpty())
		{
		iNextMessageCallBack->CallBack();
		}
	}

void CRemConMediaBrowseTargetBase::ProcessChangePath(const TDesC8& aData)
	{
	// Try to read the incoming request
	TInt error = KErrNone;
	RRemConChangePathRequest request;
	TRAP(error, request.ReadL(aData));

	// Couldn't parse the request; tell them it was invalid
	if (error != KErrNone)
		{
	    SendError(EMbChangePath, EChangePathOperationId, KErrAvrcpAirInvalidParameter);
	    return;
		}

	if(request.iDirection == AvrcpBrowsing::KUp)
		{
		if (DatabaseAware() && (request.iUidCounter > 0))
			{
			iRcdamlbo->MrcdamlboFolderUp(request.iUidCounter);
			}
		else if (!DatabaseAware() && (0 == request.iUidCounter))
			{
			iRcdumlbo->MrcdumlboFolderUp();
			}
		else
			{
			SendError(EMbChangePath, EChangePathOperationId, KErrAvrcpAirInvalidParameter);
			}
		}
	else if(request.iDirection == AvrcpBrowsing::KDown)
		{
		if (DatabaseAware() && (request.iUidCounter > 0))
			{
			iRcdamlbo->MrcdamlboFolderDown(request.iElement, request.iUidCounter);
			}
		else if (!DatabaseAware() && (0 == request.iUidCounter))
			{
			iRcdumlbo->MrcdumlboFolderDown(request.iElement);
			}
		else
			{
			SendError(EMbChangePath, EChangePathOperationId, KErrAvrcpAirInvalidParameter);
			}
		}
	else
		{
		SendError(EMbChangePath, EChangePathOperationId, KErrAvrcpAirInvalidDirection);
		}
	}

void CRemConMediaBrowseTargetBase::SendSearchResponse(TInt aResult, TUint aNumberItemsFound, TUint16 aMediaLibraryStateCookie)
	{	
	iSearchString.Close();
	
	// format the response in a RRemConSearchResponse
	RRemConMediaErrorResponse errResponse;
	RRemConSearchResponse response;
	TInt symbianError = errResponse.SymbianErrorCheck(aResult);
	
	response.iStatus = errResponse.SymbianErrToStatus(symbianError);	
	response.iPduId = AvrcpBrowsing::ESearch;//0x80
	response.iUidCounter = aMediaLibraryStateCookie;
	response.iNumberItems = aNumberItemsFound;
	TRAPD(error, response.WriteL(iOutBuf));
	if (error == KErrNone)
		{
		// send the response back to the CT
		error = InterfaceSelector().SendBulkUnreliable(
				TUid::Uid(KRemConMediaBrowseApiUid),
				ESearchOperationId,  iOutBuf );
		}
	
	//Search operatin complete.
	iSearchInProgress = EFalse;
	}

void CRemConMediaBrowseTargetBase::ProcessSearch(const TDesC8& aData)
	{
	// Don't trouble the client with this if they've informed us they don't
	// support it
	if (!iSearchSupported)
		{
		return SendSearchResponse(KErrAvrcpAirSearchNotSupported, 0, 0);
		}
	
	// We know a search operation is in progress
	if (iSearchInProgress)
		{
		return SendSearchResponse(KErrAvrcpAirSearchInProgress, 0, 0);
		}
		
	// Try to read the incoming request
	TInt error = KErrNone;
	RRemConSearchRequest request;
	TRAP(error, request.ReadL(aData));

	// Couldn't parse the request; tell them it was invalid
	if (error != KErrNone)
		{
		error = (error == KErrNoMemory) ? 
				KErrAvrcpAirInternalError : KErrAvrcpAirInvalidParameter;
		
		return SendSearchResponse(error, 0, 0);
		}

	// Check the character set
	if (request.iCharset != KUtf8MibEnum)
		{
		iSearchInProgress = EFalse;
		return SendSearchResponse(KErrAvrcpAirInvalidParameter, 0, 0);
		}
	
	iSearchString.Close();
	iSearchString.Assign(request.iSearchString);
	//iSearchString has taken ownership of request's search string.
	request.iSearchString.Assign(NULL);
	
	iSearchInProgress = ETrue;
	
	if(DatabaseAware())
		{
		iRcdamlbo->MrcdamlboSearch(iSearchString);
		}
	else
		{
		iRcdumlbo->MrcdumlboSearch(iSearchString);
		}
	
	request.Close();
	}

void CRemConMediaBrowseTargetBase::ProcessGetPath(const TDesC8& aData)
	{
	iGflResponse->iMaxResponse = *(reinterpret_cast<const TInt*>(aData.Ptr()));

	if(DatabaseAware())
		{
		iRcdamlbo->MrcdamlboGetPath(iGetPathResponse->iPath);
		}
	else
		{
		iRcdumlbo->MrcdumlboGetPath(iGetPathResponse->iPath);
		}
	}

void CRemConMediaBrowseTargetBase::ProcessGetFolderItems(const TDesC8& aData)
	{
	// The bearer is responsible for ensuring we have been supplied with response
	// max size before sending us any requests
	__ASSERT_DEBUG(iGflResponse->iMaxResponse != 0, MediaBrowseFault::Fault(ERequestWithoutMaxResponseBeingSet));
	
	iInProgress = ETrue;
	iGetFolderListing = ETrue;
	
	RRemConGetFolderItemsRequest request;
	TRAPD(err, request.ReadL(aData));
	if(err != KErrNone)
		{
		request.Close();
		SendGetFolderItemsResponse(KErrAvrcpAirInvalidParameter, KNullDesC8);
		return;
		}

	if(request.iScope == AvrcpBrowsing::KSearchScope && !iSearchSupported)
	    {
        request.Close();
        SendGetFolderItemsResponse(KErrAvrcpAirSearchNotSupported, KNullDesC8);
        return;
        }
	
	if (request.CopyAttributes(iMediaAttributeIds) != KErrNone)
		{
		request.Close();
		SendGetFolderItemsResponse(KErrAvrcpAirInternalError, KNullDesC8);
		return;
		}
	iAttributeIterator.Start();

	if(request.iScope == AvrcpBrowsing::KVirtualFilesystemScope)
		{
		iScope = EBrowseFolder;
		}
	else if(request.iScope == AvrcpBrowsing::KNowPlayingScope)
		{
		iScope = ENowPlayingFolder;
		}
	else if(request.iScope == AvrcpBrowsing::KSearchScope)
        {
        iScope = ESearchResultFolder;
        }
	else
		{
		request.Close();
		SendGetFolderItemsResponse(KErrAvrcpAirInvalidScope, KNullDesC8);
		return;
		}
	
	if (request.iStartItem > request.iEndItem)
		{
		request.Close();
		SendGetFolderItemsResponse(KErrAvrcpAirRangeOutOfBounds, KNullDesC8);
		return;
		}
	
	if(iScope == ENowPlayingFolder)
		{
		if(DatabaseAware())
			{
			iRcdanpbo->MrcdanpboGetFolderListing(request.iStartItem, request.iEndItem);
			}
		else
			{
			iRcdunpbo->MrcdunpboGetFolderListing(request.iStartItem, request.iEndItem);
			}
		}
	else
		{
		if(DatabaseAware())
			{
			iRcdamlbo->MrcdamlboGetFolderListing(iScope,request.iStartItem, request.iEndItem);
			}
		else
			{
			iRcdumlbo->MrcdumlboGetFolderListing(iScope,request.iStartItem, request.iEndItem);
			}
		}
	
	request.Close();
	}

void CRemConMediaBrowseTargetBase::SendGetFolderItemsResponse(TInt aResult, const TDesC8& aData)
	{
	if(aResult != KErrNone)
		{
		SendError(EMbGetFolderItems, EGetFolderItemsOperationId, aResult);
		}
	else
		{
		InterfaceSelector().SendBulkUnreliable(
				TUid::Uid(KRemConMediaBrowseApiUid),
				EGetFolderItemsOperationId, aData);
		}
	
	iInProgress = EFalse;
	iGetFolderListing = EFalse;
	iMediaAttributeIds.Reset();
	iGflResponse->Close();
	
	iNextItemCallBack->Cancel();
	
	if (!iMsgQueue->IsEmpty())
		{
		iNextMessageCallBack->CallBack();
		}
	}

TInt CRemConMediaBrowseTargetBase::NextMessageCb(TAny* aThis)
	{
	static_cast<CRemConMediaBrowseTargetBase*>(aThis)->DoNextMessage();
	return KErrNone;
	}

void CRemConMediaBrowseTargetBase::DoNextMessage()
	{
	__ASSERT_DEBUG(!iMsgQueue->IsEmpty(), MediaBrowseFault::Fault(EUnexpectedNextMessageCallback));
	CRemConQueuedMessage* msg = iMsgQueue->First();
	iMsgQueue->Remove(*msg);
	
	switch (msg->iOperationId)
		{
		case EGetFolderItemsOperationId:
			ProcessGetFolderItems(msg->Data());
			break;
		case EGetItemAttributesOperationId:
			ProcessGetItemAttributes(msg->Data());
			break;
		default:
			__ASSERT_DEBUG(EFalse, MediaBrowseFault::Fault(EUnexpectedNextMessageCallback));
			break;
		}
	delete msg;
	}

void CRemConMediaBrowseTargetBase::SendMediaLibraryStateCookieUpdateResponse(TUint16 aMediaLibraryStateCookie)
	{
	LOG_FUNC
	
	TUint16 newValue = DatabaseAware() ? aMediaLibraryStateCookie : KDatabaseUnawareUidCounter;

	TInt error = KErrNone;
	RRemConUidsChangedResponse response;
	response.iUidCounter = newValue;
	TRAP(error, response.WriteL(iOutBuf));

	if (error == KErrNone)
		{
		// send the response back to the CT
		error = InterfaceSelector().SendBulkUnreliable(
				TUid::Uid(KRemConMediaBrowseApiUid),
				EMediaLibraryStateCookieUpdateOperationId, iOutBuf);
		
		iLastMlscUpdate = newValue;
		iMlscUpdatePending = EFalse;
		}
	// otherwise we couldn't update the client.  Leave our state with the update
	// pending then we'll try again next time the client tells us state has 
	// changed.  
	}

void CRemConMediaBrowseTargetBase::SendChangePathResponse(TUint aItemCount, 
		TInt aResult)
	{
	if (aResult != KErrNone)
		{
		return SendError(EMbChangePath, EChangePathOperationId, aResult);
		}
	
	TInt error = KErrNone;

	// Format the response in a RRemConChangePathResponse
	RRemConChangePathResponse response;
	response.iStatus = KErrAvrcpAirBase - KErrAvrcpAirSuccess;//0x4
	response.iNumberItems = aItemCount;
	
	RBuf8 responseBuf;
	error = responseBuf.Create(KMediaBrowseOutBufMaxLength);
	if (error != KErrNone)
		{
		responseBuf.Close();
		SendError(EMbChangePath, EChangePathOperationId, KErrAvrcpAirInternalError);
		return;
		}
	
	response.iPduId = AvrcpBrowsing::EChangePath;
	TRAP(error, response.WriteL(responseBuf));
	if (error == KErrNone)
		{
		// send the response back to the CT
		error = InterfaceSelector().SendBulkUnreliable(
				TUid::Uid(KRemConMediaBrowseApiUid),
				EChangePathOperationId, responseBuf );
		}
	
	responseBuf.Close();
	}

TInt CRemConMediaBrowseTargetBase::NextItemCallBack(TAny* aThis)
	{
	static_cast<CRemConMediaBrowseTargetBase*>(aThis)->RequestNextItem();
	return KErrNone;
	}

void CRemConMediaBrowseTargetBase::RequestNextItem()
	{
	TInt err = KErrNone;
	TInt result = KErrNone;
	RBuf8 responseBuf;
	
	// If true, indicate that we have not got all the items requested, 
	// so going on. 
	// There are some array elements accessed by [] as follows without 
	// checking range for it is done in RequestNextItem function.
	if ( iGflResponse->RequestNextItem(err, responseBuf, 
			iGflResponse->iUidCounter) )
		{
		TBool folderItem = EFalse;
		if (AvrcpBrowsing::EFolderItem == iGflResponse->iItems[iGflResponse->iCurrentItem].iType)
			{
			folderItem = ETrue;
			}
		iAttributeIterator.Start();	
		if(iScope == ENowPlayingFolder)
			{
			if(DatabaseAware())
				{
				result = iRcdanpbo->MrcdanpboGetItem(
			    		iGflResponse->iItems[iGflResponse->iCurrentItem].iUid,
			    		folderItem ? iNullIterator : iAttributeIterator,
			    		iGflResponse->iUidCounter);
				}
			else
				{
				result = iRcdunpbo->MrcdunpboGetItem( 
			    		iGflResponse->iItems[iGflResponse->iCurrentItem].iUid,
			    		folderItem ? iNullIterator : iAttributeIterator);
				}
			}
		else
			{
			if(DatabaseAware())
				{
				result = iRcdamlbo->MrcdamlboGetItem(iScope,
			    		iGflResponse->iItems[iGflResponse->iCurrentItem].iUid,
			    		folderItem ? iNullIterator : iAttributeIterator,
			    		iGflResponse->iUidCounter);
				}
			else
				{
				result = iRcdumlbo->MrcdumlboGetItem(iScope, 
			    		iGflResponse->iItems[iGflResponse->iCurrentItem].iUid,
			    		folderItem ? iNullIterator : iAttributeIterator);
				}
			}
		
		// The call back function reutrns error.
		if (result != KErrNone)
			{
			SendGetFolderItemsResponse(result, KNullDesC8);
			}
		}
	// If comes here, indicate that we stop requesting the next item
	// which means two possibilities:
	// 1. Success: Have got all the items we want.
	// 2. Error: Error occured internally.
	else if ( err == KErrNone ) //Possibility 1.
		{
		SendGetFolderItemsResponse(KErrNone, responseBuf);
		}
	else // Possibility 2.
		{
		SendGetFolderItemsResponse(KErrAvrcpAirInternalError, KNullDesC8);
		}
	responseBuf.Close();
	}

void CRemConMediaBrowseTargetBase::DoItemComplete(TInt aResult)
	{
	if (aResult != KErrNone)
		{
		SendGetFolderItemsResponse(aResult, KNullDesC8);
		}
	else
		{
		// We have to put an async break in here - otherwise if large
		// numbers of items are requested we could overflow the stack
		iNextItemCallBack->CallBack();
		}
	}

void CRemConMediaBrowseTargetBase::SendError(TUint8 aPduId, 
		TUint aOperationId, 
		TInt aError)
	{
	TInt error = KErrNone;
	RRemConMediaErrorResponse response;
	response.iPduId = aPduId;
	response.iStatus = RAvrcpIPC::SymbianErrToStatus(aError);
	TRAP(error, response.WriteL(iOutBuf));
	if (error == KErrNone)
		{
		InterfaceSelector().SendBulkUnreliable(
				TUid::Uid(KRemConMediaBrowseApiUid),
				aOperationId, iOutBuf);
		}
	}

/** 
Sets an attribute value for the requested item.
*/
TInt CRemConMediaBrowseTargetBase::DoAttributeValue(
		TMediaAttributeId aAttributeId, 
		const TDesC8& aAttributeData )
	{
	iSetAttributeValue = ETrue;
	
	REAResponse resp;
	resp.iAttributeId = aAttributeId;
	resp.iCharset = KUtf8MibEnum;
	resp.iStringLen = aAttributeData.Length();
	resp.iString = aAttributeData.Alloc();
	if (!resp.iString)
		{
		iSetAttributeValue = EFalse;
		return KErrNoMemory;
		}
	TInt status = KErrNone;
	if(iGiaResponse->Size() + resp.iStringLen < iGflResponse->iMaxResponse)
		{
		status = iGiaResponse->iAttributes.Append(resp);
		if (status != KErrNone)
			{
			iSetAttributeValue = EFalse;
			resp.Close();  // make sure heap string is de-allocated
			}
		}
	
	return status;
	}

/**
Signals that all attributes requested has been supplied.
*/
 void CRemConMediaBrowseTargetBase::DoAllAttributesCompleted(TInt aResult)
	{
	TInt error = KErrNone;
	__ASSERT_DEBUG(((aResult != KErrNone) || ((aResult == KErrNone) && iSetAttributeValue)),
			MediaBrowseFault::Fault(EResultErrorCodeMismatch));
	
	if (aResult == KErrNone)
		{
		// Finalise response; update number of attributes returned
		iGiaResponse->iNumberAttributes = iGiaResponse->iAttributes.Count();
		}

	// Allocate a buffer for the formatted message
	RBuf8 messageBuffer;	
	TInt bufferSize = 
	(aResult == KErrNone) ? iGiaResponse->Size() : KBrowseResponseBaseLength;
	
	if ( messageBuffer.Create(bufferSize) != KErrNone )
		{
		SendGetItemAttributesResponse(KErrAvrcpAirInternalError, KNullDesC8);
		}
	else
		{
		// Send the result back to the CT
		iGiaResponse->iPduId = AvrcpBrowsing::EGetItemAttributes; // 0x73
		iGiaResponse->iStatus = RAvrcpIPC::SymbianErrToStatus(aResult);
		
		TRAP(error, iGiaResponse->WriteL(messageBuffer));
		if (error == KErrNone)
			{
			SendGetItemAttributesResponse(KErrNone, messageBuffer);
			}
		else
			{
			SendGetItemAttributesResponse(KErrAvrcpAirInternalError, KNullDesC8);
			}
		}
	messageBuffer.Close();
	}

TInt CRemConMediaBrowseTargetBase::ItemAttributesResult(
		const TArray<TMediaElementAttribute>& aAttributes)
	{
	TInt error = KErrNone;
	TMediaAttributeId attributeId;
	for (TInt i = 0; i < aAttributes.Count(); i++)
		{
		// check that the values supplied were requested
		attributeId = aAttributes[i].iAttributeId;
		if ( KErrNotFound == iMediaAttributeIds.Find(attributeId) )
			{
			//Omit the invalid ones
			continue;
			}
		
		error = DoAttributeValue(attributeId, *aAttributes[i].iString);
		if (error != KErrNone)
			{
			break;
			}
		}
	
	return error;
	}

void CRemConMediaBrowseTargetBase::AddToOperationQueue(TUint8 aPduId,
		TInt aOperationId, 
		const TDesC8& aData)
	{
	CRemConQueuedMessage* msg = NULL;
	TRAPD(err, msg = CRemConQueuedMessage::NewL(
			TUid::Uid(KRemConMediaBrowseApiUid), aData, aOperationId));			
	if (err == KErrNone)
		{
		iMsgQueue->AddLast(*msg);
		}
	else
		{
		SendError(aPduId, aOperationId, KErrAvrcpAirInternalError);
		}
	}

TInt CRemConMediaBrowseTargetBase::ParseGetItemAttributesRequest(
		const TDesC8& aData,
		TRemConFolderScope& aScope,
		TRemConItemUid& aItemUid,
		TUint16& aMediaLibraryStateCookie)
	{
	// Try to read the incoming request
	TInt error = KErrNone;
	RRemConGetItemAttributesRequest request;
	TRAP(error, request.ReadL(aData));

	// Couldn't parse the request;tell them it was invalid
	// Specification says unique id must not be 0x0
	if (error != KErrNone || request.iElement == 0)
		{
		request.Close();
		return KErrAvrcpAirInvalidParameter;
		}

	if (request.iNumberAttributes == 0)
		{
		// spec says this is a request for all attribs
		// current spec has 7 specified (0x01 to 0x07)
		for (TInt i = 1; i <= KMediaAttributeNum; i++)
			{
			if (iMediaAttributeIds.Append(static_cast<TMediaAttributeId>(i))
					!= KErrNone)
				{
				request.Close();
				return KErrAvrcpAirInternalError;
				}
			}
		}
	else
		{
		// No need to check 
		// request.iNumberAttributes == request.iAttributes.Count()
		// as this must be correct or request.ReadL(aData) leaves
		TUint8 value;
		TMediaAttributeId attributeid;
		for (TInt i = 0; i < request.iNumberAttributes; i++)
			{
			value = request.iAttributes[i];
			if (value > 0 && value <= KMaxMediaAttributeValue )
				{
				attributeid = static_cast<TMediaAttributeId>(value);
				if (iMediaAttributeIds.Append(attributeid) != KErrNone)
					{
					request.Close();
					return KErrAvrcpAirInternalError;
					}
				}
			}
		}
	// Check that some valid attribute ids have been found
	if (iMediaAttributeIds.Count() == 0)
		{
		request.Close();
		return KErrAvrcpAirInvalidParameter;
		}
		
	if(request.iScope == AvrcpBrowsing::KSearchScope)
		{
		aScope = ESearchResultFolder;
		}
	else if(request.iScope == AvrcpBrowsing::KVirtualFilesystemScope)
		{
		aScope = EBrowseFolder;
		}
	else if (request.iScope == AvrcpBrowsing::KNowPlayingScope)
		{
		aScope = ENowPlayingFolder;
		}
	else
		{
		request.Close();
		return KErrAvrcpAirInvalidScope;
		}
	
	aItemUid = request.iElement;
	aMediaLibraryStateCookie = request.iUidCounter;
	
	request.Close();
	return KErrNone;
	}

void CRemConMediaBrowseTargetBase::ProcessMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName,
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	TInt internalError = KErrNone;
	if (aResult == KErrNone)
		{
		__ASSERT_DEBUG(aMediaType < AvrcpBrowsing::KMediaTypeReserved, MediaBrowsePanic::Panic(EInvalidMediaType));

		RItem& item = iGflResponse->iItems[iGflResponse->iCurrentItem];
		if (item.iUid == aMediaID)
			{
			item.iType = AvrcpBrowsing::EMediaElement;
			item.iCharset = KUtf8MibEnum;
			
			item.iName = aMediaName.Alloc();		
			internalError = (!item.iName) ? KErrNoMemory : internalError;
			
			if (internalError == KErrNone)
				{
				item.iNameLength = aMediaName.Length();
				item.iMediaType = static_cast<AvrcpBrowsing::TFolderType>(aMediaType);
				item.iNumberAttributes = aAttributes.Count();
				item.iLength = KMediaElementItemBaseLength + item.iNameLength;
				TInt attributeCount = aAttributes.Count();
				REAResponse attribute;
				for (TInt i = 0; i < attributeCount; i++)
					{
					// Check that the values supplied were requested
					if (KErrNotFound == iMediaAttributeIds.Find(aAttributes[i].iAttributeId))
						{
						//Omit the invalid ones
						continue;
						}
					
					attribute.iAttributeId = aAttributes[i].iAttributeId;				
					attribute.iString = aAttributes[i].iString->Alloc();
					internalError = (!attribute.iString) ? KErrNoMemory : internalError;
					if (internalError == KErrNone)
						{
						attribute.iCharset = KUtf8MibEnum;
						attribute.iStringLen = attribute.iString->Length();
						item.iAttributes.Append(attribute);
						
						item.iLength += 
						KAttributeBaseLength + attribute.iString->Length();
						}
					else
						{
						//it's useless to continue as there is an allocation issue
						break;
						}
					}
				}
			}
		else
			{
			internalError = KErrArgument;
			}
		}
	
	aResult = 
	(KErrNone == internalError)? aResult : KErrAvrcpAirInternalError;
	
	DoItemComplete(aResult);
	}

void CRemConMediaBrowseTargetBase::ProcessFolderItemResult(
		const TRemConItemUid& aFolderID,
		const TDesC8& aFolderName, 
		TFolderItemType aFolderType, 
		TFolderItemPlayable aPlayable,
		TInt aResult)
	{
	TInt internalError = KErrNone;
	if (aResult == KErrNone)
	    {
		__ASSERT_DEBUG(aFolderType < AvrcpBrowsing::EFolderTypeReserved, MediaBrowsePanic::Panic(EInvalidFolderType));
		__ASSERT_DEBUG(aPlayable < AvrcpBrowsing::KPlayableReserved, MediaBrowsePanic::Panic(EInvalidPlayableValue));
		
		RItem& item = iGflResponse->iItems[iGflResponse->iCurrentItem];
		if (item.iUid == aFolderID)
			{
			item.iType = AvrcpBrowsing::EFolderItem;
			item.iCharset = KUtf8MibEnum;
			
			item.iName = aFolderName.Alloc();
			internalError = (!item.iName) ? KErrNoMemory : internalError;
					
			item.iNameLength = aFolderName.Length();
			
			item.iFolderType = 
			static_cast<AvrcpBrowsing::TFolderType>(aFolderType);
			
			item.iPlayable = aPlayable;
			item.iLength = KFolderItemBaseLength + item.iNameLength;
			}
		else
			{
			internalError = KErrArgument;
			}
	    }
			                        
	aResult = (internalError == KErrNone) ? aResult : KErrAvrcpAirInternalError;
	
	DoItemComplete(aResult);
	}

void CRemConMediaBrowseTargetBase::ProcessGetItemAttributesResult(
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult)
	{
	TInt internalError = KErrNone;
	if (aResult == KErrNone)
	    {
	    internalError = ItemAttributesResult(aAttributes);
	    }				
	
	aResult =
	(KErrNone == internalError)? aResult : KErrAvrcpAirInternalError;
	
	DoAllAttributesCompleted(aResult);
	}

inline TBool CRemConMediaBrowseTargetBase::DatabaseAware() const
	{
	return iRcdanpbo ? ETrue : EFalse;
	}
