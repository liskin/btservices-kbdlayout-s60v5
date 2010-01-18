// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "avrcpincomingcommandhandler.h"
#include "avrcpoutgoingcommandhandler.h"
#include "avrcpremotedevice.h"

#include "avrcplog.h"

/** Factory function.

@param aAddr The remote device this manages transactions with.
@param aRouter	A CRcpRouter to use for communication with remote devices.
@param aBearer	The CRemConBearerAvrcp this is to handle commands for.
@param aObserver The observer of the bearer. Used to acquire converters.
@param aTimer	CDeltaTimer to use for queuing timed events.
@return A fully constructed CRcpRemoteDevice.
@leave System wide error codes.
*/
CRcpRemoteDevice* CRcpRemoteDevice::NewL(const TBTDevAddr& aAddr, 
	CRcpRouter& aRouter, 
	CRemConBearerAvrcp& aBearer,
	MRemConBearerObserver& aObserver,
	CDeltaTimer& aTimer,
	CAvrcpPlayerInfoManager& aPlayerInfoManager)
	{
	LOG_STATIC_FUNC
	CRcpRemoteDevice* engine = new(ELeave) CRcpRemoteDevice(aAddr, aRouter, aBearer, aTimer);
	CleanupStack::PushL(engine);
	engine->ConstructL(aObserver, aPlayerInfoManager);
	CleanupStack::Pop(engine);
	return engine;
	}

/** Constructor.

@param aAddr The remote device this manages transactions with.
@param aRouter	A CRcpRouter to use for communication with remote devices.
@param aBearer	The CRemConBearerAvrcp this is to handle commands for.
@param aObserver The observer of the bearer. Used to acquire converters.
@param aTimer	CDeltaTimer to use for queuing timed events.
@return A partially constructed CRcpRemoteDevice.
@leave System wide error codes.
*/
CRcpRemoteDevice::CRcpRemoteDevice(const TBTDevAddr& aAddr,
	CRcpRouter& aRouter, 
	CRemConBearerAvrcp& aBearer,
	CDeltaTimer& aTimer) : 
	iDevice(aAddr), iRouter(aRouter), iBearer(aBearer), iTimer(aTimer)
	{
	LOG_FUNC
	}

/** Second phase construction.

@param aObserver An observer to pass the handlers to allow them to
				acquire converters.
*/
void CRcpRemoteDevice::ConstructL(MRemConBearerObserver& aObserver,
	CAvrcpPlayerInfoManager& aPlayerInfoManager)
	{
	LOG_FUNC	
	iIncoming = CRcpIncomingCommandHandler::NewL(iBearer, aObserver, iRouter, iTimer, aPlayerInfoManager, iDevice);
	iOutgoing = CRcpOutgoingCommandHandler::NewL(iBearer, aObserver, iRouter, iTimer);
	}

/** Destructor.
*/
CRcpRemoteDevice::~CRcpRemoteDevice()
	{
	LOG_FUNC
	delete iIncoming;
	delete iOutgoing;
	}

/** Stop handling for this remote device. 

@param aClearQueue Whether the outgoing queue should be cleared.
*/
void CRcpRemoteDevice::Disconnect(TBool aClearQueue)
	{
	LOG_FUNC
	iIncoming->Disconnect();	
	iOutgoing->Disconnect(aClearQueue);
	}

/** Get the remote address of this device.

@return the remote address of this device.
*/
const TBTDevAddr& CRcpRemoteDevice::RemoteAddress() const
	{
	LOG_FUNC
	return iDevice;
	}	

/** Get the incoming handler for this device.

@return The incoming handler for this device.
*/	
MIncomingCommandHandler& CRcpRemoteDevice::IncomingHandler() const
	{
	LOG_FUNC
	return *iIncoming;
	}

/** Get the outgoing handler for this device.

@return The outgoing handler for this device.
*/	
MOutgoingCommandHandler& CRcpRemoteDevice::OutgoingHandler() const
	{
	LOG_FUNC
	return *iOutgoing;
	}


