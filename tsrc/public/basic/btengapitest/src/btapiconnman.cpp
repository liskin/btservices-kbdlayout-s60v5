/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Used for testing BT API
*  Revision    : $Revision: $
*  Date        : $Date: $
*
*/


#include <btengconnman.h>
#include <btengconstants.h>

#include "btapiconnman.h"
#include "bttestlogger.h"

// Length of readable BT address
const TInt KBTAddrBufSize = 2 * KBTDevAddrSize;
// Typedef-ed buffer for logging readable BT address
typedef TBuf<KBTAddrBufSize> TBTAddrBuf;

// Constructor
CBTApiConnman::CBTApiConnman( MBTTestObserver& aObserver )
    : iObserver( aObserver )
    {
    }

// Destructor
CBTApiConnman::~CBTApiConnman( )
    {
    delete iBTEngConnMan;
    }


// NewL
CBTApiConnman* CBTApiConnman::NewL (MBTTestObserver& aObserver)
    {
    CBTApiConnman* self = new (ELeave) CBTApiConnman(aObserver);

    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }


// Symbian 2nd phase constructor.
void CBTApiConnman::ConstructL()
    {
    iBTEngConnMan = CBTEngConnMan::NewL( this );
    iError = KErrNone;
    iBTEngConnMan->RemoveObserver();
    iBTEngConnMan->SetObserver( this );
    }

void CBTApiConnman::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, RBTDevAddrArray* /*aConflicts*/ )
    {
    TBTAddrBuf buf;
    aAddr.GetReadable( buf );
    iError = aErr;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiConnman.ConnectComplete %S, %d" ), &buf, aErr );
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        }
    }

void CBTApiConnman::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
    {
    TBTAddrBuf buf;
    aAddr.GetReadable( buf );
    iError = aErr;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiConnman.DisconnectComplete %S, %d" ), &buf, aErr );
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        }
    }
    
void CBTApiConnman::PairingComplete( TBTDevAddr& aAddr, TInt aErr )
    {
    TBTAddrBuf buf;
    aAddr.GetReadable( buf );
    iError = aErr;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiConnman.PairingComplete %S, %d" ), &buf, aErr );
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        }
    }

// Sets BT power according to aPowerState
TInt CBTApiConnman::ConnectIfNotConnected( TBTDevAddr& aAddr, TBTDeviceClass& aCod )
    {
    TInt err = KErrNone;
    TBTEngConnectionStatus connected;    
    TBTAddrBuf buf;
    aAddr.GetReadable( buf );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "ConnectIfNotConnected ongoing %S" ),  &buf);
    
    err = iBTEngConnMan->IsConnected( aAddr, connected );
    
    if ( err )
        {
        return err;
        }
    if ( connected != EBTEngNotConnected )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "Wrong parameter: connected != EBTEngNotConnected" ) );
        // return KErrNone;;
        }
        
    err = iBTEngConnMan->Connect( aAddr, aCod );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
    return /*iError*/ KErrNone;
	}
	
TInt CBTApiConnman::DisconnectIfConnected( TBTDevAddr& aAddr )
    {
    TInt err = KErrNone;
    TBTAddrBuf buf;
    TBTEngConnectionStatus connected;
    aAddr.GetReadable( buf );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "DisconnectIfConnected ongoing_op %S" ),  &buf );        
    
    err = iBTEngConnMan->IsConnected( aAddr, connected );
    
    if ( err )
        {
        return err;
        }
    if ( connected != EBTEngConnected )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "Wrong parameter: connected != EBTEngConnected" ) );
        // return KErrNone;
        }
    
    err = iBTEngConnMan->Disconnect( aAddr, EBTDiscGraceful );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
    return /*iError*/ KErrNone;
	}
	
TInt CBTApiConnman::ConnectAndCancel( TBTDevAddr& aAddr, TBTDeviceClass& aCod )
    {
    TInt err = KErrNone;
    TBTEngConnectionStatus connected;
    TBTAddrBuf buf;
    aAddr.GetReadable( buf );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "ConnectAndCancel ongoing_op %S" ),  &buf);
          
    err = iBTEngConnMan->IsConnected( aAddr, connected );
    if ( err )
        {
        return err;
        }
        
    if ( connected == EBTEngNotConnected )
        {
        // Conncet first
        err = iBTEngConnMan->Connect( aAddr, aCod );
        if ( err )
            {
            return err;
            }
        }

    iBTEngConnMan->CancelConnect( aAddr ); 
    return KErrNone;
	}
	
TInt CBTApiConnman::GetAddresses( TBTDevAddr& aAddr )
    {
    TInt err = KErrNone;
    TBTDeviceClass cod(EMajorServiceAudio, 0, 0);
    RBTDevAddrArray addrArray;

    err = iBTEngConnMan->GetConnectedAddresses( addrArray, EBTProfileHFP  );
    if ( err )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "GetConnectedAddresses: error %d" ), err ); 
        }
    err = iBTEngConnMan->GetConnectedAddresses( addrArray );
    if ( err )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "GetConnectedAddresses: error %d" ), err ); 
        }

/*
    err = iBTEngConnMan->Connect( aAddr, cod );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
    
    err = iBTEngConnMan->GetConnectedAddresses( addrArray );
    
    if ( err )
        {
        return err;
        }
    
    if ( addrArray.Count() < 1 )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "GetConnectedAddresses: wrong value: %d" ), addrArray.Count() ); 
        // return KErrNotFound;
        }
    
    iBTEngConnMan->Disconnect( aAddr, EBTDiscImmediate );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
    
    err = iBTEngConnMan->GetConnectedAddresses( addrArray );
    
    if ( err )
        {
        return err;
        }
    
    if ( addrArray.Count() > 0 )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "GetConnectedAddresses: wrong value: %d" ), addrArray.Count() );          
        // return KErrNotFound;
        }
*/        
    return err;
	}
	
TInt CBTApiConnman::IsConnectable()
    {
    TInt err = KErrNone;
    TBTDeviceClass audioCoD( EMajorServiceAudio, 0, 0 );
    TBTDeviceClass invalidCoD( EMajorServiceTelephony, 0, 0 );
    TBool connectable = EFalse;

    err = iBTEngConnMan->IsConnectable( audioCoD, connectable );
    if ( err || !connectable )
        {
        return KErrArgument;
        }
    
    err = iBTEngConnMan->IsConnectable( invalidCoD, connectable );
    if ( err || connectable )
        {
        return KErrArgument;
        }
    return err;
	}

TInt CBTApiConnman::PairDevice( TBTDevAddr& aAddr )
    {
    TInt err = KErrNone;
    TInt err2 = KErrNone;

    err = iBTEngConnMan->PairDevice( aAddr, EBTProfileHFP );
    iBTEngConnMan->CancelPairDevice();
    
    err = iBTEngConnMan->PairDevice( aAddr );
    err2 = iBTEngConnMan->StartPairingObserver( aAddr );

    if ( !err && !err2 )
        {
        iWaiter.Start();
        }
    else
        {
        if ( err )
            {
            return err;
            }
        else
            {
            return err2;
            }
        }
    if ( iError )
        {
        return iError;
        }
        
    return iError;
	}

TInt CBTApiConnman::PrepareDiscovery( TBTDevAddr& aAddr )
    {
    TInt err = KErrNone;
    iBTEngConnMan->PrepareDiscovery();
    return err;
	}