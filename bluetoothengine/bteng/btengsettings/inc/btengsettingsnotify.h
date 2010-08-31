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
* Description:  Helper class for handling change notifications of 
*                PubSub properties and CenRep keys.
*
*/


#ifndef BTENGSETTINGSNOTIFY_H
#define BTENGSETTINGSNOTIFY_H


#include "btengactive.h"
#include "btengclient.h"

class MBTEngSettingsObserver;
class CRepository;

/**
 *  Class CBTEngSettingsNotify
 *
 *  ?more_complete_description
 *
 *  @lib btengsettings.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngSettingsNotify ) : public CBase, 
                                            public MBTEngActiveObserver
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 v3.2
     * @param aObserver Pointer to callback interface that receives notification
     *                  that a setting has changed.
     * @return Pointer to the constructed CBTEngSettingsNotify object.
     */
    static CBTEngSettingsNotify* NewL( MBTEngSettingsObserver* aObserver );

    /**
     * Destructor
     */
    virtual ~CBTEngSettingsNotify();

    /**
     * Toggles the Bluetooth power state (on or off). Power will be switched on 
     * for the duriation of the session. It will also be turned off (gracefully, 
     * if no permanent connection exists) if this object is destroyed.
     * Note: power will only be switched off gracefully if it has also been 
     * switched on through this method, otherwise KErrAccessDenied will be 
     * returned.
     *
     * @since S60 v5.0
     * @return KErrNone on success, otherwise a system wide error code.
     */
    TInt TogglePowerTemporarily();

// from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Handle a setting change.
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
     * Handle an error in the setting change handling.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aError );

private:

    /**
     * C++ default constructor
     *
     * @since S60 v3.2
     * @param aObserver Pointer to callback interface that receives notification
     *                  that a setting has changed.
     */
    CBTEngSettingsNotify( MBTEngSettingsObserver* aObserver );

    /**
     * Symbian 2nd-phase constructor
     *
     * @since S60 v3.2
     */
    void ConstructL();

private: // data

    /**
     * Active object for watching power mode setting changes.
     * Own.
     */
    CBTEngActive* iPowerKeyWatcher;

    /**
     * Active object for watching visibility mode setting changes.
     * Own.
     */
    CBTEngActive* iVisiKeyWatcher;

    /**
     * Session with the central repository for power mode setting.
     * Own.
     */
    CRepository* iPowerKeyCenRep;

    /**
     * Session with the central repository for visibility mode setting.
     * Own.
     */
    CRepository* iVisiKeyCenRep;

    /**
     * Client to notify of changes.
     * Not own.
     */
    MBTEngSettingsObserver* iObserver;

    /**
     * Handle to BTEng server for turning Bluetooth on temporarily.
     */
    RBTEng iBTeng;
    };


#endif // BTENGSETTINGSNOTIFY_H
