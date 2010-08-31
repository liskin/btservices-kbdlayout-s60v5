/*
* ============================================================================
*  Name        : btnotificationresult.h
*  Part of     : bluetoothengine / btnotif
*  Description : Abstract interface for receiving the result of a user query.
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

#ifndef BTNOTIFICATIONRESULT_H
#define BTNOTIFICATIONRESULT_H

class CHbSymbianVariantMap;

/**
 *  MBTNotificationResult returns the result from a user query.
 *
 *
 *  @since Symbian^4
 */
class MBTNotificationResult
    {

public:

    /**
     * Handle an intermediate result from a user query.
     * This function is called if the user query passes information
     * back before it has finished i.e. is dismissed. The final acceptance/
     * denial of a query is passed back in MBRNotificationClosed.
     *
     * @since Symbian^4
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRDataReceived( CHbSymbianVariantMap& aData ) = 0;

    
    /**
     * The notification is finished. The resulting data (e.g. user input or
     * acceptance/denial of the query) is passed back here.
     *
     * @since Symbian^4
     * @param aErr KErrNone or one of the system-wide error codes.
     * @param aData the returned data. The actual format 
     *              is dependent on the actual notifier.
     */
    virtual void MBRNotificationClosed( TInt aError, const TDesC8& aData ) = 0;

    };


#endif // BTNOTIFICATIONRESULT_H
