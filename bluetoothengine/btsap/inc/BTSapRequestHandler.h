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
*     This class handles SAP request message
*
*
*/

#ifndef BT_SAP_REQUEST_HANDLER_H
#define BT_SAP_REQUEST_HANDLER_H

//  INCLUDES
#include "BTSapServerState.h"

const TBTSapServerState KStateByRequest[] = 
    {
    EStateConnect, 
    EStateIdle, 
    EStateDisconnect,
    EStateIdle, 
    EStateIdle, 
    EStateAPDU,
    EStateIdle,
    EStateATR,
    EStateIdle,
    EStatePowerOff,
    EStateIdle,
    EStatePowerOn,
    EStateIdle,
    EStateReset,
    EStateIdle,
    EStateCardReaderStatus
    };

class CBTSapRequestHandler : public CActive  
{
private:
    class CSegmentTimer : public CTimer
	    {
    public:
	    static CSegmentTimer* NewL(CBTSapRequestHandler* aRequestHandler);

    private:
	    CSegmentTimer(CBTSapRequestHandler* aRequestHandler);

	    void RunL();

    private:
	    CBTSapRequestHandler* iRequestHandler;
	    };

public:
    // Constructors
    static CBTSapRequestHandler* NewL(CBTSapServerState& aServerState);

    // Destructor
	~CBTSapRequestHandler();

private:     // From CActive
   
    /**
    * DoCancel() has to be implemented by all the classes deriving CActive.
    * Request cancellation routine.
    * @param none
    * @return none 
    */
    void DoCancel();
    
    /**
    * RunL is called by framework after request is being completed.
    * @param nones
    * @return none 
    */
	void RunL();

public:

    void HandleSapData(const TDes8& aData);

    void HandleSegmentTimeout();

private:

    // Two-phase constructor
    void ConstructL();

    // Default constructor
    CBTSapRequestHandler(CBTSapServerState& aServerState);

    void HandleSapMessage(const TValidationResult aResult);

private:

    CBTSapServerState& iServerState;
    TBTSapMessage& iRequestMessage;
    TBTSapMessage iRawMessage;
    CSegmentTimer* iSegmentTimer;
};

#endif // BT_SAP_REQUEST_HANDLER_H
