/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Inline code of CBtuiPluginInterface class.
*
*/


// -----------------------------------------------------------------------------
// Empty implementations of 3 pure virtual functions 
// defined in base class CGSBaseView
// -----------------------------------------------------------------------------
inline void CBtuiPluginInterface::NewContainerL()
	{	
	}
inline void CBtuiPluginInterface::HandleListBoxSelectionL()
	{	
	}
inline CBtuiPluginInterface::CBtuiPluginInterface()
    {
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
inline CBtuiPluginInterface::~CBtuiPluginInterface()
    {
    REComSession::DestroyedImplementation( iDtor_ID_Key );
    }

// -----------------------------------------------------------------------------
// CBtuiPluginInterface::NewL
// -----------------------------------------------------------------------------
//
inline CBtuiPluginInterface* CBtuiPluginInterface::NewL( 
    TUid aImplementationUid, MBtuiPluginViewActivationObserver* aObserver)
    {
    TInt32 keyOffset = _FOFF( CBtuiPluginInterface, iDtor_ID_Key );
    TAny* ptr = REComSession::CreateImplementationL( aImplementationUid, keyOffset, aObserver);
    
    return reinterpret_cast< CBtuiPluginInterface* >( ptr ) ;
    }

// -----------------------------------------------------------------------------
// CBtuiPluginInterface::NewL
// offer a default implementation to make it non-pure virtual.
// -----------------------------------------------------------------------------
//
inline void CBtuiPluginInterface::GetCaptionL( TDes& aCaption ) const
	{
	_LIT( NO_CAPTION,"no caption to offer" );
	aCaption.Copy(NO_CAPTION);
	return;
	}

// -----------------------------------------------------------------------------
// CBtuiPluginInterface::SetCOD
// only used for derived add-on plugins out of BTUI
// -----------------------------------------------------------------------------
//
inline void CBtuiPluginInterface::SetCOD(TBTDeviceClass& aCOD)
	{
	iCOD = aCOD;
	}
inline TBTDeviceClass CBtuiPluginInterface::GetCOD() const
    {
	return iCOD;
    }
