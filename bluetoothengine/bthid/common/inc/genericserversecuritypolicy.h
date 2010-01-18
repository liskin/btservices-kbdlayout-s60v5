/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
*/


/**
 @file
 */
#ifndef __GENERICSERVERSECURITYPOLICY_H__
#define __GENERICSERVERSECURITYPOLICY_H__

const TUint KGenericServerRangeCount = 2; //16;

const TInt KGenericServerRanges[KGenericServerRangeCount] =
    {
    0, 16
    };
/*
 EGenericCreateHostResolverSubSession,	//	0	=	Alway fail
 EGenericCreateRegistrySubSession,		//	1	=	Local Services
 EGenericRegistrySearch,				//	2	=	Always pass
 EBTRegistryRetrieveDevice,			//	3	=	Always fail
 EBTRegistryAddDevice,				//	4	=	Always pass
 EBTRegistryModifyBluetoothName,		//	5	=	Write Device Data
 EBTRegistryModifyFriendlyName,		//	6	=	Always pass
 EBTRegistryModifyNamelessDevice,	//	7	=	Write Device Data
 EBTRegistryDeleteDevices,			//	8	=	Always pass
 EBTRegistryDeleteLinkKey,			//	9	=	Write Device Data
 EBTRegistryCloseView,				//	10	=	Always pass
 EBTRegistryUpdateLocalDevice,		//	11	=	Write Device Data
 EBTRegistryGetCommPortSettings,		//	12	=	Always pass
 EBTHostResolverDeviceRequest,		//	13	=	Always fail
 EGenericSetHeapFailure,				//	14	=	Local Services
 //	If new function enums are added, the following value needs to
 //	increase to be <last function number> + 1.
 //	This defines a final range that extends from the first invalid
 //	function number to KMaxTInt, and protects against attempts to
 //	attack the server with invalid functions.
 
 EGenericSubSessionCount+1				//	15	=	CPolicyServer::ENotSupported for all other fns.
 };
 */

/** Index numbers into KGenericServerElements[] */
const TInt KPolicyAlwaysFail = 0;
const TInt KPolicyLocalServices = 1;
const TInt KPolicyAlwaysPass = 2;
const TInt KPolicyWriteDeviceData = 3;

/**Mapping IPCs to policy element */
const TUint8 KGenericServerElementsIndex[KGenericServerRangeCount] =
    {
    KPolicyAlwaysPass, KPolicyAlwaysPass

    //	KPolicyAlwaysFail,				/**	EGenericCreateHostResolverSubSession	*/
        //	KPolicyLocalServices,			/**	EGenericCreateRegistrySubSession	*/
        /**	EGenericCreateCommPortSettingsSubSession	*/
        /**	EGenericCreateLocalDeviceSubSession	*/
        //	KPolicyAlwaysPass,				/**	EGenericRegistrySearch	*/
        /**	EGenericExtractRegistryDataIntoServer	*/
        /**	EGenericRetrieveRegistryData	*/
        /**	EGenericCloseSubSession	*/
        /**	EGenericCancelRequest	*/
        //	KPolicyAlwaysFail,				/**	EBTRegistryRetrieveDevice	*/
        //	KPolicyAlwaysPass,				/**	EBTRegistryAddDevice	*/
        /**	EBTRegistryGetNamelessDevice	*/
        //	KPolicyWriteDeviceData,			/**	EBTRegistryModifyBluetoothName	*/
        //	KPolicyAlwaysPass,				/**	EBTRegistryModifyFriendlyName	*/
        //	KPolicyWriteDeviceData,			/**	EBTRegistryModifyNamelessDevice	*/
        //	KPolicyAlwaysPass,				/**	EBTRegistryDeleteDevices	*/
        //	KPolicyWriteDeviceData,			/**	EBTRegistryDeleteLinkKey	*/
        /**	EBTRegistryUnpairView	*/
        //	KPolicyAlwaysPass,				/**	EBTRegistryCloseView	*/
        /**	EBTRegistryGetLocalDevice	*/
        //	KPolicyWriteDeviceData,			/**	EBTRegistryUpdateLocalDevice	*/
        //	KPolicyAlwaysPass,				/**	EBTRegistryGetCommPortSettings	*/
        /**	EBTRegistryUpdateCommPortSettings	*/
        /**	EBTRegistryDeleteCommPortSettings	*/
        //	KPolicyAlwaysFail,				/**	EBTHostResolverDeviceRequest	*/
        /**	EBTHostResolverGetNextDeviceRequest	*/
        /**	EBTHostResolverDeviceModifyDevice	*/
        /**	EBTHostResolverNotifyRequest	*/
        //	KPolicyLocalServices,			/**	EGenericSetHeapFailure	*/
        /**	EGenericSubSessionCount	*/
        //	CPolicyServer::ENotSupported	/**	EGenericCreateSecManSubSession	*/
        /**	EGenericCreateSecuritySettingsSubSession	*/
        /**	EGenericCreateBasebandSecuritySubSession	*/
        /**	EBTSecManAccessRequest	*/
        /**	EBTSecuritySettingsRegister	*/
        /**	EBTSecuritySettingsUnregister	*/
        /**	EBTBasebandSecurityLinkKeyRequest	*/
        /**	EBTBasebandSecurityNewLinkKey	*/
        /**	EBTBasebandSecurityPinRequest	*/
        /**	EBTBasebandSecurityNewLinkState	*/
        /**	EBTBasebandSecurityHCIRequestHandler	*/
        /**	EBTBasebandSecurityNameRequestResponse	*/
        /**	...and onwards to KMaxTInt	*/
        };

/** Individual policy elements */
const CPolicyServer::TPolicyElement KGenericServerElements[] =
    {
    /** the EFailClient means that the if the check fails the CheckFailed method with return KErrPermissionDenied */
        {
        _INIT_SECURITY_POLICY_FAIL
        }, /**	policyAlwaysFail	*/
        {
                _INIT_SECURITY_POLICY_C1(ECapabilityLocalServices),
                CPolicyServer::EFailClient
        }, /**	policyLocalService	*/
        {
        _INIT_SECURITY_POLICY_PASS
        }, /**	policyAlwaysPass	*/
        {
                _INIT_SECURITY_POLICY_C1(ECapabilityWriteDeviceData),
                CPolicyServer::EFailClient
        } /**	policyWriteDevData	*/
    };

/** Main policy */
const CPolicyServer::TPolicy KGenericServerPolicy =
    {
            CPolicyServer::EAlwaysPass, /** Specifies all connect attempts should pass */
            KGenericServerRangeCount,
            KGenericServerRanges,
            KGenericServerElementsIndex,
            KGenericServerElements,
    };

#endif	//__GENERICSERVERSECURITYPOLICY_H__
