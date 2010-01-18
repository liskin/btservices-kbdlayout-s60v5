/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  the base class of pairing handling
*
*/


#ifndef BTENGPAIRBASE_H_
#define BTENGPAIRBASE_H_

#include <e32base.h>
#include <bttypes.h>
#include <btnotif.h>
#include "btengactive.h"

class CBTEngPairMan;

/**  Identification for asynchronous operations */
enum TPairBaseActiveRequestId
    {
    EDevicePairUserNotification = 1
    };

/**
 *  Class CBTEngPairBase
 *
 *  The base class for pairing handling. 
 *  When the pairing has completed (successfully or unsuccessfully), the user
 *  is informed of the result
 *
 *  @since S60 S60 v5.1
 */
NONSHARABLE_CLASS( CBTEngPairBase ) : public CBase, public MBTEngActiveObserver
    {
public:
    
    /**
     * Destructor
     */
    virtual ~CBTEngPairBase();
    
    /**
     * Handle a pairing result from the pairing server.
     *
     * @param aAddr the address of the remote device which the result is for.
     * @param aResult The status code of the pairing or authentication result.
     */
    void HandlePairServerResult( const TBTDevAddr& aAddr, TInt aResult );
    
    /**
     * Handles event of new paired device event in registry.
     * @param aDev the remote device which the pair is with.
     */
    void HandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev );
    
    /**
     * Start observing the result of the pairing originated by
     * the remote device.
     * Must be specialized by subclass.
     * @param the address of the remote device to be paired
     * @return KErrNone if this request is accepted; otherwise an error code
     */
    virtual TInt ObserveIncomingPair( const TBTDevAddr& aAddr ) = 0;

    /**
     * Start an outgoing pairing with the remote device.
     * Must be specialized by subclass.
     * @param the address of the remote device to be paired
     * @return KErrNone if this request is accepted; otherwise an error code
     */
    virtual void HandleOutgoingPairL( const TBTDevAddr& aAddr, TUint aCod ) = 0;
    
    /**
     * Cancel the outstanding pairing operation.
     */
    virtual void CancelOutgoingPair();

    /**
     * Cancels pairing handling with the specified device
     * @param aAddr the address of the device the pairing is with
     */
    virtual void StopPairHandling( const TBTDevAddr& aAddr ) = 0;
    
protected:
    
    /**
     * Handle a pairing result with the remote device which this is for.
     * Must be specialized by subclass.
     *
     * @param aResult The status code of the pairing or authentication result.
     */
    virtual void DoHandlePairServerResult( TInt aResult ) = 0;
    
    /**
     * Handles event of registry new paired event with the remote 
     * device this is for.
     * Must be specialized by subclass.
     * @aType the type of authentication with the device.
     */
    virtual void DoHandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev ) = 0;
    
protected:
    
    /**
     * C++ default constructor
     */
    CBTEngPairBase(CBTEngPairMan& aParent, const TBTDevAddr& aAddr);
    
    /**
     * Symbian 2nd-phase constructor
     */
    void BaseConstructL();

    /**
     * Cancels notifying user about the pairing result.
     */
    void CancelNotifier();
    
    /**
     * Show the pairing result and ask the user to authorize the device if
     * pair succeeded.
     *
     * @since S60 v5.1
     */
    void ShowPairingNoteAndAuthorizeQuery();    
    
    /**
     * Invalidate the pair result flag of this 
     */
    void UnSetPairResult();
    
    /**
     * Sets the pair result and validate pair result flag
     */
    void SetPairResult( TInt aResult );
    
    /**
     * Tells if the pair result has been set.
     * @ETrue if the result has been set.
     */
    TBool IsPairResultSet();
    
    /**
     * Tells if this is notifying user the pairing result.
     * @return ETrue if this is is notifying user the pairing result
     */
    TBool IsNotifyingPairResult();
    
protected:

    /**
     * Address of the remote device we are trying to pair.
     */
    TBTDevAddr iAddr;
    
    /**
     * Contains the final result of pairing with the remote device
     */
    TInt iPairResult;
    
    /**
     * Pair result flag, ETrue if iPairResult is been set.
     */
    TBool iPairResultSet;
    
    /**
     * Reference to the owner of this object.
     */
    CBTEngPairMan& iParent;

    /**
     * Package buffer for passing parameters to authorization notifier.
     */
    TBTPairedDeviceSettingParamsPckg iAuthoPckg;

    /**
     * Session with the notifier server.
     * Own.
     */
    RNotifier iNotifier;
    
    /**
     * Active object helper for asynchronous operations.
     * Own.
     */
    CBTEngActive* iActive;
    };

#endif /*BTENGPAIRBASE_H_*/
