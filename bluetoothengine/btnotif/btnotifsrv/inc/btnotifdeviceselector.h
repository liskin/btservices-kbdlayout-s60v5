/*
* ============================================================================
*  Name        : btnotifdeviceselector.h
*  Part of     : BTProximity / BTProximity
*  Description : Class for tracking Bluetooth settings, and also for 
*                handling notes unrelated to specific connection.
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.2
*/

#ifndef BTNOTIFDEVICESELECTOR_H
#define BTNOTIFDEVICESELECTOR_H


#include <e32base.h>
#include <btengsettings.h>
#include <btservices/devdiscoveryobserver.h>
#include "bluetoothnotification.h"
#include <btservices/btdevrepository.h>
#include <hb/hbcore/hbsymbianvariant.h>

class CBTNotifServer;
class CAdvanceDevDiscoverer;
class CBtDevExtension;

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
NONSHARABLE_CLASS( CBTNotifDeviceSelector ) : 
        public CBase,
        public MBTNotificationResult,
        public MDevDiscoveryObserver,
        public MBtDevRepositoryObserver
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Pointer to our parent
     */
    static CBTNotifDeviceSelector* NewL( CBTNotifServer& aServer );

    /**
    * Destructor.
    */
    virtual ~CBTNotifDeviceSelector();

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

private:
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
    
    // From MDevDiscoveryObserver
    
    /**
     * Callback to notify that a device has been found.
     *
     * @param aAddr the inquiry address that contains the inquiry information
     *  of the found device.
     * @param aName the Bluetooth device name of the found device
     */
    virtual void HandleNextDiscoveryResultL( 
            const TInquirySockAddr& aAddr, const TDesC& aName );
    
    /**
     * Callback to notify that the device search has completed.
     *
     * @param aErr the error code of device search result.
     */
    virtual void HandleDiscoveryCompleted( TInt aErr );
    
    
    // From MBtDeviceRepositoryObserver
    
    void RepositoryInitialized();
    
    void DeletedFromRegistry( const TBTDevAddr& addr );
    
    void AddedToRegistry( const CBtDevExtension& dev );
    
    void ChangedInRegistry( const CBtDevExtension& dev, TUint similarity  ); 

    void ServiceConnectionChanged(
            const CBtDevExtension& dev, TBool connected );

    
private:

    CBTNotifDeviceSelector( CBTNotifServer& aServer );

    void ConstructL();
    
    void PrepareNotificationL( 
            TBluetoothDialogParams::TBTDialogType aType,
            TBTDialogResourceId aResourceId );
			
	void LoadUsedDevicesL();	
	
	void AddDataL(CHbSymbianVariantMap* aMap, const TDesC& aKey, const TAny* aData, 
	        CHbSymbianVariant::TType aDataType);
	
	void setMajorProperty(TInt aMajorProperty, TInt prop, TBool addto);
	
	TBool isBonded( const CBTDevice &dev );
	
	CBtDevExtension* GetDeviceFromRepositoryL( const TBTDevAddr& addr );
	
	void SendSelectedDeviceL( CHbSymbianVariantMap& aData );
    
private: // data    

    /**
     * Reference to our parent the server class.
     * Not own.
     */
    CBTNotifServer& iServer;
    
    /**
     * Pointer to an outstanding user interaction.
     * Not own.
     */
    CBluetoothNotification* iNotification;
    
    /**
     * device inquiry handler:
     */
    CAdvanceDevDiscoverer* iDiscoverer;
    
    /**
     *  own's the elements in the array.
     */
    RDevExtensionArray iDevices;
    
    /**
     * The message for a pending device selection request from a RNotifier client.
     */
    RMessage2 iMessage;
    
    TBool iRepositoryInitialized;
    
    TBool iLoadDevices;
    
    };

#endif // BTNOTIFDEVICESELECTOR_H
