/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BT socket event observer declaration.
*
*/


#ifndef M_BTMSOCKOBSERVER_H
#define M_BTMSOCKOBSERVER_H

//  INCLUDES
#include <bttypes.h>

/**
 *  BT socket event observer
 *
 *  This class defines the interfaces o receive events from a Bluetooth socket
 *
 *  @since S60 v3.1
 */
class MBtmSockObserver
    {
public:

    /**
     * Notification of a RFCOMM socket connect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void RfcommConnectCompletedL(TInt aErr);

    /**
     * Notification of a RFCOMM socket accpet completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     * @param aService the connected profile
     */
    virtual void RfcommAcceptCompletedL(TInt aErr, TUint aService);

    /**
     * Notification of a RFCOMM socket shutdown completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void RfcommShutdownCompletedL(TInt aErr);

    /**
     * Notification of a RFCOMM socket send completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void RfcommSendCompletedL(TInt aErr);

    /**
     * Notification of a RFCOMM socket receive completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void RfcommReceiveCompletedL(TInt aErr, const TDesC8& aData);

    /** 
     * Notification of an ioctl complete event.
     * 
     * @since S60 v3.1
     * @param aErr the returned error
     */
    virtual void RfcommIoctlCompleteL(TInt aErr);

    virtual void RfcommLinkInActiveModeL();

    virtual void RfcommLinkInSniffModeL();

    /**
     * Notification of a synchronous socket setup completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void SyncLinkSetupCompleteL(TInt aErr);

    /**
     * Notification of a synchronous socket disconnect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void SyncLinkDisconnectCompleteL(TInt aErr);

    /**
     * Notification of a synchronous socket accept completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void SyncLinkAcceptCompleteL(TInt aErr);

    /**
     * Notification of a synchronous socket send completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void SyncLinkSendCompleteL(TInt aErr);

    /**
     * Notification of a synchronous socket send completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    virtual void SyncLinkReceiveCompleteL(TInt aErr);
    
    };

#endif    // M_BTMSOCKOBSERVER_H
