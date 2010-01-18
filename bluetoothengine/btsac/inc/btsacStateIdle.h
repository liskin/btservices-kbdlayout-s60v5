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


#ifndef C_BTSACIDLE_H
#define C_BTSACIDLE_H

#include "btsacState.h"

/**
 * The state Idle
 *
 *
 *  @since S60 v3.1 
 */
class CBtsacIdle : public CBtsacState
    {
    
public:

    static CBtsacIdle* NewL(CBTSAController& aParent);
    
    virtual ~CBtsacIdle();
    
private:
    
    // From base class CBtsacState
    
    /**
     * From CBtsacState
     * Entry of this state.
     */
    void EnterL();
    
    // From base class MInternalGavdpUser
    
    void GAVDP_ConfigurationConfirm();
    void GAVDP_Error(TInt aError, const TDesC8& aErrorData);

private:    

    CBtsacIdle(CBTSAController& aParent);
    };

#endif // C_BTSACIDLE_H
            