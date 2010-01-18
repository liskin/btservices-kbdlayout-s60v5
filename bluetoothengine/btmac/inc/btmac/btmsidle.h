/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  state Idle declaration.
*
*/


#ifndef C_BTMSIDLE
#define C_BTMSIDLE

#include "btmstate.h"

/**
 * The state Idle
 *
 * This state indicates there is no connection and no pending socket accept 
 * operation issued in BTMAC.
 *
 *  @since S60 v3.1 
 */
class CBtmsIdle : public CBtmState
    {
    
public:

    static CBtmsIdle* NewL(CBtmMan& aParent);
    
    virtual ~CBtmsIdle();
    
private:
    
    // From base class CBtmState
    
    /**
     * From CBtmState
     * Entry of this state.
     */
    void EnterL();
    
    /** 
     * From CBtmState
     * Connect to BT accessory
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the accessory
     * @param aStatus On completion, will contain an error code
     */
    void ConnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

private:    

    CBtmsIdle(CBtmMan& aParent);

    };

#endif      // C_BTMSIDLE
            