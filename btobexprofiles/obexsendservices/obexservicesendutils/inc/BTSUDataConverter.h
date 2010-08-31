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
* Description:  A data conversion class.
*
*/


#ifndef BTSU_DATA_CONVERTER_H
#define BTSU_DATA_CONVERTER_H

// INCLUDES
#include <e32std.h>

// CONSTANTS

// DATA TYPES

// CLASS DECLARATION

/**
*   A class for executing data conversion
*/
class TBTSUDataConverter
    {
    public:

        /**
        * Converts the given data as integer.
        * @param aData The data to be converted.
        * @return The data converted as TUint8
        */
        static TUint8 ConvertByteL( const TPtrC8& aData );

        /**
        * Converts the given data as signed integer.
        * @param aData The data to be converted.
        * @return The data converted as TInt32.
        */
        static TInt32 ConvertDataSignedL( const TPtrC8& aData );

        /**
        * Converts the given data as unsigned integer.
        * @param aData The data to be converted.
        * @return The data converted as TUint32.
        */
        static TUint32 ConvertDataUnsignedL( const TPtrC8& aData );
    };

#endif      // BTSU_DATA_CONVERTER_H
            
// End of File
