/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Define SRCS's service controller ECom interface.
*
*/


// -----------------------------------------------------------------------------
// CSrcsTransport
// -----------------------------------------------------------------------------
//
inline CSrcsTransport::CSrcsTransport()
    {
    }
    
// -----------------------------------------------------------------------------
// ~CSrcsTransport
// -----------------------------------------------------------------------------
//
inline CSrcsTransport::~CSrcsTransport()
    {
    // Destroy any instance variables and then
    // inform the framework that this specific
    // instance of the interface has been destroyed.
    REComSession::DestroyedImplementation(iDtor_ID_Key);
    }

// -----------------------------------------------------------------------------
// NewL
// -----------------------------------------------------------------------------
//
inline CSrcsTransport* CSrcsTransport::NewL(TUid aImplUid, CImplementationInformation* aParams)
    {
    return REINTERPRET_CAST(CSrcsTransport*,
                            REComSession::CreateImplementationL(aImplUid,
                                                                _FOFF(CSrcsTransport,iDtor_ID_Key),
                                                                aParams));
    }

// -----------------------------------------------------------------------------
// ListAllImplementationsL
// -----------------------------------------------------------------------------
//
inline void CSrcsTransport::ListAllImplementationsL(RImplInfoPtrArray& aImplInfoArray)
    {
    REComSession::ListImplementationsL(KCSrcsTransportUid, aImplInfoArray);
    }

// -----------------------------------------------------------------------------
// ListImplementationsL
// -----------------------------------------------------------------------------
//
inline void CSrcsTransport::ListImplementationsL(const TDesC8& aMatchString, RImplInfoPtrArray& aImplInfoArray)
    {
    TEComResolverParams resolverParams;
    resolverParams.SetDataType(aMatchString);
    resolverParams.SetWildcardMatch(EFalse);
    REComSession::ListImplementationsL(KCSrcsTransportUid, resolverParams, aImplInfoArray);
    }
    
inline void CSrcsTransport::PostInitializeL()
    {
    }
