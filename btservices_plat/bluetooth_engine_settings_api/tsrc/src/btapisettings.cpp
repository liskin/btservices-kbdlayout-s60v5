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
    iRecoveryPowerState = EBTPowerOff;
    }
    
void CBTApiSettings::PowerStateChanged( TBTPowerStateValue aState )
    {
    iError = KErrNone;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "PowerStateChanged: %d " ), aState);
    
    if ( iExpectedPowerState != aState )
        {
			iError = KErrArgument;
			iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "PowerStateChanged: state %d != expected %d" ), aState, iExpectedPowerState );
        }
    if ( iWaiter.IsStarted() )
        {
		iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "PowerStateChanged: AsyncStop" ) );
        iWaiter.AsyncStop();
        }
    }

void CBTApiSettings::VisibilityModeChanged( TBTVisibilityMode aState )
    {
    iError = KErrNone;
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "VisibilityModeChanged: %d " ), aState );    
    
    if ( iExpectedVisibility != aState )
        {
        iError = KErrArgument;
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "VisibilityModeChanged: state %d != expected %d" ), aState, iExpectedVisibility );
        }
    if ( iWaiter.IsStarted() )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "VisibilityModeChanged: AsyncStop" ) );
        iWaiter.AsyncStop();
        }
    //User::After(2000000);
    }
    
TInt CBTApiSettings::TurnBtOn()
    {
    TBTPowerStateValue state;
    TInt err = KErrNone;
    
    err = iBTEngSettings->GetPowerState( state );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::TurnBtOn: err %d, current state %d expected state %d" ), err, state, iExpectedPowerState );
    if ( err )
        {
        return err;
        }
        
    if( state == EBTPowerOff )
        {
			state = EBTPowerOn;
			iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::TurnBtOn:  turning BT ON: %d (EBTPowerOn) " ), state);
        }
    else if ( state == EBTPowerOn )
        {
			iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::TurnBtOn:  BT is ON. Nothing to do. " ), state);
			return KErrNone;
        }
    else
    	{
			iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::TurnBtOn:  State not allowed: %d ??? " ), state);
			return KErrArgument;
    	}
        
    err = iBTEngSettings->SetPowerState( state ); 
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::TurnBtOn: SetPowerState: err %d, state %d" ), err, state);
    if ( err )
        {
        return err;
        }
        
    iExpectedPowerState = state;
    iWaiter.Start();    
    
    return iError;
    }


TInt CBTApiSettings::TurnBtOff( )
    {
    TBTPowerStateValue state;
    TInt err(KErrNone);
    
    err = iBTEngSettings->GetPowerState( state );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "Current PowerState %d" ), state );
    if ( err )
        {
			return err;
        }

	if ( state == EBTPowerOn )
		{
			state = EBTPowerOff;
		}
	else
		return KErrNone;
		
        
    err = iBTEngSettings->SetPowerState( state );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "SetPowerState: %d" ), state);
    User::After(5000000);
    if ( err )
        {
        return err;
        }
    
    iExpectedPowerState = state;
    iWaiter.Start();    
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "BT turned OFF" ) );   
    
    return iError;
    }

TInt CBTApiSettings::SwitchPowerState()
    {
    TBTPowerStateValue state;
    TInt err = KErrNone;
    
    err = iBTEngSettings->GetPowerState( state );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::SwitchPowerState: err %d, current state %d" ), err, state);
    if ( err )
        {
        return err;
        }
        
    if( state == EBTPowerOff )
        {
        state = EBTPowerOn;
        }
    else if ( state == EBTPowerOn)
        {
        state = EBTPowerOff;
        }
    else
    	{
    	return KErrArgument;
    	}
        
    err = iBTEngSettings->SetPowerState( state );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::SwitchPowerState: err %d, new state %d" ), err, state);    
    if ( err )
        {
        return err;
        }
    iExpectedPowerState = state;     
    iWaiter.Start();
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "Switching BT power: %d" ), state);
    return iError;
    }

TInt CBTApiSettings::SetPowerState( TBTPowerStateValue aState )
	{
	TInt err = KErrNone;

	err = iBTEngSettings->SetPowerState( aState );
	iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::SetPowerState: err %d, set state %d" ), err, aState);    
	if ( err )
		{
		return err;
		}
	iExpectedPowerState = aState;     
	iWaiter.Start();
	iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::SetPowerState: Done" ) );
	
	return iError;	
	}

TInt CBTApiSettings::GetExpectedState( TBTPowerStateValue & aState )
	{		
		aState = iExpectedPowerState;
		return KErrNone;
	}

TInt CBTApiSettings::GetPowerState(TBTPowerStateValue & aState)
    {
    TInt err = KErrNone;
    
    err = iBTEngSettings->GetPowerState( aState );
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::GetPowerState: err %d, state %d" ), err, aState);    
    if ( err )
        {
        return err;
        }
        
    if ( iExpectedPowerState != aState )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiSettings::GetPowerState: state %d != expected %d" ), aState);
        return KErrArgument;
        }
    return err;
    }

TInt CBTApiSettings::SetVisibilityNormalState()
	{
		TInt err = KErrNone;
		TBTVisibilityMode mode;
		
		iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "SetVisibilityNormalState:" ));
		
	 	err = iBTEngSettings->GetVisibilityMode( mode );
	 	if ( err )
	 		{
	 	        return err;
	 		}
	 	
	 	if ( mode != EBTVisibilityModeGeneral )
	 		{
	 			err = iBTEngSettings->SetVisibilityMode( EBTVisibilityModeGeneral );
	 			if ( err )
	 				{
	 					iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "SetVisibilityNormalState: err %d" ), err);
	 			        return err;
	 				}
	 		}
	 	return KErrNone;
	}

TInt CBTApiSettings::SetVisibilityMode( TBTVisibilityMode visibilityState, TInt aTimer )
    {
    TInt err = KErrNone;
    
    err = iBTEngSettings->SetVisibilityMode( visibilityState, aTimer );
     
    if ( err )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "Switching BT visibility: err %d" ), err);
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
    
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "GetVisibilityMode: err %d" ), err);
    if ( err )
        {
        return err;
        }
    
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "GetVisibilityMode: mode %d != %d" ), mode, iExpectedVisibility);    
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

TInt CBTApiSettings::KeepCurrentBtPowerStatus()
	{
		TInt err(KErrNone);
		err = iBTEngSettings->GetPowerState( iRecoveryPowerState );
		iObserver.Logger().Log(CBtTestLogger::ETLDebug, _L( "KeepCurrentBtPowerStatus %d" ), iRecoveryPowerState );
		User::After(5000000);
		if ( err )
			{
				iObserver.Logger().Log(CBtTestLogger::ETLDebug, _L( "KeepCurrentBtPowerStatus: Error %d" ), err );
				return err;
			}
		return err;
	}

TInt CBTApiSettings::RecoverBtPowerStatus()
	{
		TInt err(KErrNone);
		iObserver.Logger().Log(CBtTestLogger::ETLDebug, _L( "RecoverBtPowerStatus %d" ), iRecoveryPowerState );		
		err = iBTEngSettings->SetPowerState( iRecoveryPowerState );
		User::After(5000000);
		if ( err )
			{
				iObserver.Logger().Log(CBtTestLogger::ETLDebug, _L( "RecoverBtPowerStatus: Error %d" ), err );
				return err;
			}
		iExpectedPowerState = iRecoveryPowerState;     
		iWaiter.Start();
		iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "RecoverBtPowerStatus: Done" ) );
		return iError;
	}

TInt CBTApiSettings::GetOfflineModeSettings(TCoreAppUIsNetworkConnectionAllowed & aOffline, TBTEnabledInOfflineMode & aOfflineAllowed)
	{
		TInt err(KErrNone);
		
		err = iBTEngSettings->GetOfflineModeSettings( aOffline, aOfflineAllowed );
		iObserver.Logger().Log(CBtTestLogger::ETLDebug, _L( "Offline: %d  OfflineAllowed %d" ), aOffline, aOfflineAllowed );
		return err;
	}

TInt CBTApiSettings::ChangePowerStateTemporarily()
	{
		TInt err(KErrNone);
	/*	
		err = KeepCurrentBtPowerStatus();
		if ( err )
			{
				return err;
			}
		
		err = TurnBtOff();
		if ( err )
			{
				return err;
			}
		*/
		err = iBTEngSettings->ChangePowerStateTemporarily();
		if( err)
			{
				iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "ChangePowerStateTemporarily: Error" ) );
				return err;
			}
		iWaiter.Start();
		iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "ChangePowerStateTemporarily: Done" ) );
		/*
		err = RecoverBtPowerStatus();
		if ( err )
			{
				return err;
			}
		*/
		return err;
	}

