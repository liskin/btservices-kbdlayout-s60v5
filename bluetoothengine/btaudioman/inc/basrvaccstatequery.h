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


#ifndef C_BTASRVACCSTATEQUERY_H
#define C_BTASRVACCSTATEQUERY_H

#include "basrvaccstate.h"
#include "basrvsdpquery.h"

class CBasrvAccStateQuery : public CBasrvAccState, public MBasrvSdpQuerier
    {
public:

    static CBasrvAccStateQuery* NewL(CBasrvAcc& aParent, TBool aConnectingRequest);

    virtual ~CBasrvAccStateQuery();
    
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

private:
    // From MBasrvSdpQuerier

    void GetAccInfoCompletedL(TInt aErr, const TAccInfo* aAcc, const TBTDeviceClass* aCod);

private:

    CBasrvAccStateQuery(CBasrvAcc& aParent, TBool aConnectingRequest);
    
    void ConstructL();
    
    void ResolveConflicts(RArray<TBTDevAddr>& aConflicts);
    
private:
    CBasrvSdpQuery* iQuery;
    TBool iConnecting;
    };

#endif      // C_BTASRVACCSTATEQUERY_H

// End of File
