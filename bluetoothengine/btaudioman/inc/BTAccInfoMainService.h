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
* Description:  Loaded by Accessory Server to allow this component to initialize
*
*/


#ifndef BTACCINFOMAINSERVICE_H
#define BTACCINFOMAINSERVICE_H

//  INCLUDES
#include <AccessoryServer.h>
#include <AsyMainServiceBase.h>

// CLASS DECLARATION

/**
*  Creates ASY main service
*
*  @lib BTAccInfo.dll
*  @since Series 60 3.1
*/
class CBTAccInfoMainService : private CASYMainServiceBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.        
        */
        static CBTAccInfoMainService* NewL();                        

    public: // Functions from base classes
                
        /**
        * Startup function after creating object
        * @since Series 60 3.1
        * @return Error code of startup.
        *         KErrNone if successfully
        */
        TInt StartL();
        
    private:   // Functions

        /**
        * By default Symbian 2nd phase constructor is private.
        */        
        CBTAccInfoMainService();        

        /**
        * Destructor.
        */
        ~CBTAccInfoMainService();
    
    };

#endif // BTACCINFOMAINSERVICE_H
            
// End of File
