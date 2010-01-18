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
* Description:  Active object helper class.
*
*/



#ifndef BTUIACTIVE_H
#define BTUIACTIVE_H


#include <e32base.h>

class CBTUIActive;

/**
 *  Class MBTUIActiveObserver
 *
 *  Callback class for receiving a completed active object event.
 *  Users of CBTUIActive need to derive from this class. 
 * 
 *  @since S60 v5.0
 */
class MBTUIActiveObserver
    {

public:

    /**
     * Callback to notify that an outstanding request has completed.
     *
     * @since S60 v5.0
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBTUIActive* aActive, TInt aId, 
                                    TInt aStatus ) = 0;

    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBTUIActive* aActive, TInt aId, 
                                    TInt aError ) = 0;

    };


/**
 *  Class CBTUIActive
 *
 *  Base clase for active objects used in BTUI
 *
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CBTUIActive ) : public CActive
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 v5.0
     * @param aObserver Pointer to callback interface that receives notification
     *                  that the request has been completed.
     * @param aId The request ID
     * @param aPriority Active Object Priority 
     * @return Pointer to the constructed CBTUIActive object.
     */
    static CBTUIActive* NewL( MBTUIActiveObserver* aObserver, 
                                TInt aId, TInt aPriority );

    /**
     * Destructor
     */
    virtual ~CBTUIActive();

    /**
     * Get the request ID of this active object.
     *
     * @since S60 v5.0
     * @return The request ID of this active object.
     */
    TInt RequestId();
    
    /**
     * Set a new request ID for this active object.
     *
     * @since S60 v5.0
     * @param The new request ID of this active object.
     */
    void SetRequestId( TInt aId );
    
    /**
     * Activate the active object.
     *
     * @since S60 v5.0
     */
    void GoActive();

    /**
     * Cancel an outstanding request.
     *
     * @since S60 v5.0
     */
    void CancelRequest();

    /**
     * Get a reference to the active object request status.
     *
     * @since S60 v5.0
     * @return Reference to the active object request status.
     */
    TRequestStatus& RequestStatus();

// from base class CActive

    /**
     * From CActive.
     * Called by the active scheduler when the request has been cancelled.
     *
     * @since S60 v5.0
     */
    void DoCancel();

    /**
     * From CActive.
     * Called by the active scheduler when the request has been completed.
     *
     * @since S60 v5.0
     */
    void RunL();

    /**
     * From CActive.
     * Called by the active scheduler when an error in RunL has occurred.
     * Error handling is really done by callback in HandleError().
     *
     * @since S60 v5.0
     * @param aError Error occured in Active Object's RunL().
     * @return KErrNone. 
     */
    TInt RunError( TInt aError );

private:

    /**
     * C++ default constructor
     *
     * @since S60 v5.0
     * @param aObserver Pointer to callback interface that receives notification
     *                  that the request has been completed.
     * @param aId ID of the request (for the client to keep track of multiple 
     *            active objects).
     * @param aPriority Priority of 
     */
    CBTUIActive( MBTUIActiveObserver* aObserver, TInt aId, TInt aPriority );

    /**
     * Symbian 2nd-phase constructor
     *
     * @since S60 v5.0
     */
    void ConstructL();

private: // data

    /**
     * ID of the request (used only by our client).
     */
    TInt iRequestId;

    /**
     * Our observer.
     * Not own.
     */
    MBTUIActiveObserver* iObserver;

    };


#endif // BTUIACTIVE_H
