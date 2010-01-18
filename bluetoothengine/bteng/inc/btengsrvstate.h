/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Engine server state machine class declaration.
*
*/



#ifndef BTENGSRVSTATE_H
#define BTENGSRVSTATE_H


//#include <?include_file>

#include "btengactive.h"
#include "btserversdkcrkeys.h"

class MBTEngActiveObserver;
class CBTEngServer;

/**  ?description */
//const ?type ?constant_var = ?constant;


/**
 *  Class CBTEngSrvState
 *
 *  This class implements the state machine for BTEng server.
 *  The state machine is used for Bluetooth power on and power off operations.
 *  A state machine is needed because the required operations for loading and 
 *  initalizing all required services is time-consuming and would be too long 
 *  to handle in one operation. It also includes asynchronous operations.
 *  It is implemented in a separate class to make it easier extensible as well 
 *  as to give a better overview of the state transitions and operations.
 *
 *  The state machine is initialized in state Init, and ends in state Idle. Any
 *  state in between are temporary states (i.e. not accepting states).
 *
 *  @lib bteng.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngSrvState ) : public CBase
    {

public:

    /**  Enumeration of BTEng server states */
    enum TBTEngSrvState
        {
        EInit,
        EInitBTStack,
        ELoadDIService,
        ELoadPlugins,
        ELoadNextPlugin,
        EIdle,
        EWaitingForPowerOff,
        EStopBTStack,
        EUnloadDIService,
        EUnloadPlugins,
        EDisconnectPlugins,
        };

    /**  Enumeration of BTEng server states */
    enum TBTEngSrvOperation
        {
        ESrvOpIdle,
        EPowerOff,
        EPowerOn
        };

    /**  ?description */
//    typedef ?declaration

    /**
     * Two-phase constructor
     */
    static CBTEngSrvState* NewL( CBTEngServer* aServer );

    /**
     * Destructor
     */
    virtual ~CBTEngSrvState();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    CBTEngSrvState::TBTEngSrvOperation CurrentOperation();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    void StartStateMachineL( TBool aState );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    void ChangeState();    

private:

    /**
     * C++ default constructor
     */
    CBTEngSrvState( CBTEngServer* aServer );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();
    
    static TInt ChangeStateCb(TAny* aThis);
    
    void RequestCompletedL();
    
    void HandleError(TInt aError);

private: // data

    /**
     * Our current state.
     */
    TInt iState;

    /**
     * The current command (power-on or power-off Bluetooth).
     */
    TBTEngSrvOperation iOperation;

    /**
     * The active object used for state transitions.
     * Own.
     */
    CAsyncCallBack* iAsyncCallback;

    /**
     * The server instance.
     * Not own.
     */
    CBTEngServer* iServer;

    };


#endif // BTENGSRVSTATE
