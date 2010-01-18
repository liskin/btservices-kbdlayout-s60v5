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
* Description:  The Connect state declaration
*
*/


#ifndef C_BTASRVACCSTATECONNECT_H
#define C_BTASRVACCSTATECONNECT_H

#include "basrvaccstate.h"

class CBasrvAccStateConnect : public CBasrvAccState
    {
public:

    static CBasrvAccStateConnect* NewL(CBasrvAcc& aParent);

    virtual ~CBasrvAccStateConnect();
    
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
    CBasrvAccStateConnect(CBasrvAcc& aParent);
    void ConstructL();

private:
    CBasrvActive* iActive;
    TProfiles iConnectingProfile;
    };

#endif      // C_BTASRVACCSTATECONNECT_H

// End of File
