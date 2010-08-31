/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Data converter implementation
*
*/


// INCLUDE FILES
#include "BTSUDataConverter.h"
#include <es_sock.h> // for BigEndian functions
#include "BTSUDebug.h"

// CONSTANTS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TBTSUDataConverter::ConvertByteL
// -----------------------------------------------------------------------------
//
TUint8 TBTSUDataConverter::ConvertByteL( const TPtrC8& aData )
    {
    if ( aData.Length() != sizeof(TUint8) )
        {
        User::Leave( KErrArgument );
        }
    return *aData.Ptr();
    }

// -----------------------------------------------------------------------------
// TBTSUDataConverter::ConvertDataSignedL
// -----------------------------------------------------------------------------
//
TInt32 TBTSUDataConverter::ConvertDataSignedL( const TPtrC8& aData )
    {
    if ( aData.Length() != sizeof(TInt32) )
        {
        User::Leave( KErrArgument );
        }
    TUint32 rawVal = BigEndian::Get32(aData.Ptr());
    return *reinterpret_cast<TInt32*>(&rawVal); // reinterpret cast to extract signed nature "unscathed"
    }

// -----------------------------------------------------------------------------
// TBTSUDataConverter::ConvertDataUnsignedL
// -----------------------------------------------------------------------------
//
TUint32 TBTSUDataConverter::ConvertDataUnsignedL( const TPtrC8& aData )
    {
    if ( aData.Size() != sizeof(TUint32) )
        {
        User::Leave( KErrArgument );
        }
    return BigEndian::Get32(aData.Ptr());
    }

//  End of File  
