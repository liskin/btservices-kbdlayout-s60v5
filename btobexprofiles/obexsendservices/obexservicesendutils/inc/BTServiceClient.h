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
* Description:  Obex client
*
*/



#ifndef BT_SERVICE_CLIENT_H
#define BT_SERVICE_CLIENT_H

//  INCLUDES
#include "BTSUPassKeyRequest.h"

#include <obex.h>
#include <obexheaders.h>

 
// FORWARD DECLARATION
class CBTSUPasskeyRequest;
class CBTConnectionTimer;

// CLASS DECLARATION

/**
*  An interface used to inform about BT service client events.
*/
class MBTServiceClientObserver
    {
    public:

        /**
        * Informs the observer that a Connect operation has been completed. 
        * @param aStatus The status of the operation.
        * @return None.
        */
        virtual void ConnectCompleted( TInt aStatus ) = 0;

        /**
        * Informs the observer that a Put operation has been completed. 
        * @param aStatus The status of the operation.
        * @param aPutResponse The response object.
        * @return None.
        */
        virtual void PutCompleted( TInt aStatus,const  CObexHeaderSet* aPutResponse ) = 0;

        /**
        * Informs the observer that a Get operation has been completed. 
        * @param aStatus The status of the operation.
        * @param aGetResponse The response object.
        * @return None.
        */
        virtual void GetCompleted( TInt aStatus, CObexBufObject* aGetResponse ) = 0;

        /**
        * Informs the observer that the client connection has been closed.
        * @return None.
        */
        virtual void ClientConnectionClosed() = 0;
        
        /**
        * Informs the observer that the client connect is timed out.
        * @return None.
        */
        virtual void ConnectTimedOut() = 0;
        
      
    };

/**
*  An interface used to inform about BT Connection timeout
*/    
class MBTConTimeObserver    
    {
    public:
        /**
        * Informs the observer that the client connec is timeouted
        * @return None.
        */
        virtual void ConnectionTimedOut()=0;
    };


// CLASS DECLARATION

/**
*  An interface used to ask client's progress status.
*/
class MBTServiceProgressGetter
    {
    public:

        /**
        * Returns the progess status of the service client.
        * @return The number of bytes sent.
        */
        virtual TInt GetProgressStatus() = 0;
    };

// CLASS DECLARATION

/**
*  An active object managing the Obex client.
*/
NONSHARABLE_CLASS (CBTServiceClient) : public CActive,
                         public MBTServiceProgressGetter,
                         public MObexAuthChallengeHandler,
                         public MBTConTimeObserver
                         
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aObserver A pointer to the client observer.
        * @param aRemoteDevice The address of the remote device.
        * @param aRemotePort The port of the remote device.
        * @param aHeaderList The headers to be associated with connect object.
        * @return None.
        */
        static CBTServiceClient* NewL( MBTServiceClientObserver* aObserver,
                                       const TBTDevAddr& aRemoteDevice,
                                       const TUint aRemotePort,
                                       RArray<CObexHeader*> aHeaderList );
        
        /**
        * Destructor.
        */
        virtual ~CBTServiceClient();

    public: // New functions
        
        /**
        * Issues an Obex Get-request.
        * @param aHeaderList The headers to be associated with the object.
        * @param aFileName A filename of the Get Object.
        * @return None.
        */
        void GetObjectL( RArray<CObexHeader*>& aHeaderList, 
                         const TDesC& aFileName = KNullDesC );

        /**
        * Issues an Obex Put-request.        
        * @param aHeaderList The headers to be associated with the object.
        * @param aFileName A filename of the Put Object.
        * @return None.
        */
        void PutObjectL( RArray<CObexHeader*>& aHeaderList, 
                         const TDesC& aFileName  );
                         
        /**
        * Issues an Obex Put-request.        
        * @param aHeaderList The headers to be associated with the object.
        * @param aFile A filehandle of the Put Object.
        * @return None.
        */
        void PutObjectL( RArray<CObexHeader*>& aHeaderList, 
                         RFile&  );

        /**
        * Closes Obex Client connection.
        * @param None.
        * @return None.
        */
        void CloseClientConnection();
        
        /**
        * Send abort command to remote device
        * @param None.
        * @return None.
        */
        void Abort();        

    private: // Functions from base classes

        /**
        * From MBTServiceProgressGetter Returns the progess status.
        * @return The number of bytes sent.
        */
        TInt GetProgressStatus();

        
    private: // Functions from base classes

        /**
        * From MObexAuthChallengeHandler The Obex Passkey is requested.
        * @param aRealm The realm/challenge specified by the unit forcing 
        * the authentication.
        * @return None.
        */
        void GetUserPasswordL( const TDesC& aRealm );

        /**
        * From CActive Get's called when a request is cancelled.
        * @param None.
        * @return None.
        */
        void DoCancel();

        /**
        * From CActive Get's called when a request is completed.
        * @param None.
        * @return None.
        */
	    void RunL();
	    
	    /**
        * From MBTConTimeObserver  Get's called if bt connection is timed out.
        * @param None.
        * @return None.
        */
	    void ConnectionTimedOut();

    private:    // Data definitions

        enum TBTServiceClientState
            {
            EBTSCliIdle,
            EBTSCliConnecting,
            EBTSCliPutting,
            EBTSCliGetting,
            EBTSCliDisconnecting
            };

    private:

        /**
        * C++ default constructor.
        */
        CBTServiceClient( MBTServiceClientObserver* aObserver );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TBTDevAddr& aRemoteDevice,
                         const TUint aRemotePort,
                         RArray<CObexHeader*> aHeaderList );

    private:    // Data

        TBTServiceClientState       iClientState;

        CObexClient*                iClient;
        CBufFlat*                   iObjectBuffer;
        CObexBufObject*             iGetObject;
        CObexFileObject*            iPutObject;        
        CObexNullObject*            iConnectObject;
        TInt                        iTotalBytesSent;
        CBTSUPasskeyRequest*        iPasskeyRequest;
        CBufFlat                    *iBuffer;
        CObexBufObject*             iPutBufObject;
        CBTConnectionTimer*         iConnectionTimer;
        // Not owned
        //
        MBTServiceClientObserver*   iObserver;
    };

#endif      // BT_SERVICE_CLIENT_H
            
// End of File
