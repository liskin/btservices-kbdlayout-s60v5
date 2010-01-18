/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*	  BTSap session definition.
*
*/


#ifndef BT_SAP_SESSION_H
#define BT_SAP_SESSION_H

//	INCLUDES
#include <e32svr.h>

// Opcodes used in message passing between client and server
enum TBTSapRequest
	{
	EBTSapManageService,
	EBTSapAcceptSapConnection,
	EBTSapRejectSapConnection,
	EBTSapDisconnectSapConnection,
	EBTSapIsConnected,
	EBTSapGetRemoteBTAddress
	};

// FORWARD DECLARATIONS
class CBTSapServer;

// CLASS DECLARATION

/**
*  CBTSapSession class represent session on server side
*
*/
class CBTSapSession : public CSession2
	{
	public:	 // Constructors and destructor

		/**
		* Constructor.
		*/
		static CBTSapSession* NewL();

        // 2nd phase construct for sessions - called byb the CServer2 framework
        void CreateL();

		/**
		* Destructor.
		*/
		~CBTSapSession();

	public: // Functions from base classes

		/**
		* From CSession ServiceL
		* Client-server framework calls this function
		* to handle messages from client.
		* @param	aMessage received message class
		* @return	None
		*/
		void ServiceL(const RMessage2 &aMessage);

		/**
		* From CSession DispatchMessageL
		* Dispatchs and calls appropriate methods to
		* handle messages.
		* @param	aMessage received message class
		* @return	None
		*/
		void DispatchMessageL(const RMessage2 &aMessage);

		/**
		* From CSession Server
		* Returns reference to server.
		* @param	None
		* @return	Reference to server
		*/
		CBTSapServer& Server();

	private:

		/**
		* C++ default constructor.
		*/
		CBTSapSession();


	};

#endif		// BT_SAP_SESSION_H

// End of File
