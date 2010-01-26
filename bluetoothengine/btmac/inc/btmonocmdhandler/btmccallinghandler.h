/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Calling command handler
*
*/


#ifndef C_BTMCCALLINGHANDLER_H
#define C_BTMCCALLINGHANDLER_H


//  INCLUDES
#include <e32base.h>
#include <e32property.h>
#include "btmcactive.h"

// MACROS

// DATA TYPES
    
// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CBtmcProtocol;
class CATCommand;

// CLASS DECLARATION

/**
*  CBtmcCallingHandler listens call state events (from RMobileCall) 
*  and passes them to MBtmcPhoneStatusObserver
*/
NONSHARABLE_CLASS(CBtmcCallingHandler) : public CBase, public MBtmcActiveObserver
    {
public:
    
    /**
    * Two-phased constructor.
    */
    static CBtmcCallingHandler* NewL(CBtmcProtocol& aProtocol);

    /**
    * Destructor.
    */
    ~CBtmcCallingHandler();
    
    void HandleCallingCmdL(const CATCommand& aCmd);   
    
    TBool ActiveCmdHandling() const;
    
    /**
     * Tells if AT+CHLD command handling is in progress
     */
    TBool ActiveChldHandling() const;
    
private:  // From MBtmcActiveObserver

    void RequestCompletedL(CBtmcActive& aActive, TInt aErr);
    
    void CancelRequest(TInt aServiceId);
        
private:

    CBtmcCallingHandler(CBtmcProtocol& aProtocol);

    void ConstructL();

private:
    CBtmcProtocol& iProtocol; // unowned
    
    CBtmcActive* iActive;
    
    // for sending AT commands to btmonobearer
    RProperty iCmdProperty;
    
    // for receiving AT responses from btmonobearer
    RProperty iRespProperty;
    
    TInt iCmdId; 
};
    
#endif  // C_BTMCCALLINGHANDLER_H

// End of File
