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

#ifndef REMCONBATTERYTARGET_H
#define REMCONBATTERYTARGET_H

#include <e32base.h>
#include <remcon/remconinterfacebase.h>
#include <remcon/remconinterfaceif.h>

class MRemConBatteryTargetObserver;
class MRemConCoreApiTargetObserver;
class CRemConInterfaceSelector;

/**
Client-instantiable type supporting sending Battery Status responses.
*/
NONSHARABLE_CLASS(CRemConBatteryApiTarget) : public CRemConInterfaceBase, 
								          		public MRemConInterfaceIf
	{
public:
	/**
	Factory function.
	@see CRemConInterfaceSelector
	@see MRemConBatteryTargetObserver
	@param aInterfaceSelector The interface selector. The client must have 
	created one of these first.
	@param aObserver The observer of this interface.
	@return A new CRemConBatteryApiTarget, owned by the interface selector.
	*/
	IMPORT_C static CRemConBatteryApiTarget* NewL(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConBatteryTargetObserver& aObserver);
	
	/** Destructor */
	IMPORT_C ~CRemConBatteryApiTarget();

private:
	/** 
	Constructor.
	@param aInterfaceSelector The interface selector.
	@param aObserver The observer of this interface.
	*/
	CRemConBatteryApiTarget(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConBatteryTargetObserver& aObserver);

	void ConstructL();

private: // from CRemConInterfaceBase
	TAny* GetInterfaceIf(TUid aUid);

private: // from MRemConInterfaceIf
	void MrcibNewMessage(TUint aOperationId, const TDesC8& aData);

private: // unowned
	MRemConBatteryTargetObserver& iObserver;

private: // owned
	CBufFlat*       iOutData;
	
	
	};

#endif // REMCONBATTERYTARGET_H
