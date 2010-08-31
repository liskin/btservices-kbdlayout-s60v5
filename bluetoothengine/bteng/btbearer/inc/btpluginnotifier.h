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
* Description:  This is the BT bearer plugin  notifier header file.
*
*/


#ifndef __BTPLUGINNOTIFIER_H__
#define __BTPLUGINNOTIFIER_H__


#include <e32base.h>
#include <btserversdkcrkeys.h>
#include <locodbearerpluginparams.h>      
#include <centralrepository.h>      
#include <locodbearer.h>
#include <hbindicatorsymbian.h>


/**
 *  BT Plug in module for local connectivity daemon.
 *
 *  This active object checks BT state from central repository and start 
 *  watching BT ON/OFF state. If BT is ON, it connects to BTEngine server.
 *  It  continues to watch the key and when it has changed, it notifies 
 *  the daemon through its callback function.
 *
 *  @lib btbearer.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTPluginNotifier ) : public CActive
    {

public:

    /**  Enumeration of CenRep key types */
    enum TCenRepKeyType
        {
        EKeyInt,
        EKeyReal,
        EKeyBinary,
        EKeyString
        };
        
    /**
     * Two-phase constructor
     *
     * @param aObserver Reference to the callback class.
     * @param aUid The UID that identifies the repository.
     * @param aKeyType Key type.
     * @param aId The ID of the key.
     * @return CBTPluginNotifier Initialized object.
     */
	static CBTPluginNotifier* NewL( MLocodBearerPluginObserver& aObserver, 
	                                TUid aUid, TCenRepKeyType aKeyType, 
	                                TUint32 aId );

    /**
     * Destructor
     */
	virtual ~CBTPluginNotifier();

private:

    /**
     * C++ default constructor
     */
    CBTPluginNotifier( MLocodBearerPluginObserver& Param, TUid aUid, 
	                   TCenRepKeyType aKeyType, TUint32 aId );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

    /**
     * Subscribes to the change of central repository keys
     *
     * @since S60 v3.2
     */
    void SubscribeL();
    
    /**
     * Handles service load/unload when BT power changes.
     */
    void HandleBtPowerChanged( TBTPowerStateValue aPower );

// from base class CActive

    /**
     * From CActive.
     * Called by the active scheduler when our subscription 
     * to the setting has been cancelled.
     *
     * @since S60 v3.2
     */
    void DoCancel();

    /**
     * From CActive.
     * Called by the active scheduler when the status has changed.
     *
     * @since S60 v3.2
     */
    void RunL();
    
    /**
     * From CActive
     * Called by the active scheduler when RunL() leaves, aError contains the leave code.
     * 
     * @since S60 5.0
     */
    int RunError(TInt aError);
    
private: // data

    /**
     * The UID that identifies the setting category.
     */
    TUid iUid;

    /**
     * ID for BT power state in the central repository
     */
    TUint32 iId;

    /**
     * Central repository key type
     */
    TCenRepKeyType iKeyType;

    /**
     * Reference to the observer.
     */
    MLocodBearerPluginObserver& iHandler;

    /**
     * Session with the central repository.
     * Own.
     */
    CRepository* iSession;
    
    CHbIndicatorSymbian* iBTIndicator;

    };


#endif  // __BTPLUGINNOTIFIER_H__
