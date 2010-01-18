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


#include <btengdevman.h>

#include "btapidevman.h"
#include "bttestlogger.h"

_LIT8(KTestDeviceName, "TestDeviceName");
_LIT8(KTestDeviceName2, "TestDeviceName2");
_LIT8(KUpdatedTestDeviceName, "UpdatedTestDeviceName");
_LIT(KTestDeviceFriendlyName, "TestDeviceFriendlyName");
_LIT(KTestDeviceFriendlyName2, "TestDeviceFriendlyName2");

#define HighTestAddr 0x00e0
#define LowTestAddr 0x0370cb9c
#define HighTestAddr2 0x00e2
#define LowTestAddr2 0x0370ac8c


// Constructor
CBTApiDevman::CBTApiDevman( MBTTestObserver& aObserver )
    : iObserver( aObserver )
    {
    }

// Destructor
CBTApiDevman::~CBTApiDevman( )
    {
    delete iBTEngDevman;
    }


// NewL
CBTApiDevman* CBTApiDevman::NewL ( MBTTestObserver& aObserver )
    {
    CBTApiDevman* self = new ( ELeave ) CBTApiDevman( aObserver );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }


// Symbian 2nd phase constructor.
void CBTApiDevman::ConstructL()
    {
    iBTEngDevman = CBTEngDevMan::NewL( this );
    iError = KErrNone;
    }
    
void CBTApiDevman::HandleDevManComplete( TInt aErr )
    {
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiDevman::HandleDevManComplete: %d" ), aErr );
    MBTEngDevManObserver::HandleDevManComplete( aErr );
    iError = aErr;
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        }
    }

void CBTApiDevman::HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray )
    {
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiDevman::HandleGetDevicesComplete: %d" ), aErr );
    MBTEngDevManObserver::HandleGetDevicesComplete( aErr, aDeviceArray );
    iError = aErr;
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        };
    }

TInt CBTApiDevman::AddDevicesL()
    {
    TInt err = KErrNone;
    CBTDevice* device = NULL;
    CBTDevice* device2 = NULL;
    TBuf8<50> deviceName(KTestDeviceName());
    TBuf8<50> deviceName2(KTestDeviceName2());
    TBuf<50> friendlyName(KTestDeviceFriendlyName());
    TBuf<50> friendlyName2(KTestDeviceFriendlyName2());
    TBTDevAddr testAddress(MAKE_TINT64(HighTestAddr, LowTestAddr));
    TBTDevAddr testAddress2(MAKE_TINT64(HighTestAddr2, LowTestAddr2));
    TBTRegistrySearch criteria;
    iError = KErrNone;
    
    device = CreateDeviceL( testAddress, deviceName, friendlyName);
    CleanupStack::PushL( device );
    
    device2 = CreateDeviceL( testAddress2, deviceName2, friendlyName2);
    CleanupStack::PushL( device2 );
    
    criteria.FindAddress( testAddress );

    err = iBTEngDevman->DeleteDevices( criteria );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        CleanupStack::PopAndDestroy( 2, device );
        return err;
        }
        
    criteria.FindAddress( testAddress2 );

    err = iBTEngDevman->DeleteDevices( criteria );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        CleanupStack::PopAndDestroy( 2, device );
        return err;
        }
       
    err = iBTEngDevman->AddDevice( *device );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        CleanupStack::PopAndDestroy( 2, device );
        return err;
        }
        
    // Check the callback error code
    if ( iError )
        {
         CleanupStack::PopAndDestroy( 2, device );
         return iError;
        }
        
    err = iBTEngDevman->AddDevice( *device2 );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        CleanupStack::PopAndDestroy( 2, device );
        return err;
        }
    CleanupStack::PopAndDestroy( 2, device );
    
    // Check the callback error code
    if ( iError )
        {
         return iError;
        }
    return err;
    }

TInt CBTApiDevman::GetDevicesL()
    {
    TInt err = KErrNone;
    TBTDevAddr testAddress(MAKE_TINT64( HighTestAddr, LowTestAddr ) );
    TBTDevAddr testAddress2(MAKE_TINT64( HighTestAddr2, LowTestAddr2 ) );
    TBTRegistrySearch criteria;
    CBTDeviceArray* deviceArray = NULL;
    iError = KErrNone;

    deviceArray = new (ELeave) CBTDeviceArray( 1 );
    criteria.FindAddress( testAddress );
    err = iBTEngDevman->GetDevices( criteria, deviceArray ); // asynchronous
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        deviceArray->ResetAndDestroy();
        delete deviceArray;
        return err;
        }
    
    deviceArray->ResetAndDestroy();
    
    // Check the callback error code
    if ( iError )
        {
        delete deviceArray;
        return iError;
        }
    
    criteria.FindAddress( testAddress2 );
    err = iBTEngDevman->GetDevices( criteria, deviceArray ); // asynchronous
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        deviceArray->ResetAndDestroy();
        delete deviceArray;
        return err;
        }
        
    deviceArray->ResetAndDestroy();
    
    // Check the callback error code
    if ( iError )
        {
        delete deviceArray;
        return iError;
        }
    
    criteria.FindAll();
    err = iBTEngDevman->GetDevices( criteria, deviceArray ); // asynchronous
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        deviceArray->ResetAndDestroy();
        delete deviceArray;
        return err;
        }
        
    deviceArray->ResetAndDestroy();
    
    // Check the callback error code
    if ( iError )
        {
        delete deviceArray;
        return iError;
        }
    
    // Test the synchronous version of GetDevices
    CBTEngDevMan* devman = CBTEngDevMan::NewL( NULL );

    criteria.FindAddress( testAddress );
    err = devman->GetDevices( criteria, deviceArray ); // synchronous
    if ( err )
        {
        
        }
        
    deviceArray->ResetAndDestroy();
    delete deviceArray;
    delete devman;
    
    return err;
    }
    
TInt CBTApiDevman::ModifyDevicesL()
    {
    TInt err = KErrNone;
    CBTDevice* device = NULL;
    TBuf8<50> updatedDeviceName(KUpdatedTestDeviceName());
    TBuf<50> friendlyName(KTestDeviceFriendlyName());
    TBTDevAddr testAddress(MAKE_TINT64(HighTestAddr, LowTestAddr));
    iError = KErrNone;
    
    device = CreateDeviceL( testAddress, updatedDeviceName, friendlyName );
    CleanupStack::PushL( device );
    
    err = iBTEngDevman->ModifyDevice( *device );
    if ( !err )
        {
        iWaiter.Start();
        }
        
    CleanupStack::PopAndDestroy( device );
    
    if ( iError )
        {
        return iError;
        }
    return err;
    }
    
CBTDevice* CBTApiDevman::CreateDeviceL(const TBTDevAddr& aBDAddr, const TDesC8& aDeviceName, const TDesC& aFriendlyName)
    {
    CBTDevice* newDevice = CBTDevice::NewLC( aBDAddr );
    newDevice->SetDeviceNameL( aDeviceName );
	newDevice->SetFriendlyNameL( aFriendlyName );
	CleanupStack::Pop();
	return newDevice;
    };