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
*     This class implements a handler, which sets the physical link to
*     sniff-mode when the SAP link is inactive for some time
*
*
*/


#ifndef BT_SAP_SNIFF_HANDLER_H
#define BT_SAP_SNIFF_HANDLER_H

//	INCLUDES
#include <e32base.h>
#include <bt_sock.h>

// CLASS DECLARATION

NONSHARABLE_CLASS(CBTSapSniffHandler) : public CTimer
	{
public:
	
	// Constructor
	static CBTSapSniffHandler* NewL(RSocket& aSocket, RSocketServ& aSocketServer);
	
	// Destructor
	~CBTSapSniffHandler();
		
private:	 // From CActive
   
	/**
	* RunL is called by framework after request is being completed.
	* @param nones
	* @return none 
	*/
	void RunL();

public:	   
	
    void Enable();
    void Disable();
    void Pause();

private:

	// Default constructor
    CBTSapSniffHandler(RSocket& aSocket, RSocketServ& aSocketServer);

private:

	RSocket&               iSocket;            // Data socket 
	RSocketServ&           iSocketServer;      // Socket server
    TBool                  iIsLinkAdapterOpen; // Keep physical link adapter's state
    RBTPhysicalLinkAdapter iBTLinkAdapter;     // Sniff mode controller

	};

#endif		// BT_SAP_SNIFF_HANDLER_H	 
			
// End of File
