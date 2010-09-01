/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Helper class declaration for performing device discovery.
*
*/



#ifndef BTENGDEVICESEARCH_H
#define BTENGDEVICESEARCH_H


#include <btnotif.h>
#include <bt_sock.h>
#include "btengactive.h"

class MBTEngSdpResultReceiver;

/**
 *  Class CBTEngDeviceSearch
 *
 *  ?more_complete_description
 *
 *  @lib ?library
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngDeviceSearch ) : public CBase, 
                                          public MBTEngActiveObserver
    {

public:

    /**
     * Two-phase constructor
     */
    static CBTEngDeviceSearch* NewL();

    /**
     * Destructor
     */
    virtual ~CBTEngDeviceSearch();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    TInt StartSearch( CBTDevice* aDevice, 
            TNameEntry* aNameEntry,
            const TBTDeviceClass& aDeviceClass, 
            MBTEngSdpResultReceiver* aNotifier );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    void CancelSearch();

    TInt GetEirServiceUUIDs( const TBTDevAddr& aAddr, 
            TNameEntry* aNameEntry,
                MBTEngSdpResultReceiver* aNotifier);    
    
    void CancelGetEirServiceUUIDs();
    
// from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an outstanding request has completed.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, TInt aId, 
                                     TInt aStatus );

    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aId, TInt aError );

private:

    /**
     * C++ default constructor
     */
    CBTEngDeviceSearch();

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();
    
    TInt DoGetDeviceEir(const TBTDevAddr& aAddr );
    
    void HandleDeviceSelectionResultL(TInt aErr);
    
    void HandleDeviceEirDataResult(TInt aErr);
    
    /**
     * Notifies the caller of the search result.
     */
    void NotifyClient(TInt aErr);

private:
    enum TClientRequest
        {
        EDeviceSearch,
        EGetDeviceEir
        };
    
private: // data

    /**
     * Hanlde to the search notifier.
     */
    RNotifier iNotifier;

    /**
     * Selection parameters package buffer.
     */
    TBTDeviceSelectionParamsPckg iSelectionPckg;
    
    /**
     * Response parameters package buffer.
     */
    TBTDeviceResponseParamsPckg iResponsePckg;
    
    /**
     * Our actual active object.
     * Own.
     */
    CBTEngActive* iActive;
    
    /**
     * Socket server for Host Resolver.
     * Own.
     */
    RSocketServ         iSocketServer; 
    
    /**
     * Hose Resolver to get cached EIR.
     * Own.
     */    
    RHostResolver       iHostResolver;
    
    /**
     * used for getting cached EIR.
     */
    TInquirySockAddr    iInquirySockAddr;
    
    /**
     * Client's object to store the results.
     * Not own.
     */
    CBTDevice* iDevice;
    
    /**
     * Client's object to store EIR data.
     * Not own.
     */
    TNameEntry* iNameEntry;

    /**
     * Reference to receiver of results.
     * Not own.
     */
    MBTEngSdpResultReceiver* iResultNotifier;

    TClientRequest iClientReq;
    
    };


#endif // BTENGDEVICESEARCH
