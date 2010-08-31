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
* Description:  Image push controller
*
*/



#ifndef BT_BIP_CONTROLLER_H
#define BT_BIP_CONTROLLER_H

//  INCLUDES
#include "BTServiceClient.h"
#include "BTServiceStarter.h"
#include "BTSController.h"

// CLASS DECLARATION

/**
*  Control class for the Image sending
*/
NONSHARABLE_CLASS (CBTSBIPController) : public CBTSController, 
                                        public MBTServiceClientObserver                                    

    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTSBIPController* NewL( MBTServiceObserver* aObserver,
            const TUint aRemotePort,
            const TBTDevAddr& aRemoteDevice,
            CBTServiceParameterList* aList  );
        
        /**
        * Destructor.
        */
        virtual ~CBTSBIPController();
        
        void SendUnSupportedFiles();

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
        CBTSBIPController( MBTServiceObserver* aObserver,
                           CBTServiceParameterList* aList );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TUint aRemotePort,
                         const TBTDevAddr& aRemoteDeviceAddr );
    
    private:

        /**
        * Send image 
        * @param None.
        * @return None.
        */
        void SendL();
        
        /**
        * Send thumbnail image
        * @param aPutResponse Response packet containing the image handle
        * @return None.
        */
        void SendThumbnailL( const CObexHeaderSet* aPutResponse );

        /**
        * Get capabilities object from remote device 
        * @param None.
        * @return None.
        */
        void GetL();

        /**
        * Create temp file
        * @param aFileName Name of the file that was created
        * @return None.
        */
        void CreateTempFileL( TFileName& aFileName );
        
        /**
        * Generate a temp file name
        * @param aFileName File name that was created.
        * @return None.
        */
        void GenerateTempFileNameL( TFileName& aFileName );
        
        /**
        * Delete a temp file 
        * @param aFileName Name of the file.
        * @return None.
        */
        void DeleteTempFile( TFileName& aFileName );

        /**
        * Create image descriptor 
        * @param None.
        * @return HBufC8*.
        */
        HBufC8* CreateImageDescriptorL();

        /**
        * Handle GetCompleteIndication
        * @param aGetResponse Get response packet from remote device
        * @return None.
        */
        void HandleGetCompleteIndicationL( CObexBufObject* aGetResponse );
    
    private:    // Data
        
        TInt                        iFileIndex;
        TFileName                   iThumbnailFileName;
        TFileName                   iTempFileName;
		

        // Not owned
        //        
        CBTServiceParameterList*    iListPtr;
        MBTServiceObserver*         iObserverPtr;
        
    };

#endif      // BT_BIP_CONTROLLER_H
            
// End of File
