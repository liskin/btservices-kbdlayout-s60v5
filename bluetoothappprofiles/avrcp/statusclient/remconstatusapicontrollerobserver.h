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

/**
 @file
 @publishedAll
 @released
*/

#ifndef REMCONSTATUSAPICONTROLLEROBSERVER_H
#define REMCONSTATUSAPICONTROLLEROBSERVER_H

#include <e32std.h>

/**
Interface to observe responses from a Status API controller.
@publishedAll
@released
*/
class MRemConStatusApiControllerObserver
	{
public:
	/**	A UnitInfo response has been received. 
	
	For details on the meanings on the fields please refer to the 
	AV/C Digital Interface Command Set General Specification, 
	section 11.2.1.
	
	If all values are zero there was an error with the command.
	@param aVendorId The VendorId
	@param aUnit Unit
	@param aUnitType UnitType
	@param aExtendedUnitType Extended Unit type. This data is only 
		valid if Unit type is 0x1e. The second byte is only valid 
		if the first byte is 0xff.
	*/
	virtual void MrcsacoUnitInfoResponse(TInt aVendorId, 
		TInt aUnit, 
		TInt aUnitType, 
		TInt aExtendedUnitType) = 0;

	/** A SubunitInfo response has been received. 
	
	For details on the meanings on the fields please
	refer to the AV/C Digital Interface Command Set
	General Specification, section 11.2.2.
	
	If all values are zero there was an error with the command.
	@param aPage Page
	@param aExtension Extension.
	@param aPageData Page Data. This data remains valid only for
		the duration of this call.  If the client wishes to store
		the data it must be copied.
	*/
	virtual void MrcsacoSubunitInfoResponse(TInt aPage,
		TInt aExtension,
		TDesC8& aPageData) = 0;
		
	/**
 	 Returns a null aObject if the extension is not implemented, or 
 	 a pointer to another interface if it is.
 	 
	 @param aInterface UID of the interface to return
	 @param aObject the container for another interface as specified by aInterface
	 */
	IMPORT_C virtual void Mrcsaco_ExtensionInterfaceL(TUid aInterface, void*& aObject);
	};

#endif // REMCONSTATUSAPICONTROLLEROBSERVER_H
