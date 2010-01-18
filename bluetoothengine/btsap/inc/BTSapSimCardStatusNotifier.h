/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     This class sends SIM card status indication to a client
*
*
*/


#ifndef BT_SAP_SIM_CARD_STATUS_NOTIFIER_H
#define BT_SAP_SIM_CARD_STATUS_NOTIFIER_H

//  INCLUDES
#include <e32base.h>

#ifdef _DUMMY_CUSTOM_API
#include <DummyCustomAPI.h>
#else
#include <rmmcustomapi.h>
#endif

// FORWARD DECLARATIONS
class CBTSapServerState;

// CLASS DECLARATION

/**
* SIM card status handler class.
*/
class CBTSapSimCardStatusNotifier : public CActive
    {
public:
    
    // Constructors
    static CBTSapSimCardStatusNotifier* NewL(CBTSapServerState& aBTSapServerState);
    
    // Destructor
    ~CBTSapSimCardStatusNotifier();
        
private:     // From CActive
   
    /**
    * DoCancel() has to be implemented by all the classes deriving CActive.
    * Request cancellation routine.
    * @param none
    * @return none 
    */
    void DoCancel();
    
    /**
    * RunL is called by framework after request is being completed.
    * @param nones
    * @return none 
    */
	void RunL();

public:    
    
    /**
    * Start waiting for SIM card status change notification
    * @param    None
    * @return     None.
    */    

    void Start();
    
private:

    // Two-phase constructor
    void ConstructL();

    // Default constructor
    CBTSapSimCardStatusNotifier(CBTSapServerState& aBTSapServerState);

private:

    CBTSapServerState&           iBTSapServerState;
    RMmCustomAPI&                iSubscriptionModule;
    RMmCustomAPI::TSIMCardStatus iCardStatus;
    };

#endif      // BT_SAP_SIM_CARD_STATUS_NOTIFIER_H   
            
// End of File
