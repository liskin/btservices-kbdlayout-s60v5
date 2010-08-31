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
* Description: 
*
*/

#ifndef BTNOTIFUTIL_H
#define BTNOTIFUTIL_H

#include <e32base.h>
#include <bttypes.h>

class CBtDevExtension;

/**
 *  Utility class providing common functions required by various classes within
 *  btnotifsrv.
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( TBTNotifUtil )
    {
public:

    /**
     * Gets the name of a device for displaying in UI.
     *  
     * @param aNameBuf The buffer that will contain the name at return.
     * @param aDevExt the DevExt instance representing the remote device.
     * @param aNameInParam the given name from the parameter of a notifier request.
     * @param aAddr the address of the device. Used only when aDevExt is null.
     */
    static void GetDeviceUiNameL( TDes& aNameBuf, 
            const CBtDevExtension* aDevExt, 
            const TDesC& aNameInParam, const TBTDevAddr& aAddr );
    };

#endif // BTNOTIFUTIL_H
