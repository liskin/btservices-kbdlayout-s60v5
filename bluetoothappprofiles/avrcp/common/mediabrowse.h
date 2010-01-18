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
 @internalTechnology
 @released
*/

#ifndef MEDIABROWSE_H
#define MEDIABROWSE_H

#include <e32base.h>
#include <remconmediainformationtargetobserver.h>
#include <remconmediabrowsetypes.h>
#include "avrcpipc.h"
#include "browsingframe.h"
#include "mediainformation.h"

//-------------------------------------------------------------------------- 
// Interface Data
//--------------------------------------------------------------------------

enum TRemConMediaBrowseOperationId
	{
	EGetFolderItemsOperationId,
	EChangePathOperationId,
	EGetItemAttributesOperationId,
	ESearchOperationId,
	ESetMaxResponseOperationId,
	ESetBrowsedPlayerOperationId,
	EUIDPersistency,
	EMediaLibraryStateCookieUpdateOperationId,
	};

/**
 * The UID identifying the RemCon NowPlaying interface.
 */
const TInt KRemConMediaBrowseApiUid = 0x10285bbb;

// identifier (8bytes) NumAttributes (1) AttributeID (max is 4*7) = 37 + UID Counter + scope
const TInt KMaxLengthMediaBrowseMsg = 400;

//PDU ID(1 byte) + Parameter length(2 bytes) + Status(1 byte), 
//the total of which is 4 bytes
const TInt KBrowseResponseBaseLength = 4;

//KBrowseResponseBaseLength(4 bytes) + UID Counter(2 bytes) + Number of Items(2 bytes)
//the total of which is 8
const TInt KGetFolderItemsResponseBaseSize = 8;

//UID 8 + FolderType 1 + Is Playable 1 + Character set ID 2 + Name length 2
const TInt KFolderItemBaseLength = 14;

//UID 8 + Type 1 + Character Set 2 + Name Length 2 + Number of attributes 1 
const TInt KMediaElementItemBaseLength = 14;

//ID 4 + Character Set 2 + Length filed 2
const TInt KAttributeBaseLength = 8;

//Status 1
const TInt KErrResponseParameterLength = 1;

/**
This is the max length of a response used in media browse API for fix-length responses. 
For the response which have a fix-length the Search(PDU 0x80) has the maximum response 
which is KBrowseResponseBaseLength(4 bytes)+ UID Counter(2 bytes) + Number of Items(4 bytes) 
the total of which is 10
*/
const TInt KMediaBrowseOutBufMaxLength = 10;

class RItem
	{
public:
	IMPORT_C void Close();
	IMPORT_C TInt Size();
	
public:
	AvrcpBrowsing::TItemType	iType;
	TUint16						iLength;
	TUint64						iUid;
	
	TUint16						iCharset;
	TUint16						iNameLength;
	HBufC8*						iName;
	
	// Folder attributes
	AvrcpBrowsing::TFolderType	iFolderType;
	AvrcpBrowsing::TPlayable	iPlayable;
	
	// Media element attributes
	AvrcpBrowsing::TMediaType	iMediaType;
	TInt						iNumberAttributes;
	RArray<REAResponse>			iAttributes;
	};

//--------------------------------------------------
// GetFolderItems 0x71
//--------------------------------------------------
class RRemConGetFolderItemsRequest : public RAvrcpIPC
	{
public:
	AvrcpBrowsing::TScope	iScope;
	TUint					iStartItem;
	TUint					iEndItem;
	TInt 					iNumberAttributes;
	RArray<TInt>			iAttributes;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C void Close();
	IMPORT_C TInt CopyAttributes(RArray<TMediaAttributeId>& aOutAttributes);
	};

class RRemConGetFolderItemsResponse : public RAvrcpIPC
	{
public:
	TUint8					iPduId;
	TUint16					iParamLength;
	TUint8					iStatus;
	TUint16					iUidCounter;
	TUint16 				iNumberItems;
	RArray<RItem>			iItems;
	
	TInt 						iMaxResponse;		
	TInt 						iCurrentItem;
	TInt 						iCurrentListingSize;
	
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C TInt Size();
	IMPORT_C void Close();
	IMPORT_C TInt CopyItems(const TArray<TRemConItem>& aItems);
	IMPORT_C TBool RequestNextItem(TInt& aError, RBuf8& aOutBuf, TUint16 aCookie);
	
private:
	void ReadItemL();
	void WriteItemL(TInt aIndex);
	};

//--------------------------------------------------
// ChangePath 0x72
//--------------------------------------------------
class RRemConChangePathRequest : public RAvrcpIPC
	{
public:
	TUint16 					iUidCounter;
	AvrcpBrowsing::TDirection	iDirection;
	TUint64 					iElement;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};


class RRemConChangePathResponse : public RAvrcpIPC
	{
public:
	TUint8					iPduId;
	TUint16					iParamLength;
	TUint8					iStatus;
	TInt 					iNumberItems;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

//--------------------------------------------------
// GetItemAttributes 0x73
//--------------------------------------------------
class RRemConGetItemAttributesRequest : public RAvrcpIPC
	{
public:
	AvrcpBrowsing::TScope	iScope;
	TUint64 				iElement;
	TUint16					iUidCounter;
	TInt 					iNumberAttributes;
	RArray<TInt>			iAttributes;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C void Close();
	};


class RRemConGetItemAttributesResponse : public RAvrcpIPC
	{
public:
	TUint8					iPduId;
	TUint16					iParamLength;
	TUint8					iStatus;
	TInt 					iNumberAttributes;
	RArray<REAResponse>		iAttributes;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C TInt Size();
	IMPORT_C void Close();
	};

//--------------------------------------------------
// Search 0x80
//--------------------------------------------------
class RRemConSearchRequest : public RAvrcpIPC
	{
public:
	TUint16 iCharset;
	TUint16 iStringLen;
	RBuf8 iSearchString;
 
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C TInt Size();
	IMPORT_C void Close();
	};

class RRemConSearchResponse : public RAvrcpIPC
	{
public:
	TUint8					iPduId;
	TUint16					iParamLength;
	TUint8					iStatus;
	TUint16					iUidCounter;
	TUint					iNumberItems;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

//--------------------------------------------------
// SetBrowsedPlayer
//--------------------------------------------------
class RRemConSetBrowsedPlayerRequest : public RAvrcpIPC
	{
public:
	TInt 	iMaxResponse;
	TUint16 iPlayerId;
	
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

class RRemConGetPathResponse : public RAvrcpIPC
	{
public:
	TUint8					iPduId;
	TUint16					iParamLength;
	TUint8					iStatus;
	TUint16					iUidCounter;
	TUint					iNumberItems;
	TUint16					iCharset;
	TUint8					iFolderDepth;
	RPointerArray<HBufC8>	iPath;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C TInt Size();
	IMPORT_C void Close();
	};

class RRemConMediaErrorResponse : public RAvrcpIPC
	{
public:
	TUint8  iPduId;
	TUint16 iLength;
	TUint8 	iStatus;
	
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

class RRemConUidsChangedRequest : public RAvrcpIPC
	{
public:
	TUint16	iInitialUidCounter;
	
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

class RRemConUidsChangedResponse : public RAvrcpIPC
	{
public:
	TUint16      iUidCounter;
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};
#endif //MEDIABROWSE_H
