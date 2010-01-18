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



#ifndef BTAPIDEVMAN_H
#define BTAPIDEVMAN_H

#include <e32cmn.h>
#include <btengdevman.h>

#include "testobserver.h"

/**
*  Class for settign BT Power
*/
NONSHARABLE_CLASS ( CBTApiDevman ): public CBase, public MBTEngDevManObserver
    {
    public:  // Constructors and destructor

	/**
	* Two-phased constructor.
	*/
	static CBTApiDevman* NewL(MBTTestObserver& aObserver);

	/**
	* Destructor.
	*/
	virtual ~CBTApiDevman();
	
	TInt AddDevicesL();
    TInt GetDevicesL();
    TInt ModifyDevicesL();

    private:

	/** from MBTEngDevManObserver  */
    void HandleDevManComplete( TInt aErr );

    void HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray );
    
    // Utility method
    CBTDevice* CreateDeviceL( const TBTDevAddr& aBDAddr,
                              const TDesC8& aDeviceName,
                              const TDesC& aFriendlyName );
    
	/**
	* C++ default constructor.
	*/
	CBTApiDevman( MBTTestObserver& aObserver );

    /**
	* By default Symbian 2nd phase constructor is private.
	*/
	void ConstructL();

    private:

    MBTTestObserver& iObserver;
	CBTEngDevMan* iBTEngDevman;
	CActiveSchedulerWait iWaiter;
	TInt iError;
    };

#endif      // BTAPIDEVMAN_H

// End of File
