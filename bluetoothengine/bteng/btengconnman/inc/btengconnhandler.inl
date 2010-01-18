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
* Description:  Inline definitions of methods forwarded to RBTEng.
*
*/



// -----------------------------------------------------------------------------
// ?implementation_description
// -----------------------------------------------------------------------------
//
inline TInt CBTEngConnHandler::ConnectDevice( const TBTDevAddr& aAddr, 
    const TBTDeviceClass& aDeviceClass  )
    {
    return iBTEng.ConnectDevice( aAddr, aDeviceClass );
    }


// -----------------------------------------------------------------------------
// ?implementation_description
// -----------------------------------------------------------------------------
//
inline TInt CBTEngConnHandler::CancelConnectDevice( const TBTDevAddr& aAddr )
    {
    return iBTEng.CancelConnectDevice( aAddr );
    }


// -----------------------------------------------------------------------------
// ?implementation_description
// -----------------------------------------------------------------------------
//
inline TInt CBTEngConnHandler::DisconnectDevice( const TBTDevAddr& aAddr, 
    TBTDisconnectType aDiscType )
    {
    return iBTEng.DisconnectDevice( aAddr, aDiscType );
    }


// -----------------------------------------------------------------------------
// ?implementation_description
// -----------------------------------------------------------------------------
//
inline TInt CBTEngConnHandler::IsDeviceConnected( const TBTDevAddr& aAddr, 
    TBTEngConnectionStatus& aConnected )
    {
    return iBTEng.IsDeviceConnected( aAddr, aConnected );
    }


// -----------------------------------------------------------------------------
// ?implementation_description
// -----------------------------------------------------------------------------
//
inline TInt CBTEngConnHandler::IsDeviceConnectable( 
    const TBTDeviceClass& aDeviceClass, TBool& aConnectable )
    {
    return iBTEng.IsDeviceConnectable(TBTDevAddr(), aDeviceClass, aConnectable );
    }

// -----------------------------------------------------------------------------
// ?implementation_description
// -----------------------------------------------------------------------------
//
inline TInt CBTEngConnHandler::IsDeviceConnectable( const TBTDevAddr& aAddr, 
        const TBTDeviceClass& aDeviceClass, TBool& aConnectable )
    {
    return iBTEng.IsDeviceConnectable( aAddr, aDeviceClass, aConnectable );
    }

// -----------------------------------------------------------------------------
// ?implementation_description
// -----------------------------------------------------------------------------
//
inline TInt CBTEngConnHandler::PrepareDiscovery()
    {
    return iBTEng.PrepareDiscovery();
    }
