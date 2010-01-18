// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef REMCONMEDIAINFORMATIONTARGETOBSERVER_H
#define REMCONMEDIAINFORMATIONTARGETOBSERVER_H

#include <e32base.h>
#include <remconmediaattributeid.h>

/**
Clients must implement this interface in order to instantiate objects of type 
CRemConMediaInformationTarget. This interface passes incoming commands from RemCon to 
the client. 
*/
NONSHARABLE_CLASS(MRemConMediaInformationTargetObserver)
	{
public:

	/**
	For each element in aAttributeList for which the requested Metadata is available,
	the client shall respond by calling CRemConMediaInformationTarget::AttributeValue(). 
	After all attributes have been supplied the client shall call CRemConMediaInformationTarget::Completed().
	@param aAttributeIter A CMediaAttributeIDs containing a list of TMediaAttributeIds requested by the controller
	@see CRemConMediaInformationTarget::AttributeValue()
	@see CRemConMediaInformationTarget::Completed()
	*/
	virtual void MrcmitoGetCurrentlyPlayingMetadata( TMediaAttributeIter& aAttributeIter ) = 0;

	};

#endif // REMCONMEDIAINFORMATIONTARGETOBSERVER_H
