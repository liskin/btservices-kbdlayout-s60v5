/*
* ============================================================================
*  Name        : bluetoothdevicedialogs.h
*  Part of     : bluetoothengine / btnotif
*  Description : Data structures for passing dialog type and formatting between btnotif and the UI component showing the actual dialogs. Also defines identifiers for Bluetooth device dialogs.
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

#ifndef BLUETOOTHDEVICEDIALOGS_H
#define BLUETOOTHDEVICEDIALOGS_H

#include <btdevice.h>

/**  Enumeration for the type of notification to be launched. */
enum TBTDialogResourceId
    {
    ENoResource,
    EAuthorization,
    EIncomingPairing,
    EPinInput,
    ENumericComparison,
    EPasskeyDisplay,
    EObexPasskeyInput,
    ESetTrusted,
    EBlockUnpairedDevice,
    EBlockPairedDevice,
    EPairingSuccess,
    EPairingFailureRetry,
    EPairingFailureOk,
    EVisibilityTimeout,
    EUnpairedDevice,
    EUnusedResource	// The last ID
    };


/**
 *  TBluetoothDialogParams contains device dialog parameters except for device name and type
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( TBluetoothDialogParams )
    {

public:

    /**  Enumeration for the type of notification to be launched. */
    enum TBTDialogType
        {
        EInvalidDialog,
        ENote,
        EQuery,
        EInput,
        EMoreDevice,
        EDeviceSearch,
        ESend,
        EGlobalNotif,
        EUserAuthorization,
        EReceiveProgress,
        ETransferFailed, 
        EReceiveDone
        };

    /**  Enumeration for the notification parameters data type to be configured. */
    enum TDialogDataType
        {
        ENoParams,
        EDialogType,
        EResource,
        EAddress,
        EDialogTitle,
        EDialogExt  // Id for first data type of derived class.
        };
    
    /** Enumeration for the notification dialog heading type. */
    enum TDialogTitleDataType
        {
        EReceive,
        EReceiveFromPairedDevice,
        EConnect,
        EPairingRequest,
        ERecvFailed,
        ESendFailed,
        EReceiveCompleted
        };
    
    enum TDialogActionType
        {
        EHide,
        EShow,
        ECancelReceive,
        ECancelShow
        };

    inline TBluetoothDialogParams();

public: // data

    /**
     * Identifier for the type of notification.
     */
    TInt iDialogType;

    /**
     * Identifier for the resource to be shown in the dialog.
     */
    TInt iResourceId;

    /**
     * Address of the remote device (not used, for future extension).
     */
    TBTDevAddr iAddr;

    /**
     * Unused padding (not used, for future extension).
     */
    TUint32 iPadding;

    };


/**
 *  TBluetoothDeviceDialog contains more device dialog parameters 
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( TBluetoothDeviceDialog ) : public TBluetoothDialogParams
    {

public:

    /**  Enumeration for the notification parameters data type to be configured. */
    enum TDialogDataType
        {
        ENoParams = TBluetoothDialogParams::EDialogExt,
        EDeviceName,
        EDeviceClass,
        EAdditionalInt,
        EAdditionalDesc,
        EReceivingFileName,
        EReceivingFileSize,
        EReceivedFileCount
        };

    inline TBluetoothDeviceDialog();

public: // data

    /**
     * The name of the remote device.
     */
    TBTDeviceName iDeviceName;

    /**
     * The class of device of the remote device.
     */
    TInt iDeviceClass;

    /**
     * Additional data depending on the resource.
     */
    TInt iIntParam;

    /**
     * Additional data depending on the resource.
     */
    TBuf<10> iDescParam;

    };


/**  Typedef'ed pckgbuf to send dialog parameters to the notification framework. */
typedef TPckgBuf<TBluetoothDialogParams> TBluetoothDialogParamsPckg;

/**  Typedef'ed pckgbuf to send dialog parameters to the notification framework. */
typedef TPckgBuf<TBluetoothDeviceDialog> TBluetoothDeviceDialogPckg;


inline TBluetoothDialogParams::TBluetoothDialogParams()
:   iDialogType( EInvalidDialog ),
    iResourceId( ENoResource ),
    iAddr( TBTDevAddr() ),
    iPadding( 0 )
    {
    }

inline TBluetoothDeviceDialog::TBluetoothDeviceDialog()
:   iDeviceName( 0 ),
    iDeviceClass( 0 ),
    iIntParam( 0 )
    {
    }


#endif // BLUETOOTHDEVICEDIALOGS_H
