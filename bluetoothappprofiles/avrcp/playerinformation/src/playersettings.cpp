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

#include <remconinterfaceselector.h>
#include <playerinformationtarget.h>

#include "playerapplicationsetting.h"
#include "playerinformation.h"
#include "playersettingsutils.h"


void CPlayerInfoTarget::SendError(TInt aError, TInt aOperationId)
	{
	TInt error = 0;
	RAvrcpIPCError response;
	response.iError = aError;
	TRAP(error, response.WriteL(iOutBuf));   // Try to send internal error if OOM
	if (error == KErrNone)
		{
		InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
										aOperationId, ERemConResponse, iOutBuf);
		}								
	}


// PDU 0x11
void CPlayerInfoTarget::ProcessListPlayerApplicationAttributes(TInt aOperationId)
	{
	RRemConPlayerListOfAttributes response;
	response.iNumberAttributes = iPlayerApplicationSettings.Count();

	THashMapIter<TInt, CPlayerApplicationSettings*> iter(iPlayerApplicationSettings);
	const TInt* attribute = iter.NextKey();
	while ( attribute != NULL )
		{
		if (response.iAttributes.Append(*attribute) != KErrNone)
			{
			response.Close();
			return SendError(KErrAvrcpMetadataInternalError, aOperationId);   // Try to send internal error if OOM
			}
		attribute = iter.NextKey();
		}

	// send the response back to the CT
	TInt error = 0;
	TRAP(error, response.WriteL(iOutBuf));   // Try to send internal error if OOM
	response.Close();
	if (error != KErrNone)
		{
		return SendError(KErrAvrcpMetadataInternalError, aOperationId);
		}
		
	InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
									EListPlayerApplicationSettingAttributes, ERemConResponse, iOutBuf );
	}

// PDU 0x12
void CPlayerInfoTarget::ProcessListPlayerApplicationValues(const TDesC8& aData, TInt aOperationId)
	{
	TInt error =0;
	RDesReadStream readStream;
	readStream.Open(aData);
	TInt attributeID = 0;
	TRAP(error, attributeID = readStream.ReadUint8L());
	readStream.Close();
	if (error != KErrNone)
		{
		return SendError(KErrAvrcpMetadataParameterNotFound, aOperationId);  // Nothing in packet
		}
		
	// Send the number of values for this attribute (1 byte),
	// followed by the defined values themselves (n x 1byte)
	CPlayerApplicationSettings* thisSetting = GetSetting(attributeID);
	if (thisSetting == NULL) 
		{
		// Attribute Id not found
		return SendError(KErrAvrcpMetadataInvalidParameter, aOperationId);  
		}
		
	RRemConPlayerListOfAttributes response;
	RArray<TUint>* values = thisSetting->GetValues();
	TInt numValues = values->Count();
	response.iNumberAttributes = numValues;

	// Make sure that we always have at least one result to return
	// Table 5.18 says that the number of results provided has an
	// allowed value of 1-255, so we cannot return zero results.
	if (response.iNumberAttributes == 0)
		{
		return SendError(KErrAvrcpMetadataParameterNotFound, aOperationId);  // No attributes matched
		}

	for ( TInt i = 0; i < numValues; i++ )
		{
		TInt value = (*values)[i];
		if (response.iAttributes.Append(value) != KErrNone)
			{
			response.Close();
			return SendError(KErrAvrcpMetadataInternalError, aOperationId);   // Try to send internal error if OOM
			}
		}

	// send the response back to the CT
	TRAP(error, response.WriteL(iOutBuf));   // Try to send internal error if OOM
	response.Close();
	if (error != KErrNone)
		{
		return SendError(KErrAvrcpMetadataInternalError, aOperationId);
		}
		
	InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
									aOperationId, ERemConResponse, iOutBuf );
	}

// PDU 0x15
void CPlayerInfoTarget::ProcessGetPlayerApplicationAttributeText(const TDesC8& aData, TInt aOperationId)
	{
	TInt error =0;

	// If we can't parse the request, then return an error
	RRemConPlayerListOfAttributes request;
	TRAP(error, request.ReadL(aData));
	if (error != KErrNone)
		{
		request.Close();
		return SendError(KErrAvrcpMetadataInvalidCommand, aOperationId);
		}
		
	// Iterate through requested attributes and remove
	// those which we don't have any settings for.
	for (TInt i = 0; i < request.iNumberAttributes; i++)
		{
		if (! AttributeSettingExists(request.iAttributes[i]))
			{
			request.iAttributes.Remove(i);
			request.iNumberAttributes--;
			i--;
			}
		}
		
	// Make sure that we always have at least one result to return
	// Table 5.18 says that the number of results provided has an
	// allowed value of 1-255, so we cannot return zero results.
	RRemConGetPlayerApplicationTextResponse response;
	response.iNumberAttributes = request.iNumberAttributes;
	if (response.iNumberAttributes == 0)
		{
		request.Close();
		return SendError(KErrAvrcpMetadataInvalidParameter, aOperationId);  // No attributes matched
		}

	// for every attribute text requested
	for (TInt i = 0; i < request.iNumberAttributes; i++)
		{
		// start with the attribute id requested and the character set
		RSettingWithCharset setting;
		TInt attId = request.iAttributes[i];
		CPlayerApplicationSettings* thisSetting = GetSetting(attId);
		setting.iAttributeId = attId;
		setting.iCharset = KUtf8MibEnum;
		const TPtrC8 text = thisSetting->GetAttributeText();
		setting.iStringLen = text.Length();
		setting.iString = text.Alloc();
		
		// If OOM, try to return an internal error.
		if (setting.iString == NULL)
			{
			request.Close();
			response.Close();
			return SendError(KErrAvrcpMetadataInternalError, aOperationId);
			}
			
		// If OOM, try to return an internal error.
		if (response.iAttributes.Append(setting) != KErrNone)
			{
			setting.Close();
			request.Close();
			response.Close();
			return SendError(KErrAvrcpMetadataInternalError, aOperationId);
			}
		}
	request.Close();
	
	// Allocate a buffer for the formatted message
	RBuf8 messageBuffer;
	if ( messageBuffer.Create(response.Size()) != KErrNone )
		{
		// On OOM drop the message
		response.Close();
		return;
		}
		
	// send the response back to the CT
	TRAP(error, response.WriteL(messageBuffer));   // Try to send internal error if OOM
	response.Close();
	if (error != KErrNone)
		{
		messageBuffer.Close();
		return SendError(KErrAvrcpMetadataInternalError, aOperationId);
		}
		
	InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
									aOperationId, ERemConResponse, messageBuffer );
	
	messageBuffer.Close();
	}

// PDU 0x14
void CPlayerInfoTarget::ProcessSetPlayerApplicationValue(const TDesC8& aData, TInt aOperationId)
	{
	RRemConPlayerAttributeIdsAndValues request;
	TInt error = 0;

	// If we can't parse the request, then return an error
	TRAP(error, request.ReadL(aData));
	if (error != KErrNone)
		{
		request.Close();
		return SendError(KErrAvrcpMetadataInvalidCommand, aOperationId);
		}
		
	// Iterate through all the settings we're sent to update and set them
	for (TInt i = 0; i < request.iNumberAttributes; i++)
		{
		if (! AttributeValueCanBeSet(request.iAttributeId[i], request.iAttributeValue[i]))
			{
			// remove this setting
			request.iAttributeId.Remove(i);
			request.iAttributeValue.Remove(i);
			request.iNumberAttributes--;
			i--;
			}
		}
	__ASSERT_DEBUG(request.iAttributeId.Count() == request.iAttributeValue.Count(), PlayerSettingsUtils::Panic(EPlayerSettingsFunnyLengthData));
	
	// Section 5.7 of the AVRCP specification (page 56) says:
	// If CT sent a PDU with nonexistent PDU ID or a PDU containing
	// only one parameter with nonexistent parameter ID, TG shall return
	// REJECTED response with Error Status Code. If CT sent a PDU with
	// multiple parameters where at least one ID is existent and the
	// others are nonexistent, TG shall proceed with the existent ID and
	// ignore the non-existent IDs.
	//
	// This means we return REJECTED if we have nothing to set
	if (request.iNumberAttributes == 0)
		{
		request.Close();
		return SendError(KErrAvrcpMetadataInvalidParameter, aOperationId);
		}
		
	// If an application setting notifier has been requested then notify it
	// If the notification succeeds, then set all the values

	TRAP(error, iApplicationSettingNotifier.MpasnSetPlayerApplicationValueL(request.iAttributeId, request.iAttributeValue));
	if ( error == KErrNone )
		{
		for (TInt i = 0; i < request.iNumberAttributes; i++)
			{
			CPlayerApplicationSettings* thisSetting = GetSetting(request.iAttributeId[i]);
			thisSetting->SetCurrentValue(request.iAttributeValue[i]);
			}
		}
	else
		{
		// Return an AVRCP internal error via RemCon. See section 5.7.1 of specification.
		request.Close();
		return SendError(KErrAvrcpMetadataInternalError, aOperationId);
		}
		
	request.Close();
     // Send a notification if one has been registered
     if ( KErrNotFound != iPendingNotificationEventList.Find( ERegisterNotificationPlayerApplicationSettingChanged ))
          {
           SendNotificationResponse( ERegisterNotificationPlayerApplicationSettingChanged, ERemConNotifyResponseChanged );
          }
       
		
	// SendError KErrNone is used to send a valid response
	SendError(KErrNone, aOperationId);
	}

// PDU 0x13
void CPlayerInfoTarget::ProcessGetCurrentPlayerApplicationValue(const TDesC8& aData, TInt aOperationId)
	{
	TInt error = 0;

	// If we can't parse the request, then return an error
	RRemConPlayerListOfAttributes request;
	TRAP(error, request.ReadL(aData));
	if (error != KErrNone)
		{
		request.Close();
		return SendError(KErrAvrcpMetadataInvalidCommand, aOperationId);
		}
		
	// Look through requested attributes, and assemble a response
	// for those which we posess. If none are found, return an error
	RRemConPlayerAttributeIdsAndValues response;
	response.iNumberAttributes = 0;
	request.iAttributes.Sort();
	
	for (TInt i = 0; i < request.iNumberAttributes; i++)
		{
		TInt attToSend = request.iAttributes[i];
		if (AttributeSettingExists(attToSend))
			{
			TInt ret1 = response.iAttributeId.Append(attToSend);
			TInt ret2 = response.iAttributeValue.Append(GetSetting(attToSend)->GetCurrentValue());
			if (ret1 != KErrNone || ret2 != KErrNone)
				{
				request.Close();
				response.Close();
				return SendError(KErrAvrcpMetadataInternalError, aOperationId);   // Try to send internal error if OOM
				}
			response.iNumberAttributes++;
			}
		}
	request.Close();
		
	// Make sure that we always have at least one result to return
	// Table 5.18 says that the number of results provided has an
	// allowed value of 1-255, so we cannot return zero results.
	if (response.iNumberAttributes == 0)
		{
		response.Close();
		return SendError(KErrAvrcpMetadataInvalidParameter, aOperationId);  // No attributes matched
		}
		
	// send the response back to the CT
	TRAP(error, response.WriteL(iOutBuf));   // Try to send internal error if OOM
	response.Close();
	if (error != KErrNone)
		{
		return SendError(KErrAvrcpMetadataInternalError, aOperationId);
		}
		
	InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
									aOperationId, ERemConResponse, iOutBuf );
	}

// PDU 0x16
void CPlayerInfoTarget::ProcessGetPlayerApplicationValueText(const TDesC8& aData, TInt aOperationId)
	{
	RDesReadStream readStream;
	readStream.Open(aData);

	TInt error = 0;
	TInt reqAttribute = 0;
	
	// Read the attribute id
	TRAP(error, reqAttribute = readStream.ReadUint8L());
	if (error != KErrNone)
		{
		readStream.Close();
		return SendError(KErrAvrcpMetadataParameterNotFound, aOperationId);  // Nothing in packet
		}
		
	// If we don't have settings for this attribute, return an error
	CPlayerApplicationSettings* thisSetting = GetSetting(reqAttribute);
	if (thisSetting == NULL)
		{
		readStream.Close();
		return SendError(KErrAvrcpMetadataInvalidParameter, aOperationId);  // Attribute not found
		}
		
	// Read the number of PAS values
	TInt numSettings = 0;
	TRAP(error, numSettings = readStream.ReadUint8L());
	if (error != KErrNone)
		{
		readStream.Close();
		return SendError(KErrAvrcpMetadataParameterNotFound, aOperationId);  // Nothing in packet
		}
	
	RArray<TInt> valueTextsRequested;
	for (TInt i = 0; i < numSettings; i++)
		{
		TInt requestedValueText = 0;
		TRAP(error, requestedValueText = readStream.ReadUint8L());
		if (error == KErrNone)
			{
			if (valueTextsRequested.Append(requestedValueText) != KErrNone)
				{
				readStream.Close();
				return SendError(KErrAvrcpMetadataInternalError, aOperationId);   // Try to send internal error if OOM
				}
			}
		}
	
	readStream.Close();
	
	// format the response in a RRemConGetPlayerApplicationTextResponse
	RRemConGetPlayerApplicationTextResponse response;
	response.iNumberAttributes = 0;

	// for every attribute text requested
	RPointerArray<HBufC8>* textValues = thisSetting->GetValuesTexts();
	RArray<TUint>* values = thisSetting->GetValues();
	TInt numRequested = valueTextsRequested.Count();
	for (TInt i = 0; i < numRequested; i++)
		{
		// start with the attribute id requested and the character set
		RSettingWithCharset setting;
		TInt valueToSend = valueTextsRequested[i];
		setting.iAttributeId = valueToSend;
		setting.iCharset = KUtf8MibEnum;

		// text length followed by the text
		TInt found = values->Find(valueToSend);
		if (found != KErrNotFound)
			{
			HBufC8* text = (*textValues)[found];
			setting.iStringLen = text->Length();
			setting.iString = text->Alloc();
			
			// If OOM, try to return an internal error.
			if (setting.iString == NULL)
				{
				response.Close();
				valueTextsRequested.Close();
				return SendError(KErrAvrcpMetadataInternalError, aOperationId);
				}
				
			// If OOM, try to return an internal error. Of course, this could fail too
			if (response.iAttributes.Append(setting) != KErrNone)
				{
				response.Close();
				setting.Close();
				valueTextsRequested.Close();
				return SendError(KErrAvrcpMetadataInternalError, aOperationId);
				}
				
			response.iNumberAttributes++;
			}
		}
	valueTextsRequested.Close();
	
	// Make sure that we always have at least one result to return
	// Table 5.18 says that the number of results provided has an
	// allowed value of 1-255, so we cannot return zero results.
	if (response.iNumberAttributes == 0)
		{
		response.Close();
		return SendError(KErrAvrcpMetadataInvalidParameter, aOperationId);
		}
		
	// Allocate a buffer for the formatted message
	RBuf8 messageBuffer;
	if ( messageBuffer.Create(response.Size()) != KErrNone )
		{
		// On OOM drop the message
		response.Close();
		return;
		}
		
	// send the response back to the CT
	TRAP(error, response.WriteL(messageBuffer));   // Try to send internal error if OOM
	response.Close();
	if (error != KErrNone)
		{
		messageBuffer.Close();
		return SendError(KErrAvrcpMetadataInternalError, aOperationId);
		}
		
	InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
									aOperationId, ERemConResponse, messageBuffer );
	messageBuffer.Close();
	}

TBool CPlayerInfoTarget::AttributeSettingExists(TUint anAttributeID)
	{
	if (GetSetting(anAttributeID) == NULL)
		{
		return EFalse;
		}
	else
		{
		return ETrue;
		}
	}

TBool CPlayerInfoTarget::IsValidAttributeValue(TUint anAttributeID, TUint anAttributeValue)
	{
	// As defined in Appendix F of the AVRCP specification, page 81
	// Attribute 0x01 - range 0x01 to 0x02
	// Attribute 0x02 - range 0x01 to 0x04
	// Attribute 0x03 - range 0x01 to 0x03
	// Attribute 0x04 - range 0x01 to 0x03
	// Attribute 0x05 - 0x7f -- invalid; reserved for future use
	// Attribute 0x80 - 0xff -- vendor dependent
	if (anAttributeID >= 0x80 && anAttributeID <= 0xff && anAttributeValue <= 0xff )
		{
		return ETrue;
		}
		
	if (anAttributeID == 0x01 && anAttributeValue >= 0x01 && anAttributeValue <= 0x02)
		{
		return ETrue;
		}
		
	if (anAttributeID == 0x02 && anAttributeValue >= 0x01 && anAttributeValue <= 0x04)
		{
		return ETrue;
		}
		
	if ((anAttributeID == 0x03 || anAttributeID == 0x04)
		&& anAttributeValue >= 0x01 && anAttributeValue <= 0x03)
		{
		return ETrue;
		}
		
	// Everything else is invalid, as defined by the specification
	return EFalse;
	}

// Check that anAttributeValue is in the valid value list for anAttributeID
TBool CPlayerInfoTarget::AttributeValueCanBeSet(TUint anAttributeID, TUint anAttributeValue)
	{
	CPlayerApplicationSettings* thisSetting = GetSetting(anAttributeID);
	if (thisSetting == NULL)
		{
		return EFalse;
		}
		
	RArray<TUint>* values = thisSetting->GetValues();
	if (values == NULL)
		{
		return EFalse;
		}
		
	if (values->Find(anAttributeValue) == KErrNotFound)
		{
		return EFalse;
		}
		
	// This attribute id and value has been already defined by the RSS
	// file, and checked that it conforms to the specification when the
	// RSS file was loaded. Now allow this value to be set over the air.
	return ETrue;
	}

CPlayerApplicationSettings* CPlayerInfoTarget::GetSetting(TUint anAttributeID)
	{
	// Will return NULL if anAttributeID is not found
	CPlayerApplicationSettings** settings = iPlayerApplicationSettings.Find(anAttributeID);
	if (settings == NULL)
		{
		return NULL;
		}
		
	return *settings;
	}


// from MPlayerApplicationSettingsObserver
// exported function wrapper for internal pure virtual
EXPORT_C void MPlayerApplicationSettingsObserver::DefineAttributeL(TUint aAttributeID,
																	TDesC8& aAttributeText,
																	RArray<TUint>& aValues,
																	RArray<TPtrC8>& aValueTexts,
																	TUint aInitialValue)
	{
	DoDefineAttributeL(aAttributeID, aAttributeText, aValues, aValueTexts, aInitialValue);
	}

EXPORT_C void MPlayerApplicationSettingsObserver::SetAttributeL(TUint aAttributeID, TUint aValue)
	{
	DoSetAttributeL(aAttributeID, aValue );
	}

void CPlayerInfoTarget::DoDefineAttributeL(TUint aAttributeID,
											TDesC8& aAttributeText,
											RArray<TUint>& aValues,
											RArray<TPtrC8>& aValueTexts,
											TUint aInitialValue)
	{
	//Check Length of the player application setting attribute string is 1-255
	if(aAttributeText.Length() > KMaxPlayerApplicationSettingsValue || 
			aAttributeText.Length() < KMinPlayerApplicationSettingsValue )
		{
		User::Leave(KErrNotSupported);
		}
	
	//Check the number of player application setting values is 1-255
	if(aValues.Count() > KMaxPlayerApplicationSettingsValue || 
			aValues.Count() < KMinPlayerApplicationSettingsValue )
		{
		User::Leave(KErrNotSupported);
		}	
	
	//Check the numbers of player application setting values and 
	//player application setting value texts are equal
	if(aValues.Count() != aValueTexts.Count())
		{
		User::Leave(KErrNotSupported);
		}
	
	//Check Length of the player application setting value string is 1-255
	for(TInt i = 0; i < aValueTexts.Count(); i++ )
		{
		if(aValueTexts[i].Length() > KMaxPlayerApplicationSettingsValue ||
				aValueTexts[i].Length() < KMinPlayerApplicationSettingsValue )
			{
			User::Leave (KErrNotSupported);
			}			
		}

	for (TInt i = 0; i < aValues.Count(); i++)
		{
		// The user cannot define certain attribute ids or values; see Appendix F
		if ( ! IsValidAttributeValue(aAttributeID, aValues[i]))
			{
			User::Leave(KErrNotSupported);
			}
		}
	
	// Check the initial value, too
	if ( ! IsValidAttributeValue(aAttributeID, aInitialValue))
		{
		User::Leave(KErrNotSupported);
		}
		
	// check that aInitialValue is in aValues
	if (aValues.Find(aInitialValue) == KErrNotFound)
		{
		User::Leave(KErrNotSupported);
		}
		
	// create a new TPlayerApplicationSettings
	CPlayerApplicationSettings* newSetting = CPlayerApplicationSettings::NewL(aAttributeID, aAttributeText, aValues, aValueTexts, aInitialValue);
	CleanupStack::PushL(newSetting);
	
	// Backup the settings of aAttributeID if they exist, return NULL if the attribute ID cannot be found
	CPlayerApplicationSettings* backupSetting = GetSetting(aAttributeID); 
	
	// and save it 
	iPlayerApplicationSettings.InsertL(aAttributeID, newSetting);
	
	//Delete backupSetting, as the InsertL will replace the old objects by the provided objects 
	delete backupSetting;
	
	CleanupStack::Pop(newSetting);
	}

void CPlayerInfoTarget::DoSetAttributeL(TUint aAttributeID, TUint aValue)
	{
	// Will return NULL if the attribute ID cannot be found
	CPlayerApplicationSettings* setting = GetSetting(aAttributeID);
	if (setting == NULL)
		{
		User::Leave(KErrNotFound);
		}
		
	if ( ! IsValidAttributeValue(aAttributeID, aValue))
		{
		User::Leave(KErrNotSupported);
		}
		
	setting->SetCurrentValue(aValue);
	
	if ( KErrNotFound != iPendingNotificationEventList.Find( ERegisterNotificationPlayerApplicationSettingChanged ))
		{
		// send response
		SendNotificationResponse( ERegisterNotificationPlayerApplicationSettingChanged, ERemConNotifyResponseChanged );
		}
	}

