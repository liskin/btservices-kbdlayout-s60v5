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
#include <remcongroupnavigationtarget.h>
#include <remcongroupnavigationtargetobserver.h>

#include <remconinterfaceselector.h>

#include <avcframe.h>

#include "remcongroupnavigation.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_GROUP_NAVIGATION);
#endif

EXPORT_C CRemConGroupNavigationApiTarget* CRemConGroupNavigationApiTarget::NewL(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConGroupNavigationTargetObserver& aObserver)
	{
	LOG_STATIC_FUNC

	CRemConGroupNavigationApiTarget* self = new(ELeave) CRemConGroupNavigationApiTarget(aInterfaceSelector, aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CRemConGroupNavigationApiTarget* CRemConGroupNavigationApiTarget::NewL(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConGroupNavigationTargetObserver& aObserver,TBool aNextGroupSupported, TBool aPreviousGroupSupported)
	{
	LOG_STATIC_FUNC

	CRemConGroupNavigationApiTarget* self = new(ELeave) CRemConGroupNavigationApiTarget(aInterfaceSelector, aObserver);
	CleanupStack::PushL(self);
	self->ConstructL(aNextGroupSupported,aPreviousGroupSupported);
	CleanupStack::Pop(self);
	return self;
	}
CRemConGroupNavigationApiTarget::CRemConGroupNavigationApiTarget(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConGroupNavigationTargetObserver& aObserver)
:	CRemConInterfaceBase(TUid::Uid(KRemConGroupNavigationApiUid), 
						 KMaxSizeRemConGroupNavigationMsg, 
						 aInterfaceSelector,
						 ERemConClientTypeTarget), 
	iObserver(aObserver), iPtr(0,0,0)
	{
	}
	
void CRemConGroupNavigationApiTarget::ConstructL()
	{
	// Create a output buffer (arbitrary value of 32 for granularity of buffer expansion)
	iOutData = CBufFlat::NewL(32);
	iOutData->SetReserveL(sizeof(TPckg<TRemConGroupNavigationResponse>)); 
	BaseConstructL();
	}
void CRemConGroupNavigationApiTarget::ConstructL(TBool aNextGroupSupported, TBool aPreviousGroupSupported)
	{
	// Create a output buffer (arbitrary value of 32 for granularity of buffer expansion)
	iOutData = CBufFlat::NewL(32);
	iOutData->SetReserveL(sizeof(TPckg<TRemConGroupNavigationResponse>));
	
	RRemConInterfaceFeatures features;
	User::LeaveIfError(features.Open());
	CleanupClosePushL(features);
	
	if(aNextGroupSupported)
		{
		features.AddOperationL(ENextGroup);
		}
	
	if(aPreviousGroupSupported)
		{
		features.AddOperationL(EPreviousGroup);
		}
	
	BaseConstructL(features);
	CleanupStack::PopAndDestroy(&features);
	}
	

/** Destructor.

@publishedAll
@released
*/
EXPORT_C CRemConGroupNavigationApiTarget::~CRemConGroupNavigationApiTarget()
	{
	delete iOutData;
	iStream.Close();
	}

/** Gets a pointer to a specific interface version.

@return A pointer to the interface, NULL if not supported.
@internalComponent
@released
*/
TAny* CRemConGroupNavigationApiTarget::GetInterfaceIf(TUid aUid)
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

EXPORT_C void CRemConGroupNavigationApiTarget::NextGroupResponse(TRequestStatus*& aStatus, TInt aError)
	{
	SendGroupResponse(ENextGroup, aStatus, aError);
	}

EXPORT_C void CRemConGroupNavigationApiTarget::PreviousGroupResponse(TRequestStatus*& aStatus, TInt aError)
	{
	SendGroupResponse(EPreviousGroup, aStatus, aError);
	}

void CRemConGroupNavigationApiTarget::SendGroupResponse(TGroupNavigationPassthroughOperationIds aId, TRequestStatus*& aStatus, TInt aError)
	{
	// format data to be sent
  	TRAPD(error, iOutData->ResizeL(sizeof(TRemConGroupNavigationResponse)));
  	if (error != KErrNone)
  		{
  		// if this fails, then silently drop response
  		return;
  		}
  		
	TRemConGroupNavigationResponse response;
	response.iError = aError;
	iOutData->Write(0,TPckg<TRemConGroupNavigationResponse>(response));
	iPtr.Set(iOutData->Ptr(0));

	// if a TRequestStatus has been supplied
	if (aStatus)
		{
		InterfaceSelector().Send(*aStatus, TUid::Uid(KRemConGroupNavigationApiUid), aId, iNumRemotes, ERemConResponse, iPtr);
		}
	else
		{
		InterfaceSelector().SendUnreliable(TUid::Uid(KRemConGroupNavigationApiUid), aId, ERemConResponse, iPtr);
		}
	}

// from MRemConInterfaceIf
void CRemConGroupNavigationApiTarget::MrcibNewMessage(TUint aOperationId, const TDesC8& aData)
	{
	LOG1(_L("\taOperationId = 0x%02x"), aOperationId);
	LOG1(_L("\taData.Length = %d"), aData.Length());

	/* Decode the get GroupNavigation message */
	TGroupNavigationPassthroughOperationIds currentOp = static_cast<TGroupNavigationPassthroughOperationIds>(aOperationId);
  	TRemConCoreApiButtonAction button = static_cast<TRemConCoreApiButtonAction>(aData.Ptr()[0]);

	switch (currentOp)
		{
		case ENextGroup:
			{
			iObserver.MrcgntoNextGroup( button );
			break;
			}
		case EPreviousGroup:
			{
			iObserver.MrcgntoPreviousGroup( button );
			break;
			}		
		default:
			break;

		}
	}

