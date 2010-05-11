// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
 @released
*/

#ifndef PLAYERAPPLICATIONSETTING_H
#define PLAYERAPPLICATIONSETTING_H


/**
Class containing all the information for one attribute.
All the attributes for a player are contained in a RHashMap 
of CPlayerApplicationSettings in CPlayerInfoTarget
*/
NONSHARABLE_CLASS(CPlayerApplicationSettings) : 	public CBase
	{
public:
	/**
	Factory function.
	@param  aAttributeID The ID value
	@param  aAttributeText A text description of the attribute
	@param  aValues A list of valid values
	@param  aValueTexts A list of descriptions of the possible values
	@param  aInitialValue The starting value
	@return A new CPlayerApplicationSettings
	*/
	static CPlayerApplicationSettings* NewL( TUint aAttributeID,
		TDesC8& 						aAttributeText,
		const RArray<TUint>& 			aValues,
		RArray<TPtrC8>&					aValueTexts,
		TUint 							aInitialValue );
	
	/** Destructor */
	~CPlayerApplicationSettings();

	TUint GetAttributeID();
	TUint GetCurrentValue();
	TPtrC8 GetAttributeText();
	RArray<TUint>* GetValues();
	RPointerArray<HBufC8>* GetValuesTexts();
	TInt SetCurrentValue(TUint aValue);

private:
	/** 
	Constructor.
	@param aInterfaceSelector The interface selector.
	*/
	CPlayerApplicationSettings( );

	void ConstructL(TUint aAttributeID,
		TDesC8& 						aAttributeText,
		const RArray<TUint>& 			aValues,
		RArray<TPtrC8>&					aValueText,
		TUint 							aInitialValue );

private:
	/* attribute ID */
	TUint 							iAttributeID;
	
	/* text description of the attribute */
	HBufC8*	 						iAttributeText;
	
	/* array of allowed values */
	RArray<TUint> 					iValues;
	
	/* array of text descriptions of allowed values */
	RPointerArray<HBufC8>			iValueText;
	
	/* the current value */
	TUint 							iCurrentValue;
	};
	
#endif // PLAYERAPPLICATIONSETTING_H
