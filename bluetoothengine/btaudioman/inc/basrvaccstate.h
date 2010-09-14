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
* Description:  The base accessory state declaration
*
*/


#ifndef C_BTASRVACCSTATE_H
#define C_BTASRVACCSTATE_H

#include <e32base.h> 
#include <bttypes.h>
#include <btengconstants.h>
#include "basrvactive.h"
#include "basrvacc.h"
#include "basrvaccman.h"
#include "basrvpluginman.h"
#include "btaccPlugin.h"
#include "btaccTypes.h"
#include "BTAccInfo.h"
#include "BTAccFwIf.h"

class CBasrvAccState : public CBase, public MBasrvActiveObserver
    {
public:

    virtual ~CBasrvAccState();
    
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
    virtual CBasrvAccState* ErrorOnEntry(TInt aReason);
    
    virtual TBTEngConnectionStatus ConnectionStatus() const = 0;
    
    /**
    * Connect to an accessory. Default inplementation returns KErrInUse.
    * @param aAddr BD address of the accessory to be connected
    * @return an error
    */
    virtual void ConnectL(const TBTDevAddr& aAddr);
    
    /**
    * Cancel connecting to accessory
    */
    virtual void CancelConnect();
    
    /**
    * Disconnect accessory. Default implementation returns KErrNotFound.
    * @param aAddr BD address of the accessory to be connected
    * @return an error
    */
    virtual void DisconnectL();

    /**
    * 
    * @return an error. Default implementation returns KErrNotFound
    */
    virtual void AccessoryConnected(TProfiles aProfile);
    
    /**
    * 
    * @return an error. Default implementation returns KErrNotFound
    */
    virtual void AccOpenedAudio(TProfiles aProfile);
    
    /**
    * 
    * @return an error. Default implementation returns KErrNotFound
    */
    virtual void AccClosedAudio(TProfiles aProfile);
    
    /**
    * Called when the connection is disconnected from remote side or there
    * is error on reading/writing data packet.
    * @param aErr the error code of event
    */
    virtual void AccessoryDisconnectedL(TProfiles aProfile);
    
    /**
    * Called when an audio open request comes from Acc FW.
    */
    virtual void OpenAudioL(TAccAudioType aType);
    
    /**
    * Called when the audio open request is to be cancelled.
    */
    virtual void CancelOpenAudio();
    
    /**
    * Called when an audio close request comes from Acc FW.
    */
    virtual void CloseAudioL(TAccAudioType aType);
    
protected:
    // From MBasrvActiveObserver
    
    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void RequestCompletedL(CBasrvActive& aActive);
    
    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void CancelRequest(CBasrvActive& aActive);

protected:

    /**
    * Default constructor
    */
    CBasrvAccState(CBasrvAcc& aParent);
    
    /**
     * Gets the owner of the state machine
     *
     * @since S60 v3.1
     * @return the state machine owner
     */
    CBasrvAcc& Parent();
    
    void StatePrint(const TDesC& aStateName);

    TAccInfo& AccInfo();

    void NewProfileConnection(TProfiles aProfile);

    void ProfileDisconnected(TProfiles aProfile);
    
    TBool IsAvrcpTGCat2SupportedByRemote();

protected:

    CBasrvAcc& iParent;
    };

#endif      // C_BTASRVACCSTATE_H

// End of File
