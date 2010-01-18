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

#include <e32std.h>
#include <e32base.h>
#include <e32cmn.h>
#include <btengsettings.h>

#include "btapisettings.h"
#include "bttestlogger.h"

// Log string for Power on
_LIT( KSetBTPowerOn, "on " );
// Log string for Power off
_LIT( KSetBTPowerOff, "off" );

// Constructor
CBTApiSettings::CBTApiSettings( MBTTestObserver& aObserver )
    : iObserver( aObserver )
    {
    }

// Destructor
CBTApiSettings::~CBTApiSettings( )
    {
    delete iBTEngSettings;
    }


// NewL
CBTApiSettings* CBTApiSettings::NewL (MBTTestObserver& aObserver)
    {
    CBTApiSettings* self = new (ELeave) CBTApiSettings(aObserver);

    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }


// Symbian 2nd phase constructor.
void CBTApiSettings::ConstructL()
    {
    iBTEngSettings = CBTEngSettings::NewL( this );
    iExpectedPowerState = EBTPowerOff;
    iExpectedVisibility = EBTVisibilityModeGeneral;
    iError = KErrNone;
    iExpectedLocalName.Zero();
    }
    
void CBTApiSettings::PowerStateChanged( TBTPowerStateValue aState )
    {
    iError = KErrNone;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::PowerStateChanged: %d" ), aState);
    
    if ( iExpectedPowerState != aState )
        {
        iError = KErrArgument;
        }
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        }
    }
    
void CBTApiSettings::VisibilityModeChanged( TBTVisibilityMode aState )
    {
    iError = KErrNone;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::VisibilityModeChanged: %d" ), aState);    
    
    if ( iExpectedVisibility != aState )
        {
        iError = KErrArgument;
        }
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        }
    }
    
TInt CBTApiSettings::TurnBtOn()
    {
    TBTPowerStateValue state;
    TInt err = KErrNone;
    
    err = iBTEngSettings->GetPowerState( state );
    if ( err )
        {
        return err;
        }
        
    if( state == EBTPowerOff )
        {
        state = EBTPowerOn;
        }
    else
        {
        return KErrNone;
        }
        
    err = iBTEngSettings->SetPowerState( state ); 
    if ( err )
        {
        return err;
        }
        
    iExpectedPowerState = state;
    iWaiter.Start();
    return iError;
    }

TInt CBTApiSettings::SwitchPowerState()
    {
    TBTPowerStateValue state;
    TInt err = KErrNone;
    
    err = iBTEngSettings->GetPowerState( state );
    if ( err )
        {
        return err;
        }
        
    if( state == EBTPowerOff )
        {
        state = EBTPowerOn;
        }
    else
        {
        state = EBTPowerOff;
        }
        
    err = iBTEngSettings->SetPowerState( state );
    
    if ( err )
        {
        return err;
        }
    iExpectedPowerState = state;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "Switching BT power: %d" ), state); 
    iWaiter.Start();
    return iError;
    }

TInt CBTApiSettings::GetPowerState()
    {
    TInt err = KErrNone;
    TBTPowerStateValue state;
    
    err = iBTEngSettings->GetPowerState( state );
    
    if ( err )
        {
        return err;
        }
        
    if ( iExpectedPowerState != state )
        {
        return KErrArgument;
        }
    return err;
    }
    
TInt CBTApiSettings::SetVisibilityMode( TBTVisibilityMode visibilityState, TInt aTimer )
    {
    TInt err = KErrNone;
    err = iBTEngSettings->SetVisibilityMode( visibilityState );
    
    if ( err )
        {
        return err;
        }
    iExpectedVisibility = visibilityState;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "Switching BT visibility: %d" ), visibilityState); 
    iWaiter.Start();
    return iError;
    }
    
TInt CBTApiSettings::GetVisibilityMode()
    {
    TInt err = KErrNone;
    TBTVisibilityMode mode;
    
    err = iBTEngSettings->GetVisibilityMode( mode );
    
    if ( err )
        {
        return err;
        }
        
    if ( iExpectedVisibility!= mode )
        {
        return KErrArgument;
        }
        
    return iError;
    }
    
TInt CBTApiSettings::SetLocalName( TDesC& aName )
    {
    TInt err = KErrNone;
    iExpectedLocalName = aName;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "Setting BT localname to: %S" ), &aName); 
    err = iBTEngSettings->SetLocalName( iExpectedLocalName );
    if ( err )
        {
        iExpectedLocalName.Zero();
        }
    return err;
    }
    
TInt CBTApiSettings::GetLocalName()
    {
    TInt err = KErrNone;
    TBuf<100> name;
    err = iBTEngSettings->GetLocalName( name );
    
    if ( iExpectedLocalName.Length() )
        {
        if ( name.Compare( iExpectedLocalName ) )
            {
            err = KErrArgument;
            }
        }
    return err;
    }

