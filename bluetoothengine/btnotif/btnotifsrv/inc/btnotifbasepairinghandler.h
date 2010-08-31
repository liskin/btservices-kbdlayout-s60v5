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
* Description: 
*
*/

#ifndef BTNOTIFBASEPAIRINGHANDLER_H
#define BTNOTIFBASEPAIRINGHANDLER_H

#include <e32base.h>
#include <bttypes.h>
#include <btnotif.h>
#include <btengconstants.h>
#include <btservices/btsimpleactive.h>

class CBTNotifSecurityManager;

/**
 *  Class CBTNotifBasePairingHandler
 *
 *  The base class for pairing handling. 
 *  When the pairing has completed (successfully or unsuccessfully), the user
 *  is informed of the result
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifBasePairingHandler ) : 
    public CBase, public MBtSimpleActiveObserver
    {
public:
    
    /**
     * Destructor
     */
    virtual ~CBTNotifBasePairingHandler();
    
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
    
    /**
     * Gets the pin code to be used for pairing a device.
     * @param aPin contains the pin code if it is not empty
     * @param aAddr the device to which pairing is performed.
     * @param the required minimum length of a pin code.
     */
    virtual void GetPinCode( TBTPinCode& aPin, const TBTDevAddr& aAddr, TInt aMinPinLength );
 
    /**
     * Show a dialog for pairing success or failure
     * @param aResult The result of the pairing
     */
    void ShowPairingResultNoteL(TInt aResult);
    
protected:
    
    /**
     * Handle a pairing result with the remote device which this is for.
     * Must be specialized by subclass.
     *
     * @param aResult The status code of the pairing or authentication result.
     */
    virtual void DoHandlePairServerResult( TInt aResult ) = 0;
    
    /**
     * Handles a registry new paired event with the remote 
     * device with which this handler is dealing.
     * Must be specialized by subclass.
     * @aType the type of authentication with the device.
     */
    virtual void DoHandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev ) = 0;
    
protected:
    
    /**
     * C++ default constructor
     */
    CBTNotifBasePairingHandler(CBTNotifSecurityManager& aParent, const TBTDevAddr& aAddr);
    
    /**
     * Symbian 2nd-phase constructor
     */
    void BaseConstructL();
    
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
    CBTNotifSecurityManager& iParent;
    
    /**
     * Active object helper for asynchronous operations.
     * Own.
     */
    CBtSimpleActive* iActive;
    };

#endif /*BTNOTIFBASEPAIRINGHANDLER_H*/



