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
* Description:  Object push controller
*
*/



#ifndef BT_OPP_CONTROLLER_H
#define BT_OPP_CONTROLLER_H

//  INCLUDES
#include "BTServiceClient.h"
#include "BTServiceStarter.h"
#include "BTSController.h"

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Control class for the Object sending
*/
NONSHARABLE_CLASS (CBTSOPPController) : public CBTSController, 
                                        public MBTServiceClientObserver                                        

    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTSOPPController* NewL( MBTServiceObserver* aObserver,
            const TUint aRemotePort,
            const TBTDevAddr& aRemoteDevice,
            CBTServiceParameterList* aList  );
        
        /**
        * Destructor.
        */
        virtual ~CBTSOPPController();

    private: // Functions from base classes

        /**
        * From MBTServiceClientObserver A Connect operation has been completed.
        * @param aStatus The status of the operation.
        * @return None.
        */
        void ConnectCompleted( TInt aStatus );

        /**
        * From MBTServiceClientObserver The client connection has been closed.
        * @param None.
        * @return None.
        */
        void ClientConnectionClosed();

        /**
        * From MBTServiceClientObserver A Put operation has been completed. 
        * @param aStatus The status of the operation.
        * @param aPutResponse Response packet from remote device.
        * @return None.
        */
        void PutCompleted( TInt aStatus, const CObexHeaderSet* aPutResponse );
        
        /**
        * From MBTServiceClientObserver A Get operation has been completed. 
        * @param aStatus The status of the operation.
        * @param aGetResponse Response packet from remote device.
        * @return None.
        */
        void GetCompleted( TInt aStatus, CObexBufObject* aGetResponse );
        
        /**
        * From MBTServiceClientObserver A Connect operation is timed out.
        * @return None.
        */
        void ConnectTimedOut();
        
  
    private:

        /**
        * C++ default constructor.
        */
        CBTSOPPController( MBTServiceObserver* aObserver,
                           CBTServiceParameterList* aList );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TUint aRemotePort,
                         const TBTDevAddr& aRemoteDeviceAddr );
    
    private:

        /**
        * Send file 
        * @param None.
        * @return None.
        */
        void Send();

        /**
        * Handle Connection indication 
        * @param None.
        * @return None.
        */
        void HandleConnectCompleteIndicationL();
        
        void UpdateProgressNoteL();

    private:    // Data
        
        TInt                        iFileIndex;
        // Not owned
        //        
        CBTServiceParameterList*    iListPtr;
        MBTServiceObserver*         iObserverPtr;
    };

#endif      // BT_OPP_CONTROLLER_H
            
// End of File
