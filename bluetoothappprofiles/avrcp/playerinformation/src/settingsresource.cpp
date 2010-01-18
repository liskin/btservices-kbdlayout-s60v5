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
//

/**
 @file
 @publishedAll
 @released
*/

#include <playerinformationtargetobserver.h>
#include <barsread2.h>
#include <barsc2.h>


// AVRCP resource files must only contain one top level resource.
static const int KAVRCPResourceId = 1;
static const TInt KAVRCPFormatVersion = 1;


EXPORT_C void PlayerApplicationSettingsResourceInit::DefineAttributesL(MPlayerApplicationSettingsObserver& aSettings,
																CResourceFile& aResourceFile)
	{
	// Read the settings from the resource file
	RResourceReader reader;
	reader.OpenLC(&aResourceFile, KAVRCPResourceId);
	
	// Check the version of this resource file
	TUint16 formatVersion = reader.ReadUint16L();
	if (formatVersion != KAVRCPFormatVersion)
		{
		reader.Close();
		User::Leave(KErrCorrupt);
		}
	
	// Find out how many settings this resource file contains
	TInt settingsCount = reader.ReadUint16L();
	if (settingsCount < 0)
		{
		reader.Close();
		User::Leave(KErrCorrupt);
		}

	// read the settings from aResource
	RArray<TUint> values;
	RArray<TPtrC8> valueTexts;
	CleanupClosePushL(values);
 	CleanupClosePushL(valueTexts);

	// Read each AVRCP attribute setting in turn
	for (int i = 0; i < settingsCount; i++)
		{
		values.Reset();
		valueTexts.Reset();
		
		// Read attribute, description text, initial value and defined values
		TUint attributeID(reader.ReadUint8L());
		TUint initialValue(reader.ReadUint8L());
		TPtrC8 attributeText(reader.ReadTPtrC8L());
		TInt valuesCount(reader.ReadUint16L());

		for (int j = 0; j < valuesCount; j++)
			{
			TUint thisValue = reader.ReadUint8L();
			TPtrC8 thisDescription = reader.ReadTPtrC8L();
			
			values.AppendL(thisValue);
			valueTexts.AppendL(thisDescription);
			}
		
		// Now define this attribute, then continue to next attribute
		aSettings.DefineAttributeL(attributeID, attributeText, values, valueTexts, initialValue);
		}
	// cleaning up values and valueTexts
	 CleanupStack::PopAndDestroy(3); // values, valueTexts, reader
	}

