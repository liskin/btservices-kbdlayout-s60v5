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
* Description: Class for tracking Bluetooth settings, and also for 
* handling notes unrelated to specific connection.
*
*/

#ifndef BTNOTIFSETTINGSTRACKER_H
#define BTNOTIFSETTINGSTRACKER_H


#include <e32base.h>
#include <btengsettings.h>

#include "bluetoothnotification.h"

class CBTNotifServer;

/**
 * CBTNotifSettingsTracker keeps track of local device settings
 * 
 * @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifSettingsTracker ) : public CBase,
                                               public MBTEngSettingsObserver,
                                               public MBTNotificationResult
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Pointer to our parent
     */
    static CBTNotifSettingsTracker* NewL( CBTNotifServer* aServer );

    /**
    * Destructor.
    */
    virtual ~CBTNotifSettingsTracker();

    /**
     * Return the current power state
     *
     * @since Symbian^4
     * @return The current power state.
     */
    inline TBTPowerStateValue GetPowerState() const
        { return iPowerState; }

    /**
     * Processes a message from a notifier client related to settings.
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the client request.
     */
    void DispatchNotifierMessageL( const RMessage2& aMessage );

    /**
     * Cancels an oustanding message from a notifier client related to settings.
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the original client request.
     */
    void CancelNotifierMessageL( const RMessage2& aMessage );

// from base class MBTEngSettingsObserver

    /**
     * From MBTEngSettingsObserver.
     * Provides notification of changes in the power state 
     * of the Bluetooth hardware.
     *
     * @since Symbian^4
     * @param aState EBTPowerOff if the BT hardware has been turned off, 
     *               EBTPowerOn if it has been turned off.
     */
    virtual void PowerStateChanged( TBTPowerStateValue aState );

    /**
     * From MBTEngSettingsObserver.
     * Provides notification of changes in the discoverability 
     * mode of the Bluetooth hardware.
     *
     * @since Symbian^4
     * @param aState EBTDiscModeHidden if the BT hardware is in hidden mode, 
     *               EBTDiscModeGeneral if it is in visible mode.
     */
    virtual void VisibilityModeChanged( TBTVisibilityMode aState );

// from base class MBTNotificationResult

    /**
     * From MBTNotificationResult.
     * Handle an intermediate result from a user query.
     * This ffunction is called if the user query passes information
     * back before it has finished i.e. is dismissed. The final acceptance/
     * denial of a query is passed back in MBRNotificationClosed.
     *
     * @since Symbian^4
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRDataReceived( CHbSymbianVariantMap& aData );

    /**
     * From MBTNotificationResult.
     * The notification is finished. The resulting data (e.g. user input or
     * acceptance/denial of the query) is passed back here.
     *
     * @since Symbian^4
     * @param aErr KErrNone or one of the system-wide error codes.
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRNotificationClosed( TInt aError, const TDesC8& aData );

private:

    CBTNotifSettingsTracker( CBTNotifServer* aServer );

    void ConstructL();

private: // data

    /**
     * Local copy of current power state.
     */
    TBTPowerStateValue iPowerState;

    /**
     * Local copy of current visibility mode.
     */
    TBTVisibilityMode iVisibilityMode;

    /**
     * ?description_of_pointer_member
     * Own.
     */
    CBTEngSettings* iSettings;

    /**
     * Pointer to an outstanding user interaction.
     * Not own.
     */
    CBluetoothNotification* iNotification;

    /**
     * Reference to our parent the server class.
     * Not own.
     */
    CBTNotifServer* iServer;

    };

#endif // BTNOTIFSETTINGSTRACKER_H
