/*
* ============================================================================
*  Name        : btnotifwrapper.h
*  Part of     : bluetoothengine / btnotif
*  Description : Wrapper for Bluetooth Notifiers
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

#ifndef BTNOTIFWRAPPER_H
#define BTNOTIFWRAPPER_H


#include <eiknotapi.h>
#include <btnotifclient.h>
#include <btservices/btsimpleactive.h>

class CBTNotifWrapperUnitTester;

/**
 * Function for creating the array of notifiers supported by this plug-in.
 * The main purpose for declaring it here is to allow it to be used
 * by the test module.
 *
 * @since Symbian^4
 * @return Array of notifiers.
 */
CArrayPtr<MEikSrvNotifierBase2>* CreateNotifierArrayL();

/**
 *  RNotif plugin implementation providing the client API for BTNotif 
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifWrapper ) : public CBase,
                                       public MEikSrvNotifierBase2,
                                       public MBtSimpleActiveObserver
    {

public:

    /**
     * Two-phased constructor.
     */
    static CBTNotifWrapper* NewLC( const TUid& aUid, const TUid& aChannel );

    /**
    * Destructor.
    */
    virtual ~CBTNotifWrapper();

private:
    
// from base class MEikSrvNotifierBase2

    /**
     * From MEikSrvNotifierBase2.
     * Frees all resources owned by this notifier.
     *
     * @since Symbian^4
     */
    virtual void Release();

    /**
     * From MEikSrvNotifierBase2.
     * Performs any initialisation that this notifier may require.
     * As a minimum, the function should return a TNotifierInfo instance 
     * describing the notifier parameters.
     *
     * @since Symbian^4
     * @return Describes the parameters of the notifier.
     */
    virtual TNotifierInfo RegisterL();

    /**
     * From MEikSrvNotifierBase2.
     * Gets the notifier parameters.
     *
     * @since Symbian^4
     * @return Describes the parameters of the notifier.
     */
    virtual TNotifierInfo Info() const;

    /**
     * From MEikSrvNotifierBase2.
     *  This is called as a result of a client-side call to 
     *  RNotifier::StartNotifier(), which the client uses to 
     *  start a notifier from which it does not expect a response.
     *
     * @since Symbian^4
     * @param aBuffer Data that can be passed from the client-side. The format 
     *                and meaning of any data is implementation dependent.
     * @return A pointer descriptor representing data that may be returned. 
     *         The format and meaning of any data is implementation dependent.
     */
    virtual TPtrC8 StartL(const TDesC8& aBuffer);

    /**
     * From MEikSrvNotifierBase2.
     *  This is called as a result of a client-side call to the asynchronous 
     *  function RNotifier::StartNotifierAndGetResponse(). This means that the 
     *  client is waiting, asynchronously, for the notifier to tell the 
     *  client that it has finished its work.
     *
     * @since Symbian^4
     * @param aBuffer Data that can be passed from the client-side. The format 
     *                and meaning of any data is implementation dependent.
     * @param aReplySlot Identifies which message argument to use for the reply.
     *                   This message argument will refer to a modifiable 
     *                   descriptor, a TDes8 type, into which data can be 
     *                   returned. The format and meaning of any returned data 
     *                   is implementation dependent.
     * @param aMessage Encapsulates a client request.
     */
    virtual void StartL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage);

    /**
     * From MEikSrvNotifierBase2.
     * Cancels an active notifier.
     * This is called as a result of a client-side call to RNotifier::CancelNotifier().
     *
     * @since Symbian^4
     */
    virtual void Cancel();

    /**
     * From MEikSrvNotifierBase2.
     * Updates a currently active notifier with new data.
     * This is called as a result of a client-side call 
     * to RNotifier::UpdateNotifier().
     *
     * @since Symbian^4
     * @param aBuffer Data that can be passed from the client-side. The format 
     *                and meaning of any data is implementation dependent.
     * @return A pointer descriptor representing data that may be returned. 
     *         The format and meaning of any data is implementation dependent.
     */
    virtual TPtrC8 UpdateL(const TDesC8& aBuffer);

    /**
     * From MEikSrvNotifierBase2.
     * Updates a currently active notifier with new data.
     * This is called as a result of a client-side call to the asynchronous 
     * function RNotifier::UpdateNotifierAndGetResponse().
     *
     * @since Symbian^4
     * @param aBuffer Data that can be passed from the client-side. The format 
     *                and meaning of any data is implementation dependent.
     * @param aReplySlot Identifies which message argument to use for the reply.
     *                   This message argument will refer to a modifiable 
     *                   descriptor, a TDes8 type, into which data can be 
     *                   returned. The format and meaning of any returned data
     *                   is implementation dependent.
     * @param aMessage Encapsulates a client request.
     */
    virtual void UpdateL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage);

// from base class MBtSimpleActiveObserver

    /**
     * Callback to notify that an outstanding request has completed.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus );

    /**
     * Callback for handling cancelation of an outstanding request.
     *
     * @since Symbian^4
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId );

    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aError The error occurred in RunL.
     */
    virtual void HandleError( CBtSimpleActive* aActive, TInt aError );
    
private:

    CBTNotifWrapper( const TUid& aUid, const TUid& aChannel );

    void ConstructL();

    /**
     * Check if this notifier is a synchronous or asunchronous notifier.
     *
     * @since Symbian^4
     * @return ETrue is the notifier is synchronous, EFalse if asyncrhonous.
     */
    TBool IsSync() const;

private: // data

    /**
     * The UID identifying the notifier.
     */
    TUid iUid;

    /**
     * The channel of the notifier.
     */
    TUid iChannel;

    /**
     * The replyslot for the result.
     */
    TInt iReplySlot;

    /**
     * Session with the BT notifier server.
     */
    RBTNotifier iBTNotif;

    /**
     * The client-side message containing the current request.
     */
    RMessagePtr2 iMessage;

    /**
     * Buffer containing a copy of the notifier parameters, for async notifiers.
     * Own.
     */
    RBuf8 iParamsBuf;

    /**
     * Modifiable pointer descriptor for getting the response back.
     */
    TPtr8 iResponsePtr;

    /**
     * Buffer for receiving the response from the BT notifier server.
     * Own.
     */
    RBuf8 iResponseBuf;

    /**
     * Active object helper class.
     * Own.
     */
    CBtSimpleActive* iActive;

    friend class CBTNotifWrapperUnitTester;
    
    };

#endif // BTNOTIFWRAPPER_H
