/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*				 
*				 
*				 
*
*/


#ifndef BTSAC_GAVDP_OBSERVER_H
#define BTSAC_GAVDP_OBSERVER_H

#include <bluetoothav.h>

class MInternalGavdpUser
/**
Mixin class for GAVDP-related events.

To enforce thinking about the sequence of GAVDP events, these are pure
@publishedPartner
@released
*/
	{
public:
/**
Callback when a signalling channel has successfully been created
Note this can be called by GAVDP without the client having called RGavdp::Connect
The reason being that as soon as a GAVDP session is created AVDTP begins listening
Therefore a remote device can connect to us.
The GAVDP client can check the address to confirm if the device is that they originally Connect()ed
@param aDeviceAddr the Bluetooth device address for which a signalling channel has been created
*/
	virtual void GAVDP_ConnectConfirm(const TBTDevAddr& aDeviceAddr) =0;
/**
Callback when a remote SEP has been discovered
@param aSEP the details of the remote SEP
@see TAvdtpSEPInfo
*/
	virtual void GAVDP_SEPDiscovered(const TAvdtpSEPInfo& aSEP) =0;
/**
Callback when no more SEPs have been discovered on the remote device.
This may be called without GAVDP_SEPDiscovered being called if the remote has no SEPs
(such a remote would be in contravention of the specification however)
*/
	virtual void GAVDP_SEPDiscoveryComplete() =0;	
/**
Callback when a capability from a remote SEP has been retrieved
The client takes owenership of the capability given
@param aCapability pointer to a capability. Ownership is transferred to the MGavdpUser implementor.
@see TAvdtpServiceCapability
*/
	virtual void GAVDP_SEPCapability(TAvdtpServiceCapability* aCapability) =0; // transfers ownership
/**
Callback when remote SEP has no more capabilities to be discovered
*/
	virtual void GAVDP_SEPCapabilityComplete() =0;	
/**
Callback when the locally-initiated Abort stream has successfully completed
*/	
	virtual void GAVDP_AbortStreamConfirm()=0;
/**
Callback when the locally-initiated Start stream has successfully completed
*/	
	virtual void GAVDP_StartStreamsConfirm()=0;
/**
Callback when the locally-initiated Suspend stream has successfully completed
*/	
	virtual void GAVDP_SuspendStreamsConfirm()=0;
/**
Callback when the locally-initiated Security Control message successfully completed
@param aResponseData The data returned by the ACP - may be the same or different to that supplied by INT
*/	
	virtual void GAVDP_SecurityControlConfirm(const TDesC8& aResponseData)=0;

/**
Callback when locally-initiated (re)configuration procedure successfully completes
*/	
	virtual void GAVDP_ConfigurationConfirm()=0;	// configuration complete and SEP selected *AND* reconfigure confirm
	
/**
Callback when remote is beginning to configure a local SEP
This cannot be rejected by the client at this stage - it must examine each
capability proposed by the GAVDP peer.
@param aLocalSEID the SEID of the local SEP being configured
@param aRemoteSEID the SEID of the remote SEP configuring us
*/
	virtual void GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID) =0;
	
/**
Callback when each capability is being proposed by the peer during configuration
Each capability can be refused, but doing so completes whole configure operation
@return the client's result - eg whether it is accepting or rejecting the configuration
@param aCapability The capability being proposed. Ownership transferred to client.
*/
	virtual TInt GAVDP_ConfigurationIndication(TAvdtpServiceCapability* aCapability) =0;

/**
Callback when remotely initiated configuration of local SEP is completed	
The client can still error, as the local side may have expected to see more capabilities proposed
@return The client's acceptance or rejection of the whole configuration
*/	
	virtual TInt GAVDP_ConfigurationEndIndication() =0;

/**
Callback when the remote is trying to Start a stream
@param aSEID the SEID of the local SEP being started
*/
	virtual TInt GAVDP_StartIndication(TSEID aSEID) =0;
/**
Callback when the remote is trying to Suspend a stream
@param aSEID the SEID of the local SEP being suspended
*/
	virtual TInt GAVDP_SuspendIndication(TSEID aSEID) =0;
/**
Callback when the remote is sending Security Control information
@param aSEID the SEID of the local SEP for which security control is for
@param aSecurityDataInOut Security Data inbound. The client may change this descriptor for the outbound response
*/
	virtual TInt GAVDP_SecurityControlIndication(TSEID aSEID, TDes8& aSecurityDataInOut) =0;

/**
Callback when the remote has Aborted the stream
@param aSEID the SEID of the local SEP which has been Aborted
*/
	virtual void GAVDP_AbortIndication(TSEID aSEID) =0;
	
/**
Callback when the remote end has Released a stream
The user-plane bearers will subsequently error with KErrDisconnected.
@param aSEID the SEID of the local SEP which was been released
*/
	virtual void GAVDP_ReleaseIndication(TSEID aSEID) =0;
/**
Callback when a bearer has been created for a session in the stream.
The session the bearer is for can be examined from the socket address given in the callback
Each socket created in this way is created on the client's socket server session,
and responsibility for socket closure is ceded to the client.
@param aSocket the socket created on the client's socket server session.
@param aAddress the socket address (containing SEID, Session type)
@see TAvdtpSockAddr
*/
	virtual void GAVDP_BearerReady(RSocket aSocket, const TAvdtpSockAddr& aAddress)=0;

/**
Callback when a GAVDP error occurs
@param aError system-wide or GAVDP or AVDTP error code
@param aErrorData extra data relating to the error - the client can deduce the nature based on their GAVDP state and the spec
@see TAvdtpConfigRejectPckgBuf as an example of what may be passed back
*/
	virtual void GAVDP_Error(TInt aError, const TDesC8& aErrorData) =0;
	};
	
#endif // BTSAC_GAVDP_OBSERVER_H
	