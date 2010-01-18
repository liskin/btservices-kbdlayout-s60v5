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
* Description:  Loaded by Accessory Server to retrieve information of an accessory
*
*/


#ifndef BTACCINFOCMDHANDLER_H
#define BTACCINFOCMDHANDLER_H

//  INCLUDES
#include <AsyCmdTypes.h>
#include <e32base.h>
#include <AsyCommandHandlerBase.h>

#include "BTAccInfoRequester.h"

// CLASS DECLARATION

/**
*  Implements interface provided by Accessory Server. This interface
*  is used to get information about a bluetooth accessory. 
*
*  @lib BTAccInfo.dll
*  @since Series 60 3.1
*/
class CBTAccInfoCmdHandler : public CASYCommandHandlerBase, public MBTAccInfoNotifier
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param
        * @return The created object.
        */
        static CBTAccInfoCmdHandler* NewL();
        
        /**
        * Destructor.
        */
        ~CBTAccInfoCmdHandler();
        
    public: // Functions from base classes
        
        /**
        * Processes a command from Acccessory Server
        * @since Series 60 3.1
        * @param aCommand Command to process.
        * @param aCmdParams Params of command to process.
        */
        void ProcessCommandL(const TProcessCmdId aCommand, const TASYCmdParams& aCmdParams);
        
        /**
        * Call-back from BTAccInfoRequester; Provides information of an accessory
        * Ownership of TASYBTAccInfo is recieved by this method
        * @since Series 60 3.1
        * @param aErr Whether information of an accessory could be retrieved or not. 
        * @param aAccInfo T-class containing information of an accessory
        */
        void GetBTAccInfoCompletedL(TInt aErr, const TASYBTAccInfo& aAccInfo);
        

    private:    // Functions
             
        /**
        * C++ default constructor.
        */
        CBTAccInfoCmdHandler();

    private:    // Data
        CBTAccInfoRequester* iRequester; // Owned; Pointer to class which is used to get information of an accessory
        
    };

#endif // BTACCINFOCMDHANDLER_H

// End of File

