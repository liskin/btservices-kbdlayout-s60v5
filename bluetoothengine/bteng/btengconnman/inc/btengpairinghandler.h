/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Engine API for connection management functionality.
*
*/

#ifndef BTENGPAIRINGHANDLER_H
#define BTENGPAIRINGHANDLER_H

#include <btnotifclient.h>
#include <btmanclient.h>

#include "btengactive.h"

class CBTEngConnMan;
class MBTEngConnObserver;


/**
 *  ?one_line_short_description
 *
 *  ?more_complete_description
 *
 *  @lib ?library
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngPairingHandler ) : public CBase, public MBTEngActiveObserver
    {

public:

    /**
     * Two-phase constructor
     */
    static CBTEngPairingHandler* NewL( MBTEngConnObserver* aObserver, 
                                        CBTEngConnMan* aParent );

    /**
     * Destructor
     */
    virtual ~CBTEngPairingHandler();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    void StartPairingL( const TBTDevAddr& aAddr, TBTDeviceClass& aDeviceClass );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    void CancelPairing();   

// from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, 
                                    TInt aStatus );

    /**
     * Callback for handling cancelation of an outstanding request.
     *
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId );
    
    /**
     * From MBTEngActiveObserver.
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void HandleError( CBTEngActive* aActive, 
                                    TInt aError );

private:

    /**
     * C++ default constructor
     */
    CBTEngPairingHandler( MBTEngConnObserver* aObserver, 
                           CBTEngConnMan* aParent );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

private: // data

    /**
     * the address of the remote device to be paired.
     */
    TBTDevAddrPckgBuf iAddr;
    
    /**
     * the class of device value of the remote device to be paired.
     */
    TBTDeviceClass iCod; 

    /**
     * the session to notifier which handles the actual pairing operation.
     * Own.
     */
    RBTNotifier iBtNotifier;

    /**
     * Active object for pairing request
     * Own.
     */
    CBTEngActive* iActive;

    /**
     * the observer that receives the result of pairing operation.
     * Not own.
     */
    MBTEngConnObserver* iObserver;

    /**
     * ?description_of_pointer_member
     * Not own.
     */
    CBTEngConnMan* iParent;

    };


#endif // BTENGPAIRINGHANDLER_H
