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
* Description:  Basic printing controller
*
*/



#ifndef BT_SERVICE_BPP_CONTROLLER_H
#define BT_SERVICE_BPP_CONTROLLER_H

//  INCLUDES
#include "BTServiceStarter.h"
#include "BTServiceParameterList.h"
#include "BTServiceClient.h"
#include "BTSBPPObjectServer.h"
#include "BTSBPPServerWait.h"
#include "BTSController.h"




// CLASS DECLARATION

/**
*  A class controlling printing service using Basic Printing Profile.
*/
NONSHARABLE_CLASS (CBTSBPPController) : public CBTSController,                           
                          public MBTServiceClientObserver,
                          public MBTSBPPObjectServerObserver,
                          public MBTSBPPServerWaitObserver
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTSBPPController* NewL( MBTServiceObserver* aObserver,                                                                              
                                        const TUint aRemotePort,
                                        const TBTDevAddr& aRemoteDevice,
                                        const CBTServiceParameterList* aList,
                                        CBTEngDiscovery* aBTEngDiscoveryPtr );
        
        /**
        * Destructor.
        */
        virtual ~CBTSBPPController();


    private: // Functions from base classes

        /**
        * From MBTServiceClientObserver A Connect operation has been completed. 
        * @param aStatus The status of the operation.
        * @return None.
        */
        void ConnectCompleted( TInt aStatus );

        /**
        * From MBTServiceClientObserver A Put operation has been completed. 
        * @param aStatus The status of the operation.
        * @param aPutResponse The response object.
        * @return None.
        */
        void PutCompleted( TInt aStatus,const  CObexHeaderSet* aPutResponse );

        /**
        * From MBTServiceClientObserver A Get operation has been completed. 
        * @param aStatus The status of the operation.
        * @param aGetResponse The response object.
        * @return None.
        */
        void GetCompleted( TInt aStatus, CObexBufObject* aGetResponse );

        /**
        * From MBTServiceClientObserver The client connection has been closed.
        * @return None.
        */
        void ClientConnectionClosed();
        
        /**
        * From MBTServiceClientObserver A filehandle can be reseted.
        * @return None.
        */
        void ResetFileHandleL();
        /**
        * From MBTPSServerObserver An error in server has occurred.
        * @param aError The error.
        * @return None.
        */
        void ServerError( TInt aError );

        /**
        * From MBTPSServerObserver The server connection has been established.
        * @param None.
        * @return None.
        */
        void ServerConnectionEstablished();

        /**
        * From MBTPSServerObserver The server connection has been closed.
        * @param None.
        * @return None.
        */
        void ServerConnectionClosed();

        /**
        * From MBTPSServerWaitObserver The server wait has completed.
        * @param None.
        * @return None.
        */
        void WaitComplete();
        
        /**
        * From MBTServiceClientObserver A Connect operation is timed out.
        * @return None.
        */
        void ConnectTimedOut();

    private:

        /**
        * C++ default constructor.
        */
        CBTSBPPController( MBTServiceObserver* aObserver,
                           const CBTServiceParameterList* aList );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TBTDevAddr& aRemoteDevice,
                         const TUint aRemotePort,
                         CBTEngDiscovery* aBTEngDiscoveryPtr );

        /**
        * Selects the next document to send and issues a send request 
        * to Obex client.
        * @return None.
        */
        void SelectAndSendL();
    
    private:    // Data definitions

        enum TBTSBPPObjectServerState
            {
            EBTSBPPSrvIdle,
            EBTSBPPSrvServing,
            EBTSBPPSrvDone
            };

    private:    // Data
        
        // Owned
        //
        CBTSBPPObjectServer*        iServer;
        CBTSBPPServerWait*          iServerWait;

        TInt                        iSendIndex;
        TUint                       iClientChannel;
        TBTSBPPObjectServerState    iServerState;
        TBool                       iClientDone;

        // Not owned
        //
        MBTServiceObserver*         iObserverPtr;
        const CBTServiceParameterList* iListPtr;
        
    };

#endif      // BT_SERVICE_BPP_CONTROLLER_H
            
// End of File
