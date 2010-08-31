/*
* ============================================================================
*  Name        : btnotifclient.h
*  Part of     : bluetoothengine / btnotifclient
*  Description : Session class for client-server interaction with btnotifserver.
*
*  Copyright © 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef RBTNOTIFCLIENT_H
#define RBTNOTIFCLIENT_H

#include <e32base.h>
#include <btmanclient.h>

/**
 *  A handle to a session with the Bluetooth notifier server for showing
 *  Bluetooth-related notifications to the user.
 *
 *  ?more_complete_description
 *  @code
 *   ?good_class_usage_example(s)
 *  @endcode
 *
 *  @lib ?library
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( RBTNotifier ) : public RSessionBase
    {
public:

    /**
     * Constructor.
     *
     * @since Symbian^4
     */
    IMPORT_C RBTNotifier();
    
    /**
     * Connect to the Bluetooth notifier server.
     *
     * @since Symbian^4
     * @return KErrNone, if successful. Otherwise one of 
     *         the system-wide error codes.
     */
    IMPORT_C TInt Connect();

    /**
     * Get the version information about the client.
     *
     * @since Symbian^4
     * @return The version of the client-server interface implemented by this client.
     */
    IMPORT_C TVersion Version();

    /**
     * Requests the extended notifier server to start the notifier
     * identified by the specified UID. The request is synchronous; 
     * the call returns when the request is complete.
     *
     * @since Symbian^4
     * @param aNotifierUid The UID identifying the notifier. This is 
     *        the same UID as used with RNotifier for the same purpose.
     * @param aBuffer Data that can be passed to the notifier; the format and 
     *        meaning of this depends on the notifier.
     * @return KErrNone, if successful; KErrNotFound, if there 
     *         is no notifier matching the specified UID; otherwise 
     *         one of the other system-wide error codes.
     */
    IMPORT_C TInt StartNotifier( TUid aNotifierUid, const TDesC8& aBuffer );

    /**
     * Requests the extended notifier server to start the notifier 
     * identified by the specified UID. This is an asynchronous request.
     *
     * @since Symbian^4
     * @param aRs The request status. On request completion, contains KErrNone, 
     *        if successful; KErrNotFound, if there is no notifier matching the 
     *        specified UID; KErrCancel, if the notifier was cancelled through 
     *        RBTNotifier::CancelNotifier; otherwise, one of the other system 
     *        wide error codes.
     * @param aNotifierUid The UID identifying the notifier. This is 
     *        the same UID as used with RNotifier for the same purpose.
     * @param aBuffer Data that can be passed to the notifier; the format and 
     *        meaning of this depends on the notifier.
     * @param aResponse Response data from the notifier; the format and meaning 
     *        of this depends on the notifier.
     */
    IMPORT_C void StartNotifierAndGetResponse( TRequestStatus& aRs, 
                TUid aNotifierUid, const TDesC8& aBuffer, TDes8& aResponse );

    /**
     * Requests the Bluetooth notifier server to cancel the notifier 
     * identified by the specified UID. The request is synchronous; 
     * the call returns when the request is complete.
     *
     * @since Symbian^4
     * @param aNotifierUid The UID identifying the notifier. This is 
     *        the same UID as used with RNotifier for the same purpose.
     * @return KErrNone, if successful; KErrNotFound, if there 
     *         is no notifier matching the specified UID; otherwise 
     *         one of the other system-wide error codes.
     */
    IMPORT_C TInt CancelNotifier( TUid aNotifierUid );

    /**
     * Requests the Bluetooth notifier server to update the active
     * notifier identified by the specified UID, with the data supplied.
     * The request is synchronous; the call returns when the request is complete.
     *
     * @since Symbian^4
     * @param aNotifierUid The UID identifying the notifier. This is 
     *        the same UID as used with RNotifier for the same purpose.
     * @param aBuffer Data that can be passed to the notifier; the format and 
     *        meaning of this depends on the notifier.
     * @param aResponse Response data from the notifier; the format and meaning 
     *        of this depends on the notifier.
     * @return KErrNone, if successful; KErrNotFound, if there 
     *         is no notifier matching the specified UID; otherwise 
     *         one of the other system-wide error codes.
     */
    IMPORT_C TInt UpdateNotifier( TUid aNotifierUid, 
                const TDesC8& aBuffer, TDes8& aResponse );

    /**
     * Requests the Bluetooth notifier server to pair with the device 
     * specified by the given Bluetooth device address.
     * The request is asynchronous; Use CancelPairing() to cancel
     * an outstanding pairing request.
     *
     * @since Symbian^4
     * @param aAddr The address of the remote device to perform pairing with.
     * @param aDeviceClass the CoD of the remote device.
     * @param aStatus The request status. On request completion, contains KErrNone, 
     *        if successful; KErrCancel, if the notifier was cancelled through 
     *        RBTNotifier::CancelPair; otherwise, one of the other system 
     *        wide error codes.
     */
    IMPORT_C void PairDevice( const TBTDevAddrPckgBuf& aAddr, TInt32 aDeviceClass,
                TRequestStatus& aStatus );
    
    /**
     * Requests the Bluetooth notifier server to cancel the current pairing 
     * request. The request is synchronous; 
     * the call returns when the request is complete.
     * 
     * If this request is issued when the Bluetooth
     * notifier server has completed pairing with the device, the pairing 
     * will not be un-done. That is, the device will not be unpaired. 
     *
     * @since Symbian^4
     */
    IMPORT_C void CancelPairDevice();

private:

private: // data

    /**
     * ?description_of_member
     */
//    ?type ?member_name;

    };

#endif // RBTNOTIFCLIENT_H
