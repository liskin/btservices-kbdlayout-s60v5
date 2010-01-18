/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BT Mono Audio Controller class declaration.
*
*/


#ifndef REFERENCE_ATEXT_IMPL_H
#define REFERENCE_ATEXT_IMPL_H

#include <atextpluginbase.h>
#include "genericactive.h"

NONSHARABLE_CLASS(CAtExtPluginStub) : public CATExtPluginBase, public MGenericActiveObserver
  	{
  	public:
        
        /**
        * Two-phased constructor.
        */
        static CAtExtPluginStub* NewL();
        
        /**
        * Destructor.
        */
        ~CAtExtPluginStub();

private:
    
    void HandleCommand( const TDesC8& aCmd, RBuf8& aReply );

private:

    void RequestCompletedL(CGenericActive& aActive);

    void CancelRequest(CGenericActive& aActive);

    TInt HandleRunError(TInt aErr);
    

private:

    CAtExtPluginStub();
    
    void ConstructL();
       
    void StartTimer(TInt aTimeout, TInt aId);
    
private:    // Data
    CGenericActive* iActive;
    RTimer iTimer;
    };

#endif      // REFERENCE_ATEXT_IMPL_H
            
// End of File
