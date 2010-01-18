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
* Description:  The base state declaration
*  Version     : %version: 2.2.5 %
*
*  Copyright © 2005-2006 Nokia.  All rights reserved.
*/


#ifndef C_BTMSTATE_H
#define C_BTMSTATE_H

#include <e32base.h>
#include <es_sock.h>
#include <bttypes.h>
#include <bttypes.h>
#include "btmman.h"
#include "btmsockobserver.h"

enum TNotifyItem
    {
    ENotifyNone = 0,
    ENotifyAudioClosedByRemote,
    ENotifyAudioOpenedByRemote,
    };

enum TRequestCategory
    {
    ERequestNone,
    ERequestConnect,
    ERequestOpenAudio,
    };


/**
 *  the base class of the state machine
 *
 *  @since S60 v3.1
 */
class CBtmState : public CBase, public MBtmSockObserver
    {
public:

    virtual ~CBtmState();
    
    void SetNotifyItemAtEntry(TNotifyItem aNotify);
    
    TNotifyItem NotifyItem();    
    
    /**
     * Entry of this state.
     *
     * @since S60 v3.1
     */
    virtual void EnterL() = 0;
    
    /**
     * Returns the next state if EnterL leaves.
     *
     * @since S60 v3.1
     * @return the next state
     */
    virtual CBtmState* ErrorOnEntryL(TInt aReason);

    /**
     * Starts bt audio listner
     * @since S60 v3.2
     */
    virtual void StartListenerL();
    
    /** 
     * Connect to BT accessory 
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    virtual void ConnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * Cancel the outstanding connecting operation
     *
     * @since S60 v3.1
     */
    virtual void CancelConnectL();

    /** 
     * disconnect the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    virtual void DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * Open audio link (synchronous) to the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    virtual void OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * Cancel the outstanding operation
     *
     * @since S60 v3.1
     */
    virtual void CancelOpenAudioLinkL(const TBTDevAddr& aAddr);

    /** 
     * Close the audio link (synchronous) to the connected accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    virtual void CloseAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);
    
    /** 
     * Cancel closing audio link
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     */
    virtual void CancelCloseAudioLinkL(const TBTDevAddr& aAddr);
    
    /** 
     * Called when accessory is in use (attached to AccessoryServer). 
     *
     * @since S60 v3.1
     */
    virtual void AccInUse();

	/*
	* Defaults from observer base
	*/
	virtual void SlcIndicateL(TBool aSlc);
	
	virtual void SendProtocolDataL(const TDesC8& aData);
	
	virtual TBool CanDisableNrec();
	
	virtual TBTDevAddr Remote();
	
	virtual TInt AudioLinkLatency();   
	
protected:

    CBtmState(CBtmMan& aParent, TRequestStatus* aRequest);

    /**
     * Gets the owner of the state machine
     *
     * @since S60 v3.1
     * @return the state machine owner
     */
    CBtmMan& Parent();
    
    /**
     * Completes the pending request in this state with specified error code 
     * and then zero the pointer.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void CompleteStateRequest(TInt aErr);
    
    /**
     * Completes the give pending request with specified error code..
     *
     * @since S60 v3.1
     * @param aRequest the request to be completed
     * @param aErr the completion error code
     */
    void CompleteRequest(TRequestStatus* aRequest, TInt aErr);
    
    TRequestStatus* StateRequest();
    
    void SetStateRequest(TRequestStatus& aStatus);
    
    TRequestStatus* SwapStateRequest();
        
private:

    /**
     * the owner of the state machine
     * Not own.
     */
    CBtmMan& iParent;
        
    /**
     * Possible to be NULL.
     * A not NULL iRequest means the async request performed in this state machine.
     * It must be completed when state machine will transfer to another state.
     */
    TRequestStatus* iRequest;
    
    TNotifyItem iNotify;
    
    };

#endif // C_BTMSTATE_H
            
