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



#ifndef BTAPICONNMAN_H
#define BTAPICONNMAN_H

#include <e32cmn.h>
#include <btengconnman.h>

#include "testobserver.h"

/**
*  Class for testing BT engine API
*/
NONSHARABLE_CLASS ( CBTApiConnman ): public CBase, public MBTEngConnObserver
    {
    public:  // Constructors and destructor

	/**
	* Two-phased constructor.
	*/
	static CBTApiConnman* NewL(MBTTestObserver& aObserver);

	TInt ConnectIfNotConnected( TBTDevAddr& aAddr, TBTDeviceClass& aCod );
    TInt DisconnectIfConnected( TBTDevAddr& aAddr );
    TInt ConnectAndCancel( TBTDevAddr& aAddr, TBTDeviceClass& aCod );
    TInt GetAddresses( TBTDevAddr& aAddr );
    TInt IsConnectable();
    TInt PairDevice( TBTDevAddr& aAddr );
    TInt PrepareDiscovery( TBTDevAddr& aAddr );

	/**
	* Destructor.
	*/
	virtual ~CBTApiConnman();

    private:

	/** from MBTEngConnObserver  */
    void ConnectComplete( TBTDevAddr& aAddr, TInt aErr, RBTDevAddrArray* aConflicts = NULL );
    
    void DisconnectComplete( TBTDevAddr& aAddr, TInt aErr );
    
    void PairingComplete( TBTDevAddr& aAddr, TInt aErr );
    
	/**
	* C++ default constructor.
	*/
	CBTApiConnman( MBTTestObserver& aObserver );

    /**
	* By default Symbian 2nd phase constructor is private.
	*/
	void ConstructL();

    private:

    MBTTestObserver& iObserver;
	CBTEngConnMan* iBTEngConnMan;
    TInt iError;
    CActiveSchedulerWait iWaiter;
    };

#endif      // BTAPICONNMAN_H

// End of File
