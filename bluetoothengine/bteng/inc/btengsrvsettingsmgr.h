/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Class to manage Bluetooth hardware and stack settings.
*
*/

#ifndef BTENGSRVSETTINGSMGR_H
#define BTENGSRVSETTINGSMGR_H

#include <bluetooth/btpowercontrol.h>
#include <btfeaturescfg.h>
#ifndef __WINS__
#include <bluetooth/dutmode.h>
#endif  //__WINS__  

#include "btengprivatecrkeys.h"
#include "btengconstants.h"
#include "btengactive.h"

class CBTEngServer;


/**
 *  ?one_line_short_description
 *  ?more_complete_description
 *
 *  @code
 *   ?good_class_usage_example(s)
 *  @endcode
 *
 *  @lib ?library
 *  @since S60 ?S60_version *** for example, S60 v3.0
 */
NONSHARABLE_CLASS( CBTEngSrvSettingsMgr ) : public CBase,
                                            public MBTEngActiveObserver
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Pointer to server instance.
     */
    static CBTEngSrvSettingsMgr* NewL( CBTEngServer* aServer );

    /**
     * Destructor.
     */
    virtual ~CBTEngSrvSettingsMgr();

    /**
     * Utility to get the Bluetooth hardware power state.
     *
     * @since Symbian^3
     * @param aState On return, this will contain the current HW power state.
     * @return KErrNone if successful, otherwise one of the system-wide error codes.
     */
    TInt GetHwPowerState( TBTPowerState& aState );

    /**
     * Set the Bluetooth hardware power state.
     *
     * @since Symbian^3
     * @param aState The new power state.
     */
    TInt SetHwPowerState( TBTPowerState aState );

    /**
     * Set Bluetooth on or off. This function is for internal classes of bteng server
     * for power management.
     *
     * @since Symbian^3
     * @param aState The new power state.
     * @param aTemporary Turn BT off after use (ETrue) or not (EFalse).
     */
    void SetPowerStateL( TBTPowerState aState, TBool aTemporary );
    
    /**
     * Set Bluetooth on or off upon a client power management request.
     * This function is only for power management request from clients of bteng server.
     *
     * @since Symbian^3
     * @param aState The new power state.
     * @param aTemporary Turn BT off after use (ETrue) or not (EFalse).
     */
    void SetPowerStateL( const RMessage2 aMessage );
    
    /**
     * Initialize Bluetooth stack settings.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void InitBTStackL();

    /**
     * Reset settings and disconnect all links.
     *
     * @since Symbian^3
     */
    void StopBTStackL();

    /**
     * Update the central repository key for Bluetooth power state.
     *
     * @since Symbian^3
     * @param aValue The new Bluetooth power state.
     */
    void UpdateCenRepPowerKeyL( TBTPowerState aValue );

    /**
    * ?description
    *
    * @since S60 v3.2
    * @param ?arg1 ?description
    */
    void SetUiIndicatorsL();
    
    /**
    * ?description
    *
    * @since S60 v3.2
    * @param ?arg1 ?description
    */
    void SetIndicatorStateL( const TInt aIndicator, const TInt aState );

    /**
     * Update the Bluetooth visibility mode.
     *
     * @since Symbian^3
     * @param ?arg1 ?description
     */
    void SetVisibilityModeL( TBTVisibilityMode aMode, TInt aTime );

    /**
     * Update the Bluetooth visibility mode.
     *
     * @since Symbian^3
     * @param ?arg1 ?description
     */
    void UpdateVisibilityModeL( TInt aStackScanMode );

    /**
     * Set Device Under Test mode.
     *
     * @since Symbian^3
     * @param aDutMode The mode to be set (DUT mode on or off).
     */
    void SetDutMode( TInt aDutMode );

    /**
     * Timed visible mode has expired. 
     *
     * @since Symbian^3
     */
    void ScanModeTimerCompletedL();

    /**
     * Check whether BT should be turned off automatically.
     *
     * @since Symbian^3
     */
    void CheckAutoPowerOffL();

    /**
     * Queue a timer if secure simple pairing debug mode has been enabled.
     *
     * @since Symbian^3
     * @param aDebugMode State of Simple Pairing debug mode.
     */
    void CheckSspDebugModeL( TBool aDebugMode );

    /**
     * Be informed that a session will be closed.
     *
     * @since Symbian^3
     * @param aSession the session to be cloased.
     */
    void SessionClosed(CSession2* aSession );
    
private:
    
// from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an outstanding request has completed.
     *
     * @since Symbian^3
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, TInt aId, TInt aStatus );

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an error has occurred in RunL.
     *
     * @since Symbian^3
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aId, TInt aError );

private:

    /**
     * C++ default constructor.
     */
    CBTEngSrvSettingsMgr( CBTEngServer* aServer );

    /**
     * Symbian second-phase constructor.
     */
    void ConstructL();

    /**
     * Open a handle to the Bluetooth power manager, and 
     * initialize the power to off.
     *
     * @since Symbian^3
     */
    void LoadBTPowerManagerL();

    /**
     * Checks if a client requests temporary power on/off, and keeps track 
     * of the number of clients requesting that.
     *
     * @since Symbian^3
     * @param aCurrentState On return, will contain the current power state.
     * @param aNewState The requested power state.
     * @param aTemporary Indicates if this is about a tempororary state change.
     */
    void CheckTemporaryPowerStateL( TBTPowerState& aCurrentState,
                                     TBTPowerState aNewState, TBool aTemporary );

private: // data

#ifdef __WINS__
    /**
     * Current BT power state (power manager is not used in emulator).
     */
    TBTPowerState iPowerState;
#endif  //__WINS__

    /**
     * Flag indicating if BT is allowed to be turned on.
     */
    BluetoothFeatures::TEnterpriseEnablementMode iEnterpriseEnablementMode;

    /**
     * Flag indicating if BT is going to be switched off automatically.
     */
    TBool iAutoSwitchOff;

    /**
     * Number of clients that are using BT temporarily.
     * Note that this is equal or less than the active number of links.
     */
    TInt iAutoOffClients;

    /**
     * Flag indicating hidden mode has been set for temporary power on.
     */
    TBool iRestoreVisibility;

    /**
     * BT power control.
     */
    RBTPowerControl iPowerMgr;

    /**
     * Active object helper.
     * Own.
     */
    CBTEngActive* iActive;

    /**
     * The server instance.
     * Not own.
     */
    CBTEngServer* iServer;
    
    /**
     * Client-server message for power change requests.
     */
    RMessage2 iMessage;
    
    };


#endif // BTENGSRVSETTINGS_H
