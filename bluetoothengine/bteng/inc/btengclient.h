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
* Description:  BTEng server header definition
*
*/



#ifndef BTENGCLIENT_H
#define BTENGCLIENT_H


#include <bt_sock.h>

#include "btengdomaincrkeys.h"
#include "btengclientserver.h"
#include "btengconstants.h"


/**
 *  Class RBTEng
 *
 *  ?more_complete_description
 *
 *  @lib bteng*.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( RBTEng ) : public RSessionBase
    {

public:

    /**
     * Constructor.
     *
     * @since S60 v3.2
     */
    RBTEng();

    /**
     * ?description
     *
     * @since S60 v3.2
     */
    TInt Connect();

    /**
     * Get the version information about the client.
     *
     * @since S60 v3.2
     * @return ?description
     */
    TVersion Version();

    /**
     * Set Bluetooth power state.
     *
     * @since S60 v3.2
     * @param aState The new power state.
     * @param aTemp Turn BT off after use (ETrue) or not (EFalse).
     * @return ?description
     */
    TInt SetPowerState( const TBTPowerStateValue aState, const TBool aTemp );

    /**
     * Set Bluetooth power state.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @return ?description
     */
    TInt SetVisibilityMode( const TBTVisibilityMode aMode, const TInt aTime );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @return ?description
     */
    TInt ConnectDevice( const TBTDevAddr& aAddr, 
                         const TBTDeviceClass& aDeviceClass  );
	
    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @return ?description
     */
    TInt CancelConnectDevice( const TBTDevAddr& aAddr );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @return ?description
     */
    TInt DisconnectDevice( const TBTDevAddr& aAddr, TBTDisconnectType aDiscType );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    TInt IsDeviceConnected( const TBTDevAddr& aAddr, 
                             TBTEngConnectionStatus& aConnected );
    
    /**
     * ?description
     *
     * @since S60 v5.1
     * @param aAddr ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    TInt IsDeviceConnectable( const TBTDevAddr& aAddr,
            const TBTDeviceClass& aDeviceClass, TBool& aConnectable );
    
    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @return ?description
     */
    TInt GetConnectedAddresses( TDes8& aArrayPkg, TBTEngParamPkg& aProfilePkg );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aEvent ?description
     * @return ?description
     */
    TInt NotifyConnectionEvents( TDes8& aEvent, TRequestStatus& aStatus );
    
    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aEvent ?description
     * @return ?description
     */
    TInt CancelNotifyConnectionEvents();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @return ?description
     */
    TInt PrepareDiscovery();
    
    };


#endif // BTENGCLIENT_H
