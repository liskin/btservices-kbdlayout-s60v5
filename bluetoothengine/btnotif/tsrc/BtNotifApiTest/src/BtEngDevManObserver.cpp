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

#include "BtEngDevManObserver.h"

CBtEngDevManObserver::CBtEngDevManObserver()
    {
    
    }

CBtEngDevManObserver::~CBtEngDevManObserver()
    {
    if( iWait->IsStarted() )
        {
        iWait->AsyncStop();
        }
    delete iWait;
    iWait = NULL;
    delete iDevMan;
    iDevMan = NULL;
    }

CBtEngDevManObserver* CBtEngDevManObserver::NewLC()
    {
    CBtEngDevManObserver* self = new ( ELeave ) CBtEngDevManObserver();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CBtEngDevManObserver* CBtEngDevManObserver::NewL()
    {
    CBtEngDevManObserver* self = CBtEngDevManObserver::NewLC();
    CleanupStack::Pop(); // self;
    return self;
    }

void CBtEngDevManObserver::ConstructL()
    {
    iWait = new ( ELeave ) CActiveSchedulerWait();
    iDevMan = CBTEngDevMan::NewL( this );
    }

TInt CBtEngDevManObserver::GetDevices( const TBTRegistrySearch& aCriteria, 
                               CBTDeviceArray* aResultArray )
    {
    TInt res;
    res = iDevMan -> GetDevices( aCriteria, aResultArray );
    if ( res != KErrNone )
        {
        return res;
        }
        
    iWait -> Start();
    
    return KErrNone;
    }

TInt CBtEngDevManObserver::AddDevice( const CBTDevice& aDevice )
    {
    TInt res;
    res = iDevMan -> AddDevice( aDevice );
    if ( res != KErrNone )
        {
        return res;
        }
        
    iWait -> Start();
    
    return KErrNone;
    }

TInt CBtEngDevManObserver::DeleteDevices( const TBTRegistrySearch& aCriteria )
    {
    TInt res;
    res = iDevMan -> DeleteDevices( aCriteria );
    if ( res != KErrNone )
        {
        return res;
        }
        
    iWait -> Start();
    
    return KErrNone;
    }

void CBtEngDevManObserver::HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray )
    {
    iWait -> AsyncStop();
    }

void CBtEngDevManObserver::HandleDevManComplete( TInt aErr )
    {
    iWait -> AsyncStop();
    }
