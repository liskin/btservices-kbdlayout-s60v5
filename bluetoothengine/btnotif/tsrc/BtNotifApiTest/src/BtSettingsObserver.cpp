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

#include "BtSettingsObserver.h"

CBtSettingsObserver::CBtSettingsObserver()
    {
    }

CBtSettingsObserver::~CBtSettingsObserver()
    {
    delete iSettings;
    if( iWaiter->IsStarted() )
        {
        iWaiter->AsyncStop();
        }
    delete iWaiter;
    }

CBtSettingsObserver* CBtSettingsObserver::NewLC()
    {
    CBtSettingsObserver* self = new ( ELeave ) CBtSettingsObserver();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CBtSettingsObserver* CBtSettingsObserver::NewL()
    {
    CBtSettingsObserver* self = CBtSettingsObserver::NewLC();
    CleanupStack::Pop(); // self;
    return self;
    }

void CBtSettingsObserver::ConstructL()
    {
    iWaiter = new ( ELeave ) CActiveSchedulerWait();
    iSettings = CBTEngSettings::NewL( this );
    iError = KErrNone;
    }

void CBtSettingsObserver::PowerStateChanged( TBTPowerStateValue /*aState*/ )
    {
    if( iWaiter->IsStarted() )
        {
        iWaiter->AsyncStop();
        }
    }

void CBtSettingsObserver::VisibilityModeChanged( TBTVisibilityMode /*aState*/ )
    {
    if( iWaiter->IsStarted() )
        {
        iWaiter->AsyncStop();
        }
    }

TInt CBtSettingsObserver::GetPowerState( TBTPowerStateValue& aState )
    {
    iError = iSettings->GetPowerState( aState );
    return iError;
    }

TInt CBtSettingsObserver::SetPowerState( TBTPowerStateValue aState )
    {
    TBTPowerStateValue state;
    
    iError = iSettings->GetPowerState( state );
    if( iError )
        {
        return iError;
        }
    else if( state != aState )
            {
            if( state == EBTPowerOff )
                {
                iError = iSettings->SetPowerState( EBTPowerOn );
                }
            else
                {
                iError = iSettings->SetPowerState( EBTPowerOff );
                }
            
            if( iError )
                {
                return iError;
                }
            else
                {
                if ( !iWaiter->IsStarted() )
                    {
                    iWaiter->Start();
                    }
                }
            }
    return iError;
    }

TInt CBtSettingsObserver::GetVisibilityMode( TBTVisibilityMode& /*aMode*/ )
    {
    return KErrNone;
    }

TInt CBtSettingsObserver::SetVisibilityMode( TBTVisibilityMode /*aMode*/, TInt /*aTime*/ )
    {
    return KErrNone;
    }

TInt CBtSettingsObserver::GetLocalName( TDes& aName )
    {
    iError = iSettings -> GetLocalName( aName );    
    return iError;
    }

TInt CBtSettingsObserver::SetLocalName( const TDes& aName )
    {
    iError = iSettings -> SetLocalName( aName );    
    return iError;
    }
