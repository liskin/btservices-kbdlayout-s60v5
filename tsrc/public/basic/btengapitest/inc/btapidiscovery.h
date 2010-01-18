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



#ifndef BTAPIDISCOVERY_H
#define BTAPIDISCOVERY_H

#include <e32cmn.h>
#include <btengdiscovery.h>

#include "testobserver.h"

/**
*  Class for testing BT engine API
*/
NONSHARABLE_CLASS ( CBTApiDiscovery ): public CBase, public MBTEngSdpResultReceiver
    {
    public:  // Constructors and destructor

	/**
	* Two-phased constructor.
	*/
	static CBTApiDiscovery* NewL(MBTTestObserver& aObserver);

	/**
	* Destructor.
	*/
	virtual ~CBTApiDiscovery();
	
	TInt SearchRemoteDeviceL();
    TInt RegisterSdpRecord();
    TInt DeleteSdpRecord();
    TInt RemoteSdpQueryL( TBTDevAddr& aAddr );
    TInt ParseSdpAttrValues();

    private:

	/** from MBTEngSdpResultReceiver  */
    void ServiceSearchComplete( const RSdpRecHandleArray& aResult, 
                                         TUint aTotalRecordsCount, TInt aErr );
    void AttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                           const RSdpResultArray& aAttr, 
                                           TInt aErr );
    void ServiceAttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                                  const RSdpResultArray& aAttr, 
                                                  TInt aErr );
    void DeviceSearchComplete( CBTDevice* aDevice, TInt aErr );
    
	/**
	* C++ default constructor.
	*/
	CBTApiDiscovery( MBTTestObserver& aObserver );

    /**
	* By default Symbian 2nd phase constructor is private.
	*/
	void ConstructL();

    private:

    MBTTestObserver& iObserver;
	CBTEngDiscovery* iBTEngDiscovery;
    CActiveSchedulerWait iWaiter;
    TInt iError;
    CBTDevice* iRemoteDevice;
    TSdpServRecordHandle iHandle1;
    TSdpServRecordHandle iHandle2;
    RSdpResultArray iSdpResults;
    };

#endif      // BTAPIDISCOVERY_H

// End of File
