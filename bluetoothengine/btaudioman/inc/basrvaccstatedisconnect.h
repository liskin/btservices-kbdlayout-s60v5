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
* Description:  The Disconnect state declaration
*
*/


#ifndef C_BTASRVACCSTATEDISCONNECT_H
#define C_BTASRVACCSTATEDISCONNECT_H

#include "basrvaccstate.h"

class CBasrvAccStateDisconnect : public CBasrvAccState
    {
public:

    static CBasrvAccStateDisconnect* NewL(CBasrvAcc& aParent, TInt aConnErr = KErrNone);

    virtual ~CBasrvAccStateDisconnect();
    
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
    CBasrvAccStateDisconnect(CBasrvAcc& aParent, TInt aConnErr);

    void DoDisconnectL();
    
protected:
    CBasrvActive* iActive;
    TInt iDiscErr;
    TInt iConnErr;
    TInt iProfiles;
    };

#endif      // C_BTASRVACCSTATEDISCONNECT_H

// End of File
