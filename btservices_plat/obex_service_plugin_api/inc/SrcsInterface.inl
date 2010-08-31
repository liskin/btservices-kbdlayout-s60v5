/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
// CSrcsInterface
// -----------------------------------------------------------------------------
//
inline CSrcsInterface::CSrcsInterface()
    {
    }

// -----------------------------------------------------------------------------
// destructor
// -----------------------------------------------------------------------------
//
inline CSrcsInterface::~CSrcsInterface()
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
inline CSrcsInterface* CSrcsInterface::NewL(TUid aImplUid)
    {
    return REINTERPRET_CAST(CSrcsInterface*,
                            REComSession::CreateImplementationL(aImplUid,
                                                                _FOFF(CSrcsInterface,iDtor_ID_Key)));
    }

// -----------------------------------------------------------------------------
// ListAllImplementationsL
// -----------------------------------------------------------------------------
//
inline void CSrcsInterface::ListAllImplementationsL(RImplInfoPtrArray& aImplInfoArray)
    {
    REComSession::ListImplementationsL(KCSrcsInterfaceUid, aImplInfoArray);
    }

// -----------------------------------------------------------------------------
// ListImplementationsL
// -----------------------------------------------------------------------------
//
inline void CSrcsInterface::ListImplementationsL(const TDesC8& aMatchString, RImplInfoPtrArray& aImplInfoArray)
    {
    TEComResolverParams resolverParams;
    resolverParams.SetDataType(aMatchString);
    resolverParams.SetWildcardMatch(EFalse);
    REComSession::ListImplementationsL(KCSrcsInterfaceUid, resolverParams, aImplInfoArray);
    }

// -----------------------------------------------------------------------------
// SetMediaType
// -----------------------------------------------------------------------------
//
inline void CSrcsInterface::SetMediaType( TSrcsMediaType )
	{	
	}
// -----------------------------------------------------------------------------
// SetObexServer
// -----------------------------------------------------------------------------
//
inline TInt CSrcsInterface::SetObexServer( CObexServer* )
	{	
	return KErrNone;
	}
