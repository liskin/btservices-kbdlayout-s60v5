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
 @publishedAll
 @released
*/

#include <bluetooth/logger.h>
#include <remcon/avrcpspec.h>
#include <remconmediainformationtarget.h>
#include <remconmediainformationtargetobserver.h>
#include <remconinterfaceselector.h>
#include <avcframe.h>

#include "mediainformation.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_MEDIA_INFO);
#endif

EXPORT_C CRemConMediaInformationTarget* CRemConMediaInformationTarget::NewL(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConMediaInformationTargetObserver& aObserver)
	{
	LOG_STATIC_FUNC

	CRemConMediaInformationTarget* self = new(ELeave) CRemConMediaInformationTarget(aInterfaceSelector, aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CRemConMediaInformationTarget::CRemConMediaInformationTarget(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConMediaInformationTargetObserver& aObserver)
:	CRemConInterfaceBase(TUid::Uid(KRemConMediaInformationApiUid), 
						 KMaxLengthMediaInformationMsg, 
						 aInterfaceSelector,
						 ERemConClientTypeTarget), 
	iObserver(aObserver),
	iAttributeIterator(iMediaAttributeIDs),
	iMsgQueue(_FOFF(CRemConMediaInformationQueuedMessage, iLink))
	{
	}
	
void CRemConMediaInformationTarget::ConstructL()
	{
	iResponse = new(ELeave)RRemConGetElementAttributesResponse();
	TCallBack cb(&NextMessageCb, this);
	iNextMessageCallBack = new (ELeave) CAsyncCallBack(cb, CActive::EPriorityStandard);
	BaseConstructL();
	}
	

/** Destructor.

@publishedAll
@released
*/
EXPORT_C CRemConMediaInformationTarget::~CRemConMediaInformationTarget()
	{
	iMediaAttributeIDs.Close();
	iResponse->Close();
	delete iResponse;
	iNextMessageCallBack->Cancel();
	delete iNextMessageCallBack;
	TSglQueIter<CRemConMediaInformationQueuedMessage> iter(iMsgQueue);
	CRemConMediaInformationQueuedMessage* msg;
	iter.SetToFirst();
	while ((msg = iter++) != NULL)
		{
		iMsgQueue.Remove(*msg);
		delete msg;
		}
	}

/** Gets a pointer to a specific interface version.

@return A pointer to the interface, NULL if not supported.
@internalComponent
@released
*/
TAny* CRemConMediaInformationTarget::GetInterfaceIf(TUid aUid)
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

EXPORT_C TInt CRemConMediaInformationTarget::AttributeValue( TMediaAttributeId aAttributeId, TDesC8& aAttributeData )
	{
	// check that the values supplied were requested
	if ( KErrNotFound == iMediaAttributeIDs.Find( aAttributeId ) )
		{
		return KErrNotFound;
		}
		
	REAResponse resp;
	resp.iAttributeId = aAttributeId;
	resp.iCharset = KUtf8MibEnum;
	resp.iStringLen = aAttributeData.Length();
	resp.iString = aAttributeData.Alloc();
	if (resp.iString == NULL)
		{
		return KErrNoMemory;
		}
	TInt status = iResponse->iAttributes.Append(resp);
	if (status != KErrNone)
		{
		resp.Close();  // make sure heap string is de-allocated
		}
	return status;
	}

// from MRemConInterfaceIf
void CRemConMediaInformationTarget::SendError(TInt aError)
	{
	RBuf8 outBuf;
	if (outBuf.Create(KAVCFrameMaxLength) != KErrNone)
		{
		// On OOM drop the message
		return;
		}
	
	TInt error = 0;
	RAvrcpIPCError response;
	response.iError = aError;
	TRAP(error, response.WriteL(outBuf));   // Don't send error if OOM
	if (error == KErrNone)
		{
		InterfaceSelector().SendUnreliable(TUid::Uid(KRemConMediaInformationApiUid),
							EGetElementAttributes, ERemConResponse, outBuf);
		}
	outBuf.Close();
	}

// from MRemConInterfaceIf
void CRemConMediaInformationTarget::MrcibNewMessage(TUint aOperationId, const TDesC8& aData )
	{
	LOG1(_L("\taOperationId = 0x%02x"), aOperationId);
	LOG1(_L("\taData.Length = %d"), aData.Length());

	(void) aOperationId; // ignore warning about this variable being unused
	
	if (!iInProgress && iMsgQueue.IsEmpty())
		{
		ProcessMessage(aData);
		}
	else
		{
		CRemConMediaInformationQueuedMessage* msg = NULL;
		TRAPD(err, msg = CRemConMediaInformationQueuedMessage::NewL(aData));
		if (err == KErrNone)
			{
			iMsgQueue.AddLast(*msg);
			}
		}
	}
	
void CRemConMediaInformationTarget::ProcessMessage(const TDesC8& aData)
	{
	iInProgress = ETrue;
	// Try to read the incoming request
	TInt error = KErrNone;
	RRemConGetElementAttributesRequest request;
	TRAP(error, request.ReadL(aData));

	// Couldn't parse the request; tell them it was invalid
	if (error != KErrNone)
		{
		request.Close();
		return SendError(KErrAvrcpMetadataParameterNotFound);
		}

	// Specification section 5.3.1 (page 49) says unique id
	// must be 0x0. All other values are currently reserved
	if (request.iElement != 0)
		{
		request.Close();
		return SendError(KErrAvrcpMetadataInvalidParameter);
		}

	// this may have been used by a previous GetElementAttributes, so
	iMediaAttributeIDs.Reset();

	if (request.iNumberAttributes == 0)
		{
		// spec says this is a request for all attribs
		// current spec has 7 specified (0x01 to 0x07)
		for (TInt i = 1; i <= 7; i++)
			{
			if (iMediaAttributeIDs.Append((TMediaAttributeId)i) != KErrNone)
				{
				request.Close();
				return SendError(KErrAvrcpMetadataInternalError);
				}
			}
		}
	else
		{
		// No need to check request.iNumberAttributes == request.iAttributes.Count()
		// as this must be correct or request.ReadL(aData) leaves
		for (TInt i = 0; i < request.iNumberAttributes; i++)
			{
			TUint8 value = request.iAttributes[i];
			if (value > 0 && value <= KMaxMediaAttributeValue )
				{
				if (iMediaAttributeIDs.Append((TMediaAttributeId)value) != KErrNone)
					{
						request.Close();
						return SendError(KErrAvrcpMetadataInternalError);
					}
				}
			}
		}
	request.Close();

	// check that some valid attribute ids have been found
	if (iMediaAttributeIDs.Count())
		{
		// if the client has not yet called Completed() on the last request
		// clear the the attributes from the previous response
		iResponse->Close();
		
		// reset the interator to the start, as it may have been used before
		iAttributeIterator.Start();
		
		// call the client API to get the client value.
		iObserver.MrcmitoGetCurrentlyPlayingMetadata(iAttributeIterator);
		}
	else
		{
		// no valid attribute ids found so return an error to bearer
		SendError(KErrAvrcpMetadataParameterNotFound);
		}
	}

// The client application has signaled that all attributes have been returned so 
// response can now be sent
EXPORT_C void CRemConMediaInformationTarget::Completed()
	{
	if (!iInProgress)
		{
		return;
		}
	// Finalise response; update number of attributes returned
	iResponse->iNumberAttributes = iResponse->iAttributes.Count();

	//Check the bound of the number of attributes, zero is not permitted
	if (iResponse->iNumberAttributes == 0)
		{
		return SendError(KErrAvrcpMetadataInternalError);
		}
	
	// Allocate a buffer for the formatted message
	RBuf8 messageBuffer;
	if ( messageBuffer.Create(iResponse->Size()) != KErrNone )
		{
		// On OOM drop the message
		iResponse->Close();
		return;
		}
		
	// send the result back to the CT
	TInt error = KErrNone;
	TRAP(error, iResponse->WriteL(messageBuffer));
	if (error == KErrNone)
		{
		InterfaceSelector().SendUnreliable(TUid::Uid(KRemConMediaInformationApiUid),
								EGetElementAttributes, ERemConResponse, messageBuffer);
		}
	
	// Make sure attribute list is reset for next time
	iResponse->Close();
	messageBuffer.Close();
	
	iInProgress = EFalse;
	if (!iMsgQueue.IsEmpty())
		{
		iNextMessageCallBack->CallBack();
		}
	
	}

int CRemConMediaInformationTarget::NextMessageCb(TAny* aThis)
	{
	static_cast<CRemConMediaInformationTarget*>(aThis)->DoNextMessage();
	return KErrNone;
	}

void CRemConMediaInformationTarget::DoNextMessage()
	{
	CRemConMediaInformationQueuedMessage* msg = iMsgQueue.First();
	iMsgQueue.Remove(*msg);
	ProcessMessage(msg->Data());
	delete msg;
	}

EXPORT_C TMediaAttributeIter::TMediaAttributeIter(RArray<TMediaAttributeId>& aMediaAttributeIDs) :
    iMediaAttributeIDs(aMediaAttributeIDs),
    iterator(0)
	{
	}

EXPORT_C void TMediaAttributeIter::Start()
	{
	iterator = 0;
	}

EXPORT_C TBool TMediaAttributeIter::Next(TMediaAttributeId& aId)
	{
	TInt count = iMediaAttributeIDs.Count();
	if (iterator > count - 1)
		{
		return EFalse;
		}
	aId = iMediaAttributeIDs[iterator];
	iterator++;
	return ETrue;
	}

CRemConMediaInformationQueuedMessage* CRemConMediaInformationQueuedMessage::NewL(const TDesC8& aData)
	{
	CRemConMediaInformationQueuedMessage* self = new (ELeave) CRemConMediaInformationQueuedMessage();
	CleanupStack::PushL(self);
	self->ConstructL(aData);
	CleanupStack::Pop(self);
	return self;
	}

CRemConMediaInformationQueuedMessage::CRemConMediaInformationQueuedMessage()
	{
	
	}

void CRemConMediaInformationQueuedMessage::ConstructL(const TDesC8& aData)
	{
	iData.CreateL(aData);
	}

CRemConMediaInformationQueuedMessage::~CRemConMediaInformationQueuedMessage()
	{
	iData.Close();
	}

const TDesC8& CRemConMediaInformationQueuedMessage::Data()
	{
	return iData;
	}



