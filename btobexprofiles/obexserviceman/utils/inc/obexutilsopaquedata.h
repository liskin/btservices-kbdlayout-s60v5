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
* Description: 
*     Service controller resouce file's opaque_data parser.
*
*/


#ifndef OBEXUTILSOPAQUEDATA_H
#define OBEXUTILSOPAQUEDATA_H


//  INCLUDES
#include <e32std.h>
#include <e32cons.h>

const TUint KObexUtilsMaxOpaqueDataStringLen = 0x100;

// CLASS DECLARATION

/**
* Process opaque_data field in service controller ECom plug-in resource file.
* opaque_data field contains 2 types of data: string and number.
*/
NONSHARABLE_CLASS( CObexUtilsOpaqueData ): public CBase
    {
public:

	/**
    * constructor.
	* @param	aDes buffer to hold the opaque_data
    */
    IMPORT_C CObexUtilsOpaqueData(const TDesC8& aDes);

	/**
    * Get string from current location. Return error code if invalid string format..
	* @param	aString buffer to hold the parsed string.
	* @return	Error code.
    */
    IMPORT_C TInt GetString(TBuf8<KObexUtilsMaxOpaqueDataStringLen> &aString);

	/**
    * Get number from current location. Return error code if invalid string format..
	* @param	aNumber the parsed number.
	* @return	Error code.
    */
    IMPORT_C TInt GetNumber(TUint &aNumber);

    /*
	* Peek if next entry is string by looking for a '\'.
	* @return	TRUE:   is valid string.
	*			FALSE:	is not string.
	*/
    IMPORT_C TBool IsString();

private:
    TLex8 iData;// data buffer.
    };

#endif
