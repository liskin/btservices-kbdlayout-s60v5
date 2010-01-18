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
* Description:  The Query state declaration
*
*/


#ifndef C_BTASRVACCSTATEATTACH_H
#define C_BTASRVACCSTATEATTACH_H

#include "basrvaccstate.h"

class CBasrvAccStateAttach : public CBasrvAccState
    {
public:

    static CBasrvAccStateAttach* NewL(CBasrvAcc& aParent, TBool aConnecting);

    virtual ~CBasrvAccStateAttach();
    
    /**
     * Entry of this state.
     *
     * @since S60 v3.1
     */
    void EnterL();
    
    /**
     * Returns the next state if EnterL leaves.
     *
     * @since S60 v3.1
     * @return the next state
     */
    CBasrvAccState* ErrorOnEntry(TInt aReason);
    
    TBTEngConnectionStatus ConnectionStatus() const;
    
    /**
    * Cancel connecting to accessory
    */
    void CancelConnect();

    void DisconnectL();

   
    /**
    * Called when the connection is disconnected from remote side or there
    * is error on reading/writing data packet.
    * @param aErr the error code of event
    */
    void AccessoryDisconnectedL(TProfiles aProfile);
    
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
    CBasrvAccStateAttach(CBasrvAcc& aParent, TBool aConnecting);

    void ConstructL();
    
protected:
    CBasrvActive* iActive;
    TBool iConnecting;
    };

#endif      // C_BTASRVACCSTATEATTACH_H

// End of File
