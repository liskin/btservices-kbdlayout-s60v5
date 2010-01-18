/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*     BTSap Plugin header definition
*
*
*/


#ifndef BT_SAP_SERVER_STATE_H
#define BT_SAP_SERVER_STATE_H

//  INCLUDES
#include <e32base.h>
#include <e32property.h>

#ifdef _DUMMY_CUSTOM_API
#include <DummyCustomAPI.h>
#else
#include <rmmcustomapi.h>
#endif

#include "BTSapPlugin.h"
#include "BTSapMessage.h"

enum TBTSapServerState
    {
    EStateInit,
    EStateNotConnected,
    EStateConnect,
    EStateDisconnect,
    EStateIdle,
    EStateAPDU,
    EStateATR,
    EStatePowerOff,
    EStatePowerOn,
    EStateReset,
    EStateCardReaderStatus
    };

enum TSapConnectionCompleteReason
    {
    EConnectionError,
    EUserAccepted,
    EUserRejected
    };

enum TSapConnectionState
    {
    ESapNotConnected,
    ESapConnecting,
    ESapConnected
    };

// FORWARD DECLARATIONS
class CBTSapSocketHandler;
class CBTSapRequestHandler;
class CBTSapSimCardStatusNotifier;
class RTelServer;
class RMobilePhone;
class MSapStatusObserver;

/*
* An abstarct interface that defines a callback method
* for the SAP connection state observer. SapStatusChangedL
* is called by CBTSapStatusObserver when the SAP connection
* state has changed. aStatus represents the new status value.
*/
class MSapStatusObserver
    {
    public:
    virtual void SapStatusChangedL(TInt aStatus) = 0;
    };
    
/*
* An observer that monitors KPSUidBluetoothSapConnectionState
* P&S key. The clients can call SubscribeSapStatusL in order to 
* get a chenged new SAP connection status returned via 
* MSapStatusObserver::SapStatusChangedL.
*/ 
class CBTSapStatusObserver : public CActive
    {
public:
    /**
    * Two-phased constructor.
    */
    static CBTSapStatusObserver* NewL();
    /**
    * Destructor.
    */
    virtual ~CBTSapStatusObserver();
    
    /**
    * Starts listening KPSUidBluetoothSapConnectionState P&S key.
    * When the key value is changed, the new state is indicated
    * through the observer interface (MSapStatusObserver).
    */
    void SubscribeSapStatusL(MSapStatusObserver* aObserver);
      
private:
    /**
    * Two-Phase constructor
    */
    void ConstructL();
    
    /**
    * Default constructor
    */
    CBTSapStatusObserver();
    
    // From CActive
    virtual void DoCancel();
    virtual void RunL();
      
private:
    MSapStatusObserver* iObserver;
    RProperty iProperty;
    };


/**
*  CBTSapServerState core class
*
*/
class CBTSapServerState : public CActive, MSapStatusObserver
    {
private:
    class TState
        {
    public:
        TState(CBTSapServerState& aServerState);
        virtual void Enter(TRequestStatus& aStatus) = 0;
        virtual TBTSapServerState Complete(TInt aReason = KErrNone) = 0;
        virtual void Cancel() = 0;

        virtual TInt DisconnectSapConnection(TBTSapDisconnectType aType);
        virtual TInt AcceptSapConnection();
        virtual TInt RejectSapConnection(TBTSapRejectReason aReason);
        virtual void SimCardStatusChanged(TCardStatus aCardStatus);
        virtual TInt ChangeState(TBTSapServerState& aNextState);

        void NotifySapState(TSapConnectionState aState);
        void StartBTNotifier(TInt type);
        TBool IsSimPresent();

    protected:
        TInt iSimStatus;
        CBTSapServerState& iServerState;
        TBTSapMessage& iResponseMessage; //
        TRequestStatus* iStatus; // TState can't access parent's member directly
        };

    class TStateInit : public TState
        {
    public:
        TStateInit(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();
        };

    class TStateNotConnected : public TState
        {
    public:
        TStateNotConnected(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt ChangeState(TBTSapServerState& aNextState);
        };

    class TStateIdle : public TState
        {
    public:
        TStateIdle(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt DisconnectSapConnection(TBTSapDisconnectType aType);
        void SimCardStatusChanged(TCardStatus aCardStatus);
        TInt ChangeState(TBTSapServerState& aNextState);
        };

    class TStateConnect : public TStateIdle
        {
    public:
        TStateConnect(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt AcceptSapConnection();
        TInt RejectSapConnection(TBTSapRejectReason aReason);
        TInt ChangeState(TBTSapServerState& aNextState);
        void SimCardStatusChanged(TCardStatus aCardStatus);

    private:
        void CheckMaxMsgSize(TConnectionStatus& aMsgSizeStatus);
        TBool IsCallOngoing();

    private:
        TBool iConnectRequestOK;
        TCardStatus iCardStatus;
        RProperty iProperty;
        };

    class TStateDisconnect : public TStateIdle
        {
    public:
        TStateDisconnect(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt ChangeState(TBTSapServerState& aNextState);
        };

    class TStateAPDU : public TStateIdle
        {
    public:
        TStateAPDU(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt ChangeState(TBTSapServerState& aNextState);

    private:
        TBuf8<KMaxMsgSize> iApduCmd;
        TBuf8<KMaxMsgSize> iApduResp;
        RMmCustomAPI::TApduParameters iApduParameters;
        };

    class TStateATR : public TStateIdle
        {
    public:
        TStateATR(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt ChangeState(TBTSapServerState& aNextState);

    private:
        TBuf8<KMaxMsgSize> iATR;
        };

    class TStatePowerOff : public TStateIdle
        {
    public:
        TStatePowerOff(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt ChangeState(TBTSapServerState& aNextState);
        };

    class TStatePowerOn : public TStateIdle
        {
    public:
        TStatePowerOn(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt ChangeState(TBTSapServerState& aNextState);
        void SimCardStatusChanged(TCardStatus aCardStatus);

    private:
        TResultCode iResultCode;
        TCardStatus iCardStatus;
        };

    class TStateReset : public TStateIdle
        {
    public:
        TStateReset(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt ChangeState(TBTSapServerState& aNextState);
        void SimCardStatusChanged(TCardStatus aCardStatus);

    private:
        TResultCode iResultCode;
        TCardStatus iCardStatus;
        };

    class TStateCardReaderStatus : public TStateIdle
        {
    public:
        TStateCardReaderStatus(CBTSapServerState& aServerState);
        void Enter(TRequestStatus& aStatus);
        TBTSapServerState Complete(TInt aReason);
        void Cancel();

        TInt ChangeState(TBTSapServerState& aNextState);

    private:
        RMmCustomAPI::TSimCardReaderStatus iCardReaderStatus;
        };

public:
    /**
    * Two-phased constructor.
    */
    static CBTSapServerState* NewL(CBTSapPlugin& aBTSapPlugin);
    /**
    * Destructor.
    */
    virtual ~CBTSapServerState();

public:

    void StartL();

    void ChangeState(const TBTSapServerState aNextState);

    TBTSapServerState CurrentState();

    TInt DisconnectSapConnection(TBTSapDisconnectType aType);

    TInt AcceptSapConnection();

    TInt RejectSapConnection(TBTSapRejectReason aReason);

    TBool IsSapConnected();

    TInt GetRemoteBTAddress(TBTDevAddr& aBTDevAddr);
    
    TInt CreateStatesL();
    
    TInt ReleaseStatesL();
    
    TInt DisconnectCompleteL(TInt aErr);

public:

    TInt HandleStateChangeRequest(TBTSapServerState& aNextState);

    void SendErrorResponse();

    void SimCardStatusChanged(TCardStatus aCardStatus);

    CBTSapSocketHandler& BTSapSocketHandler();

    CBTSapSimCardStatusNotifier& BTSapSimCardStatusNotifier();

    RMmCustomAPI& SubscriptionModule();

    TBTSapMessage& BTSapRequestMessage();

    TBTSapMessage& BTSapResponseMessage();

    // From MSapStatusObserver
    void SapStatusChangedL(TInt aStatus);

private:

    void OpenSubscriptionModuleL();

private:     // From CActive

    /**
    * DoCancel() has to be implemented by all the classes deriving CActive.
    * Request cancellation routine.
    * @param none
    * @return none 
    */
    virtual void DoCancel();

    /**
    * RunL is called by framework after request is being completed.
    * @param nones
    * @return none 
    */
    virtual void RunL();

private:

    /**
    * Default constructor
    */
    CBTSapServerState(CBTSapPlugin& aBTSapPlugin);

    /**
    * Two-Phase constructor
    */
    void ConstructL();

private:    // Data

    CBTSapPlugin& iBTSapPlugin;
    CBTSapSocketHandler* iSocketHandler;
    CBTSapRequestHandler* iRequestHandler;
    CBTSapSimCardStatusNotifier* iSimCardStatusNotifier;

    TBTSapServerState iCurrentState;
    RPointerArray <TState> iStateArray;

    RTelServer iTelServer;
    RMobilePhone iPhone;
    RMmCustomAPI iSubscriptionModule;

    TBTSapMessage iResponseMessage;
    TBTSapMessage iRequestMessage;
    CBTSapStatusObserver* iStatusObserver;
    TBool iStatesCreated;
    };

#endif      // BT_SAP_SERVER_STATE_H

// End of File
