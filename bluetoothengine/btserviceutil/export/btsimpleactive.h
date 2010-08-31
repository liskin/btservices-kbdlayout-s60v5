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
* Description:  Active object helper class.
*
*/

#ifndef BTSIMPLEACTIVE_H
#define BTSIMPLEACTIVE_H

#include <e32base.h>

class CBtSimpleActive;

/**
 * APIs from this class offer functionalities that are common in mw and app 
 * components of Bluetooth packages. They do not serve as domain APIs.
 * 
 * Using these from external components is risky, due to possible source
 * and binary breaks in future.
 * 
 */


/**
 *  Class MBtSimpleActiveObserver
 *
 *  Callback class for receiving a completed active object event.
 *  Users of CBtSimpleActive need to derive from this class. 
 *
 *  @lib bteng*.lib
 *  @since S60 v3.2
 */
class MBtSimpleActiveObserver
    {
public:

    /**
     * Callback from RunL() to notify that an outstanding request has completed.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus ) = 0;

    /**
     * Callback from Docancel() for handling cancelation of an outstanding request.
     *
     * @since Symbian^4
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId ) = 0;

    /**
     * Callback from RunError() to notify that an error has occurred in RunL.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aError The error occurred in RunL.
     */
    virtual void HandleError( CBtSimpleActive* aActive, TInt aError ) = 0;
    };

/**
 *  Class CBtSimpleActive.
 *  
 *  This Active Object provides its client a TRequestStatus for performing 
 *  asynchronous requests. It does not know the detail of the asynch operation.
 *  All of AO callbacks, such as RunL, will be delegated to the client
 *  for processing, via interface MBtSimpleActiveObserver.
 *  
 */
NONSHARABLE_CLASS ( CBtSimpleActive ): public CActive
    {

public:

    /**
     * Two-phase constructor
     *
     * @since Symbian^4
     * @param aObserver Pointer to callback interface that receives notification
     *                  that the request has been completed.
     * @param aId Identifier for the CBtSimpleActive instance.
     * @param aPriority The priority of the active object.
     * @return Pointer to the constructed CBtSimpleActive object.
     */
    IMPORT_C static CBtSimpleActive* NewL( MBtSimpleActiveObserver& aObserver, TInt aId,
                                TInt aPriority = CActive::EPriorityStandard );

    /**
     * Destructor
     */
    IMPORT_C virtual ~CBtSimpleActive();

    /**
     * Get the request ID of this active object.
     *
     * @since Symbian^4
     * @return The request ID of this active object.
     */
    IMPORT_C TInt RequestId();

    /**
     * Set a new request ID for this active object.
     *
     * @since Symbian^4
     * @param The new request ID of this active object.
     */
    IMPORT_C void SetRequestId( TInt aId );

    /**
     * Activate the active object.
     *
     * @since Symbian^4
     */
    IMPORT_C void GoActive();

    /**
     * Get a reference to the active object request status.
     *
     * @since Symbian^4
     * @return Reference to the active object request status.
     */
    IMPORT_C TRequestStatus& RequestStatus();

private:
// from base class CActive

    /**
     * From CActive.
     * Called by the active scheduler when the request has been cancelled.
     *
     * @since S60 v3.2
     */
    void DoCancel();

    /**
     * From CActive.
     * Called by the active scheduler when the request has been completed.
     *
     * @since S60 v3.2
     */
    void RunL();

    /**
     * From CActive.
     * Called by the active scheduler when an error in RunL has occurred.
     *
     * @since S60 v3.2
     */
    TInt RunError( TInt aError );

private:

    /**
     * C++ default constructor
     *
     * @since S60 v3.2
     * @param aObserver Pointer to callback interface that receives notification
     *                  that the request has been completed.
     * @param aId ID of the request (for the client to keep track of multiple 
     *            active objects).
     * @param aPriority Priority of 
     */
    CBtSimpleActive( MBtSimpleActiveObserver& aObserver, TInt aId, TInt aPriority );

private: // data

    /**
     * ID of the request (used only by our client).
     */
    TInt iRequestId;

    /**
     * Our observer.
     * Not own.
     */
    MBtSimpleActiveObserver& iObserver;

    };

#endif // BTSIMPLEACTIVE_H
