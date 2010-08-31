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
* Description: Class for managing an actual user notification or query.
*              It hides UI framework-specifics in a private class.
*
*/

#ifndef BLUETOOTHNOTIFICATION_H
#define BLUETOOTHNOTIFICATION_H

#include <e32base.h>
#include <hb/hbcore/hbdevicedialogsymbian.h>
#include "bluetoothdevicedialogs.h"
#include "btnotificationresult.h"
#include "bluetoothtrace.h"

class CBTNotificationManager;
class CHbSymbianVariantMap;

/**
 *  CBluetoothNotification manages and controls notifications.
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBluetoothNotification ) : public CBase,
                                              public MHbDeviceDialogObserver
    {


public:

    /**
     * Two-phased constructor.
     * @param aManager Reference to the notification manager.
     * @param aObserver Reference to our observer.
     */
    static CBluetoothNotification* NewL( CBTNotificationManager* aManager );

    /**
    * Destructor.
    */
    virtual ~CBluetoothNotification();

    /**
     * Sets the receiver of callbacks.
     *
     * @since Symbian^4
     * @param aObserver Pointer to the callback interface.
     */
    inline void SetObserver( MBTNotificationResult* aObserver )
        { iObserver = aObserver; }

    /**
     * Clears the receiver of callbacks.
     *
     * @since Symbian^4
     */
    inline void RemoveObserver()
        { iObserver = NULL; }

    /**
     * Getter for the notification type.
     *
     * @since Symbian^4
     * @return Notification type.
     */
    inline TBluetoothDialogParams::TBTDialogType NotificationType()
        { return iType; }

    /**
     * Getter for the resource id.
     *
     * @since Symbian^4
     * @return Resource identifier.
     */
    inline TBTDialogResourceId ResourceId()
        { return iResourceId; }

    inline CHbSymbianVariantMap* Data()
         { return iNotificationData; }
    
    /**
     * Sets the type of notification (query, note, etc).
     *
     * @since Symbian^4
     * @param aType The type of the notification.
     * @param aResourceId The id of the resource to be shown.
     * @return Error code
     */
    inline void SetNotificationType( TBluetoothDialogParams::TBTDialogType aType,
                TBTDialogResourceId aResourceId )
            { iType = aType; iResourceId = aResourceId; }

    /**
     * Sets the data to be shown to the user.
     *
     * @since Symbian^4
     * @param aType Identifies the type of data parameter to be set.
     * @param aData Additional descriptor data to be shown in the dialog.
     * @return Error code
     */
    TInt SetData( TInt aDataType, const TDesC& aData );

    /**
     * Sets the data to be shown to the user.
     *
     * @since Symbian^4
     * @param aType Identifies the type of data parameter to be set.
     * @param aData Additional integer data to be shown in the dialog.
     * @return Error code
     */
    TInt SetData( TInt aDataType, TInt aData );

    /**
     * Updates the data to be shown to the user.
     *
     * @since Symbian^4
     * @param ?arg1 ?description
     * @return Error code
     */
    TInt Update( const TDesC& aData =KNullDesC );

    /**
     * Show the notification, which means that it 
     * is added to the queue.
     *
     * @since Symbian^4
     * @param ?arg1 ?description
     * @return Error code
     */
    void ShowL();

    /**
     * Stop showing the notification.
     *
     * @since Symbian^4
     * @param ?arg1 ?description
     * @return Error code
     */
    TInt Close();


private:

    CBluetoothNotification( CBTNotificationManager* aManager );

    void ConstructL();
    
    /**
     * Sets the data to be shown to the user, leaves on error.
     *
     * @since Symbian^4
     * @param aType Identifies the type of data parameter to be set.
     * @param aData Additional descriptor data to be shown in the dialog.
     */
    void SetDataL( TInt aType, const TDesC& aData );

    /**
     * Sets the data to be shown to the user, leaves on error.
     *
     * @since Symbian^4
     * @param aType Identifies the type of data parameter to be set.
     * @param aData Additional integer data to be shown in the dialog.
     */
    void SetDataL( TInt aType, TInt aData );
    
    /**
     * From MHbDeviceDialogObserver.
     * This callback is called when data is received from a device dialog.
     *
     * @since Symbian^4
     * @param aData contains data from the dialog plugin.
     */
    virtual void DataReceived( CHbSymbianVariantMap& aData );

    /**
     * From MHbDeviceDialogObserver.
     * This callback is called when a device dialog is closed. Any data sent by
     * the dialog is indicated by the dataReceived() callback. If no observer is
     * set in CHbDeviceDialogSymbian::Show the latest data can be retrieved with
     * CHbDeviceDialogSymbian::receivedData().
     *
     * @since Symbian^4
     * @param aCompletionCode gives the result of the dialog completion. Code can be
     *                        either Symbian error code or device dialog error code.
     */
    virtual void DeviceDialogClosed( TInt aCompletionCode );

#ifdef BLUETOOTHTRACE_ENABLED
    void debugHbSymbianVariantMap( CHbSymbianVariantMap& aData);
#endif // BLUETOOTHTRACE_ENABLED

private: // data

    /**
     * The type of notification currently showing.
     */
    TBluetoothDialogParams::TBTDialogType iType;

    /**
     * The id of the resource (string identifier)
     * of the currently showing notification.
     */
    TBTDialogResourceId iResourceId;

    /**
     * Reference to the queue manager.
     * Not own.
     */
    CBTNotificationManager* iManager;

    /**
     * Reference to the receiver of the results.
     * Not own.
     */
    MBTNotificationResult* iObserver;
    
    /**
     * The data to be shown to the user in a device dialog.
     * Own.
     */
    CHbSymbianVariantMap* iNotificationData;

    /**
     * Buffer for receiving return data from the notifier.
     */
    CHbSymbianVariantMap* iReturnData;

    /**
     * Session object with the notification server.
     * Own.
     */
    CHbDeviceDialogSymbian *iDialog;
    
    BTUNITTESTHOOK

    };

#endif // BLUETOOTHNOTIFICATION_H
