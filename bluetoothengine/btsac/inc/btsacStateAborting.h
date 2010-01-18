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
* Description:  state Aborting declaration.
*
*/


#ifndef C_BTSACABORTING_H
#define C_BTSACABORTING_H

#include "btsacState.h"

/**
 * The state Aborting
 *
 *
 *  @since S60 v3.1 
 */
class CBtsacAborting : public CBtsacState
    {
    
public:

    static CBtsacAborting* NewL(CBTSAController& aParent);
    
    virtual ~CBtsacAborting();
    
private:
    
    // From base class CBtsacState
    
    /**
     * From CBtsacState
     * Entry of this state.
     */
    void EnterL();        	

	void HandleGavdpErrorL(TInt aError);

private:    

    CBtsacAborting(CBTSAController& aParent);
    };

#endif      // C_BTSACABORTING_H
            