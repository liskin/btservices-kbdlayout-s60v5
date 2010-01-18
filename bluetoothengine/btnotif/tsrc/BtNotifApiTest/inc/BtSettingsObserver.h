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

#ifndef BTSETTINGSOBSERVER_H_
#define BTSETTINGSOBSERVER_H_

#include <e32std.h>
#include <e32base.h>
#include <btengsettings.h>

NONSHARABLE_CLASS( CBtSettingsObserver ) : public CBase,
                                   public MBTEngSettingsObserver
    {
public:
    /**
     * Destructor.
     */
    ~CBtSettingsObserver();

    /**
     * Two-phased constructor.
     */
    static CBtSettingsObserver* NewL();

    /**
     * Two-phased constructor.
     */
    static CBtSettingsObserver* NewLC();
    
    TInt GetPowerState( TBTPowerStateValue& aState );
    TInt SetPowerState( TBTPowerStateValue aState );
    TInt GetVisibilityMode( TBTVisibilityMode& aMode );
    TInt SetVisibilityMode( TBTVisibilityMode aMode, TInt aTime = 0 );
    TInt GetLocalName( TDes& aName );
    TInt SetLocalName( const TDes& aName );

private:

    // From MBTEngSettingsObserver
    void PowerStateChanged( TBTPowerStateValue aState );
    void VisibilityModeChanged( TBTVisibilityMode aState );
    

    CBtSettingsObserver();
    void ConstructL();

    
private: //Data
    
    CBTEngSettings* iSettings; // Own.
    CActiveSchedulerWait* iWaiter; //Own.
    TInt iError;    
    };

#endif /*BTSETTINGSOBSERVER_H_*/
