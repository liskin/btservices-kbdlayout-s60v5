/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/


#ifndef BTENGDEVMANOBSERVER_H_
#define BTENGDEVMANOBSERVER_H_

#include "BTEngDevMan.h"

NONSHARABLE_CLASS( CBtEngDevManObserver ) : public MBTEngDevManObserver
{
public:
	static CBtEngDevManObserver* NewL();
	static CBtEngDevManObserver* NewLC();
	
	TInt GetDevices( const TBTRegistrySearch& aCriteria, 
                               CBTDeviceArray* aResultArray );
	TInt AddDevice( const CBTDevice& aDevice );
	TInt DeleteDevices( const TBTRegistrySearch& aCriteria );
    
	virtual ~CBtEngDevManObserver();
    void HandleDevManComplete( TInt aErr );
    void HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray );
	
private:
    CBtEngDevManObserver();
    void ConstructL();
    
private:
    CActiveSchedulerWait* iWait;
    CBTEngDevMan* iDevMan;

};

#endif /*BTENGDEVMANOBSERVER_H_*/
