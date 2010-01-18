/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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



#ifndef BTAPISETTINGS_H
#define BTAPISETTINGS_H

#include <e32cmn.h>
#include <bttypes.h>
#include <btengsettings.h>

#include "testobserver.h"

/**
*  Class for testing BT engine API
*/
NONSHARABLE_CLASS ( CBTApiSettings ): public CBase, public MBTEngSettingsObserver
    {
    public:  // Constructors and destructor

	/**
	* Two-phased constructor.
	*/
	static CBTApiSettings* NewL(MBTTestObserver& aObserver);

	/**
	* Destructor.
	*/
	virtual ~CBTApiSettings();
	
	// Testing functions
	TInt TurnBtOn();
	TInt SwitchPowerState();
    TInt GetPowerState();
    TInt SetVisibilityMode( TBTVisibilityMode visibilityState, TInt aTimer = 0 );
    TInt GetVisibilityMode();
    TInt SetLocalName( TDesC& aName );
    TInt GetLocalName();


    private:

	/** from MBTEngSettingsObserver  */
    void PowerStateChanged( TBTPowerStateValue aState );
    void VisibilityModeChanged( TBTVisibilityMode aState );
    
	/**
	* C++ default constructor.
	*/
	CBTApiSettings( MBTTestObserver& aObserver );

    /**
	* By default Symbian 2nd phase constructor is private.
	*/
	void ConstructL();

    private:

    MBTTestObserver& iObserver;
	CBTEngSettings* iBTEngSettings;
    TBTPowerStateValue iExpectedPowerState;
    TBTVisibilityMode iExpectedVisibility;
    TBuf<100> iExpectedLocalName;
    TInt iError;
    CActiveSchedulerWait iWaiter;
    };

#endif      // BTAPISETTINGS_H

// End of File
