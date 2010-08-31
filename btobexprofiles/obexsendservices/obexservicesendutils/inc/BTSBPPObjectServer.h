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
* Description:  Server part of basic printing profile
*
*/



#ifndef BTSBPP_OBJECT_SERVER_H
#define BTSBPP_OBJECT_SERVER_H

//  INCLUDES
//#include "BTSBPPPasskeyRequest.h"

#include <obex.h>       // Obex
#include <badesca.h>    // CDesCArray
#include <btengdiscovery.h>
#include "BTServiceStarter.h"

// FORWARD DECLARATIONS
class CBTSBPPObjectRequest;

// CLASS DECLARATION

/**
*  An interface used to inform about CBTSBPPObjectServer events.
*/
class MBTSBPPObjectServerObserver
    {
    public:

        /**
        * Informs the observer that an error has been occurred.
        * @param aError The error.
        * @return None.
        */
        virtual void ServerError( TInt aError ) = 0;

        /**
        * Informs the observer that the server connection has been established.
        * @param None.
        * @return None.
        */
        virtual void ServerConnectionEstablished() = 0;

        /**
        * Informs the observer that the server connection has been closed.
        * @param None.
        * @return None.
        */
        virtual void ServerConnectionClosed() = 0;
    };

// CLASS DECLARATION

/**
*  A class managing the Obex Server.
*/
NONSHARABLE_CLASS (CBTSBPPObjectServer) : public CBase, 
                            public MObexServerNotify,                    
                            public MObexAuthChallengeHandler
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTSBPPObjectServer* NewL( MBTSBPPObjectServerObserver* aObserver,
                                          CBTEngDiscovery* aBTEngDiscoveryPtr,
                                          const TBTDevAddr& aExpectedClient );
        
        /**
        * Destructor.
        */
        virtual ~CBTSBPPObjectServer();

    public: // New functions

        /**
        * Gives the server a list of referenced objects that can be requested.
        * by the printer. This method should be called always before sending
        * any documents. 
        * @param aRefObjectList A pointer to the list.
        * @return None.
        */
        void SetReferencedObjectList( const CDesCArray* aRefObjectList );

        /**
        * Tells whether the current referenced object list has any items.
        * @param aRefObjectList A pointer to the list.
        * @return None.
        */
        TBool HasReferencedObjects();

    private: // Functions from base classes

        /**
        * From MObexAuthChallengeHandler Get's called when Obex Password is 
        * requested.
        * @param aRealm The realm/challenge specified by the unit forcing 
        *               the authentication.
        * @return None.
        */
        void GetUserPasswordL( const TDesC& aRealm );

    private: // Functions from MObexServerNotify

        /**
        * Informs about an error in Obex connection.
        * @param aError The occurred error.
        * @return None.
        */
        void ErrorIndication( TInt aError );

        /**
        * Informs that Obex transport layer is up.
        * @return None.
        */
        void TransportUpIndication();

        /**
        * Informs that Obex transport layer is down.
        * @return None.
        */
        void TransportDownIndication();

        /**
        * Informs about Obex connect request.
        * @param aRemoteInfo Information about the remote party.
        * @param aInfo Information about the connection request.
        * @return Symbian OS error code.
        */
        TInt ObexConnectIndication( const TObexConnectInfo& aRemoteInfo, 
                                    const TDesC8& aInfo );

        /**
        * Informs about Obex disconnect request.
        * @param aInfo Information about the disconnect request.
        * @return None.
        */
        void ObexDisconnectIndication( const TDesC8& aInfo );

        /**
        * Informs about Obex put request.
        * @return The requested object or NULL;
        */
        CObexBufObject* PutRequestIndication();

        /**
        * Informs that part of the put object is sent.
        * @return Symbian OS error code.
        */
        TInt PutPacketIndication();

        /**
        * Informs that the put request is completed.
        * @return Symbian OS error code.
        */
        TInt PutCompleteIndication();

        /**
        * Informs about Obex put request.
        * @param aRequiredObject The Get request object.
        * @return The response object or NULL;
        */
        CObexBufObject* GetRequestIndication( 
        CObexBaseObject* aRequiredObject );

        /**
        * Informs that part of the get response object is sent.
        * @return Symbian OS error code.
        */
        TInt GetPacketIndication();

        /**
        * Informs that the get request is completed.
        * @return Symbian OS error code.
        */
        TInt GetCompleteIndication();

        /**
        * Informs about Obex Set Path request.
        * @param aPathInfo Information about the path.
        * @param aInfo Information about the Set Path request.
        * @return Symbian OS error code.
        */
        TInt SetPathIndication( const CObex::TSetPathInfo& aPathInfo, 
                                const TDesC8& aInfo );

        /**
        * Informs about Obex Abort operation.
        * @return None.
        */
        void AbortIndication();

    private:

        /**
        * C++ default constructor.
        */
        CBTSBPPObjectServer( MBTSBPPObjectServerObserver* aObserver,
                             CBTEngDiscovery* aBTEngDiscoveryPtr,
                             const TBTDevAddr& aExpectedClient );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

        TInt                            iObjectChannel;
        TSdpServRecordHandle            iSDPHandle;

        CObexServer*                    iServer;
        CBTSBPPObjectRequest*           iObjectRequest;
        //CBTSBPPPasskeyRequest*        iPasskeyRequest;        

        // Not owned
        //
        const CDesCArray*               iObjectListPtr;
        const TBTDevAddr*               iExpectedClientPtr;
        CBTEngDiscovery*                iBTEngDiscoveryPtr;
        MBTSBPPObjectServerObserver*    iObserverPtr;
    };

#endif      // BTSBPP_OBJECT_SERVER_H
            
// End of File
