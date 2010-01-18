/*
* Copyright (c) 2004-2008 Nokia Corporation and/or its subsidiary(-ies).
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
*	  BTSap plugin header definition
*
*/
 

#ifndef BT_SAP_PLUGIN_H
#define BT_SAP_PLUGIN_H

//	INCLUDES
#include <btengplugin.h>

enum TBTSapRejectReason
    {
    ERejectGeneralError,
    ERejectReserved
    };

enum TBTSapDisconnectType
    {
    EDisconnectGraceful,
    EDisconnectImmediate
    };

// FORWARD DECLARATIONS
class TBTDevAddr;
class CBTSapServerState;

/*
* 
*/
class MSapAsyncObserver
    {
    public:
    enum TOperation
        {
        EConnect,
        ECancelConnect,
        EDisconnect
        };
        
    virtual void OperationCompletedL(TOperation aOperation, TInt aError) = 0;
    };
       
/*
* A class that proveides a utility service for calling
* CSapServerState methods asychronously
*/ 
class CBTSapAsyncHelper : public CActive
    {    
public:
    static CBTSapAsyncHelper* NewL(CBTSapServerState* aServerState, MSapAsyncObserver* aObserver);
    virtual ~CBTSapAsyncHelper();
    
    void AsyncConnect(const TBTDevAddr& aAddr);
    void AsyncCancelConnect(const TBTDevAddr& aAddr);
    void AsyncDisconnect(const TBTDevAddr& aAddr, TBTSapDisconnectType aDiscType);
    
private:
    void ConstructL(CBTSapServerState* aServerState, MSapAsyncObserver* aObserver);
    CBTSapAsyncHelper();
    
    // From CActive
    virtual void DoCancel();
    virtual void RunL();
    
private:
    MSapAsyncObserver::TOperation iOperation;
    CBTSapServerState* iServerState;
    MSapAsyncObserver* iObserver;
    TBTSapDisconnectType iDiscType;
    };
    

/**
* CBTSapPlugin core class.
* Implements the BT Engine ECom interface
*/
class CBTSapPlugin : public CBTEngPlugin, MSapAsyncObserver
	{
public:	 // Constructors and destructor

	/**
	* Two-phased constructor.
	*/
	static CBTSapPlugin* NewL();

	/**
	* Destructor.
	*/
    virtual ~CBTSapPlugin();

public:
    // From CBTEngPlugin (ECom)

    virtual void GetSupportedProfiles( RProfileArray& aProfiles );

    virtual void SetObserver( MBTEngPluginObserver* aObserver );

    virtual TBool IsProfileSupported( const TBTProfile aProfile ) const;

    virtual TInt Connect( const TBTDevAddr& aAddr );

    virtual void CancelConnect( const TBTDevAddr& aAddr );

    virtual TInt Disconnect( const TBTDevAddr& aAddr, TBTDisconnectType aDiscType );
    
    virtual void GetConnections( RBTDevAddrArray& aAddrArray, TBTProfile aConnectedProfile );
                               
    virtual TBTEngConnectionStatus IsConnected( const TBTDevAddr& aAddr );
    
    // From MSapAsyncObserver
    
    void OperationCompletedL(MSapAsyncObserver::TOperation aOperation, TInt aError);

public:

	TInt AcceptSapConnection();

	TInt RejectSapConnection(TBTSapRejectReason aReason = ERejectGeneralError);

	TInt DisconnectSapConnection(TBTSapDisconnectType aType = EDisconnectGraceful);

	TBool IsSapConnected();

	TInt GetRemoteBTAddress(TBTDevAddr& aBTDevAddr);
	
    void ConnectComplete();

private:

	/**
	* Default constructor
	*/
	CBTSapPlugin();

	/**
	* Two-Phase constructor
	*/
	void ConstructL();

	void StartBTSapServiceL();

	void StopBTSapServiceL();

private:	// Data

	enum TBTSapServiceState
		{
		EServiceOff,
		EServiceOn,
		EServiceShuttingDown
		};

	CBTSapServerState* iBTSapServerState;
	TBTSapServiceState iServiceState;
	MBTEngPluginObserver* iObserver;
	CBTSapAsyncHelper* iAsyncHelper;
	TBTDevAddr iAddr;
	};
	
#endif		// BT_SAP_PLUGIN_H
// End of File
