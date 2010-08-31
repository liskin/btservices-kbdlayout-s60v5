/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  API for sending services
*
*/



#ifndef BT_SERVICE_API_H
#define BT_SERVICE_API_H

//  INCLUDES
#include "BTServiceParameterList.h"

// CONSTANTS

// DATA TYPES

enum TBTServiceType
    {
    EBTSendingService,
    EBTPrintingService,
    EBTObjectPushService // use this if support for only OPP is wanted
    };

// FORWARD DECLARATIONS
class CBTServiceStarter;

// CLASS DECLARATION

/**
*  An API for starting Bluetooth services.
*
*  @lib BtServiceUtils.lib
*  @since Series 60 2.6
*/
class CBTServiceAPI : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CBTServiceAPI* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CBTServiceAPI();

    public: // New functions
        
        /**
        * Starts the given service.
        * Returns when service is started.
        * @since Series 60 2.6
        * @param aService The service to be started.
        * @param aList Parameters for the service.
        * @return None.
        */
        IMPORT_C void StartServiceL( TBTServiceType aService, 
                                     CBTServiceParameterList* aList );

        /**
        * Starts the given service.
        * Returns when service is completed.
        * @since Series 60 2.6
        * @param aService The service to be started.
        * @param aList Parameters for the service.
        * @return None.
        */
        IMPORT_C void StartSynchronousServiceL( TBTServiceType aService, 
                                     CBTServiceParameterList* aList );
    private:

        /**
        * C++ default constructor.
        */
        CBTServiceAPI();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

        CBTServiceStarter*      iStarter;
        CActiveSchedulerWait    iSyncWaiter;
    };

#endif      // BT_SERVICE_API_H
            
// End of File
