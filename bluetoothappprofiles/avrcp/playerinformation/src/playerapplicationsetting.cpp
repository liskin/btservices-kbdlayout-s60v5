// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// This file contains the CPlayerApplicationSettings class.
// This is used to contain the Player application Setting Attributes
// defined by the Bluetooth AVRCP 1.3 specification
// 
//

/**
 @file
 @internalComponent
 @released
*/

#include <playerinformationtarget.h>

#include "playerapplicationsetting.h"
#include "playerinformation.h"

CPlayerApplicationSettings* CPlayerApplicationSettings::NewL( TUint aAttributeID,
		TDesC8& 						aAttributeText,
		RArray<TUint> 					aValues,
		RArray<TPtrC8>&					aValueTexts,
		TUint 							aInitialValue )
	{
	CPlayerApplicationSettings* self = new(ELeave) CPlayerApplicationSettings();
	CleanupStack::PushL(self);
	self->ConstructL( aAttributeID, aAttributeText, aValues, aValueTexts, aInitialValue);
	CleanupStack::Pop(self);
	return self;
	}
	

CPlayerApplicationSettings::~CPlayerApplicationSettings()
	{
	delete iAttributeText;
	iValues.Close();
	iValueText.ResetAndDestroy();
	}

CPlayerApplicationSettings::CPlayerApplicationSettings( )
	{
	}

void CPlayerApplicationSettings::ConstructL(TUint aAttributeID,
		TDesC8& 						aAttributeText,
		RArray<TUint> 					aValues,
		RArray<TPtrC8>&					aValueText,
		TUint 							aInitialValue )
	{
	iAttributeID = aAttributeID;

	// copy the attribute description	
	iAttributeText = aAttributeText.AllocL();

	// copy the allowed values
	for (TInt i=0; i < aValues.Count(); i++ )
		{
		
		// avoid duplicated defined values
		if (iValues.Find(aValues[i]) != KErrNotFound)
			{
			User::Leave(KErrArgument);
			}

		iValues.AppendL( aValues[i] );
		
		// copy the value description into a HBuf and add pointer to iValueTexts
		HBufC8 * valueText = aValueText[i].AllocL();
		CleanupStack::PushL(valueText);
		iValueText.AppendL(valueText);
		CleanupStack::Pop(valueText);
		}
		
	TInt error = SetCurrentValue( aInitialValue );
	if ( error != KErrNone )
		{	
		User::Leave( error );
		}
	}

TUint CPlayerApplicationSettings::GetAttributeID()
	{
	return( iAttributeID );
	}

TUint CPlayerApplicationSettings::GetCurrentValue()
	{
	return( iCurrentValue );
	}

TPtrC8 CPlayerApplicationSettings::GetAttributeText()
	{
	return( *iAttributeText );
	}

RArray<TUint>* CPlayerApplicationSettings::GetValues()
	{
	return( &iValues );
	}

RPointerArray<HBufC8>* CPlayerApplicationSettings::GetValuesTexts()
	{
	return( &iValueText );
	}

TInt CPlayerApplicationSettings::SetCurrentValue(TUint aValue)
	{
	// Only allow setting already defined values
	for (TInt i=0; i < iValues.Count(); i++ )
		{
		if ( aValue == iValues[i] )
			{
			iCurrentValue = aValue;
			return KErrNone;
			}
		}
	
	// value was not already defined, so return KErrArgument
	return KErrArgument;
	}

