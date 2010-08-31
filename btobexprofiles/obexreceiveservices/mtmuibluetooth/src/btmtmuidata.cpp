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
*      Class CBtMtmUiData
*
*/


// INCLUDE FILES
#include "btmtmuidata.h"
#include "btmtmuidebug.h"
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <mtmdef.hrh>       //kuidmtmquerymaxbodysizevalue etc
#else
#include <mtmdef.hrh>       //kuidmtmquerymaxbodysizevalue etc
#include <mtmuidsdef.hrh>       //kuidmtmquerymaxbodysizevalue etc
#endif
#include <msvuids.h>
#include <obexutilsmessagehandler.h>


// todo @ QT migration: take official definition from Messaging at app layer (btmsgtypeuid.h)
const TUid KUidMsgTypeBt = {0x10009ED5};

// ================= MEMBER FUNCTIONS =======================

// Two-phased constructor.
CBtMtmUiData* CBtMtmUiData::NewL( CRegisteredMtmDll& aRegisteredDll )
	{
    FLOG( _L( " CBtMtmUiData: CBtMtmUiData\t" ) );
	CBtMtmUiData* self = new(ELeave) CBtMtmUiData( aRegisteredDll );
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
	}

// Symbian OS default constructor can leave.
void CBtMtmUiData::ConstructL()
	{
    FLOG( _L( " CBtMtmUiData: ConstructL\t" ) );
    CBaseMtmUiData::ConstructL();
    TObexUtilsMessageHandler::CreateDefaultMtmServiceL(KUidMsgTypeBt);
    FLOG( _L( " CBtMtmUiData: ConstructL done\t" ) );
	}

// C++ default constructor can NOT contain any code, that
// might leave.
//
CBtMtmUiData::CBtMtmUiData(CRegisteredMtmDll& aRegisteredDll)
	:       CBaseMtmUiData(aRegisteredDll)
	{
    FLOG( _L( " CBtMtmUiData: CBtMtmUiData\t" ) );
	}

// Destructor
CBtMtmUiData::~CBtMtmUiData()
	{ 
    FLOG( _L( " CBtMtmUiData: ~CBtMtmUiData\t" ) );
	}

// ---------------------------------------------------------
// OperationSupportedL(...)
// 
// ---------------------------------------------------------
//
TInt CBtMtmUiData::OperationSupportedL( TInt /*aOperationId*/, 
                                       const TMsvEntry& /*aContext*/) const
	{
    FLOG( _L( " CBtMtmUiData: OperationSupportedL\t" ) );
    
    //todo: This was returning avkon resource Id of sting "This item is not available".
    return 0;
	}

// ---------------------------------------------------------
// QueryCapability(...)
// MTM query capability
// ---------------------------------------------------------
//
TInt CBtMtmUiData::QueryCapability( TUid aCapability, 
                                   TInt& aResponse ) const
	{
    FLOG( _L( " CBtMtmUiData: QueryCapability\t" ) );
	switch( aCapability.iUid )
		{
	    // --- Supported valued capabilities ---
	    case KUidMtmQueryMaxBodySizeValue:
            {
		    aResponse = KMaxTInt;
		    break;
            }
	    case KUidMtmQueryMaxTotalMsgSizeValue:
            {
		    aResponse = KMaxTInt;
		    break;
            }
	    case KUidMsvMtmQueryEditorUidValue:
            {
			aResponse = 0;
		    break;
            }
	    case KUidMtmQuerySupportAttachmentsValue:
	    case KUidMtmQueryCanSendMsgValue:
            {
		    break;
            }
	    case KUidMtmQuerySupportedBodyValue:    
	    default:
            {
		    return KErrNotSupported;
            }
		}
	return KErrNone;
	}

// ---------------------------------------------------------
// CBitmapArray& CBtMtmUiData::ContextIcon(...)
// Handles received msg icon.
// ---------------------------------------------------------
//
const CBaseMtmUiData::CBitmapArray& CBtMtmUiData::ContextIcon( const TMsvEntry& /*aContext*/, 
                                                              TInt /*aStateFlags*/) const
	{
	// Check if NFC context
	TInt icon = 0;
	//todo: This function will not work as iIconArrays is not populated might cause panic.
	return *iIconArrays->At(icon); 
	}

// ---------------------------------------------------------
// PopulateArraysL()
// Populates bitmap array 
// ---------------------------------------------------------
//
void CBtMtmUiData::PopulateArraysL()
	{
    FLOG( _L( " CBtMtmUiData: PopulateArraysL\t" ) );
	//todo: Was populating iIconArrays which should be removed.
	}

// ---------------------------------------------------------
// GetResourceFileName(TFileName& aFileName) const
// Gives resource filename.
// ---------------------------------------------------------
//
void CBtMtmUiData::GetResourceFileName( TFileName& /*aFileName*/ ) const
	{ 
    FLOG( _L( " CBtMtmUiData: GetResourceFileName\t" ) );
    //todo: This was returning avkon resource file, which is not required now.
	}

// ---------------------------------------------------------
// CanCreateEntryL(...)
// Checks if it is possible to create entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanCreateEntryL( const TMsvEntry& aParent, 
                                    TMsvEntry& aNewEntry, 
                                    TInt& aReasonResourceId ) const
	{
    FLOG( _L( " CBtMtmUiData: CanCreateEntryL\t" ) );
	aReasonResourceId = 0;
	if( CheckEntry( aNewEntry ) )
		{
        // --- Can create messages in local folders ---
		return ( aParent.iMtm.iUid == KMsvLocalServiceIndexEntryIdValue );
		}
	// --- Can't create other types ---
	//todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0; 
	return EFalse;
	}

// ---------------------------------------------------------
// CanOpenEntryL(...)
// Checks if it is possible to open entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanOpenEntryL( const TMsvEntry& aContext, 
                                  TInt& aReasonResourceId ) const
	{
    FLOG( _L( " CBtMtmUiData: CanOpenEntryL\t" ) );
	if( CheckEntry( aContext ) )
		{
		if( aContext.iType.iUid == KUidMsvMessageEntryValue )
			{
			aReasonResourceId = 0;
			return ETrue;
			}
		}
	//todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0; 
	return EFalse;
	}

// ---------------------------------------------------------
// CanCloseEntryL(...)
// Checks if it is possible to close entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanCloseEntryL( const TMsvEntry& aContext, 
                                   TInt& aReasonResourceId ) const
	{
    FLOG( _L( " CBtMtmUiData: CanCloseEntryL\t" ) );
	if( CheckEntry( aContext ) )
		{
		if( aContext.iType.iUid == KUidMsvServiceEntryValue )
			{
			aReasonResourceId = 0;
			return ETrue;
			}
		}
	//todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0; 
	return EFalse;
	}

// ---------------------------------------------------------
// CanViewEntryL(...)
// Checks if it is possible to view entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanViewEntryL( const TMsvEntry& aContext, 
                                  TInt& aReasonResourceId ) const
	{
    FLOG( _L( " CBtMtmUiData: CanViewEntryL\t" ) );
	if( CheckEntry( aContext ) )
		{
		if( aContext.iType.iUid == KUidMsvMessageEntryValue )
			{
			aReasonResourceId = 0;
			return ETrue;
			}
		}
	//todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0; 
	return EFalse;
	}

// ---------------------------------------------------------
// CanEditEntryL(...)
// Checks if it is possible to edit entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanEditEntryL( const TMsvEntry& aContext, 
                                  TInt& aReasonResourceId ) const
	{
    FLOG( _L( " CBtMtmUiData: CanEditEntryL\t" ) );
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0; 
	if( CheckEntry( aContext ) )
		{
		if ( ( aContext.iType.iUid == KUidMsvMessageEntryValue ) || 
             ( aContext.iType.iUid == KUidMsvFolderEntryValue ) )
			{
			aReasonResourceId=0;
			return ETrue;
			}
		}
	return EFalse;
	}

// ---------------------------------------------------------
// CanDeleteServiceL(...)
// Checks if it is possible to delete service.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanDeleteServiceL( const TMsvEntry& /*aService*/, 
                                      TInt& aReasonResourceId ) const
	{
    FLOG( _L( " CBtMtmUiData: CanDeleteServiceL\t" ) );
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0; 
    return EFalse; 
	}

// ---------------------------------------------------------
// CanDeleteFromEntryL(...)
// Checks if it is possible to delete from entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanDeleteFromEntryL( const TMsvEntry& /*aContext*/, 
                                        TInt& aReasonResourceId ) const
	{
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0;
	return EFalse;
	}

// ---------------------------------------------------------
// CanCopyMoveToEntryL(...)
// Checks if it is possible to move to entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanCopyMoveToEntryL( const TMsvEntry& /*aContext*/, 
                                        TInt& aReasonResourceId ) const
	{
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0; 
	return EFalse;
	}

// ---------------------------------------------------------
// CanCopyMoveFromEntryL(...)
// Checks if it is possible to move from entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanCopyMoveFromEntryL( const TMsvEntry& /*aContext*/,
                                          TInt& aReasonResourceId ) const
	{
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0; 
	return EFalse;
	}

// ---------------------------------------------------------
// CanReplyToEntryL(...)
// Checks if it is possible to reply to entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanReplyToEntryL( const TMsvEntry& /*aContext*/, 
                                     TInt& aReasonResourceId ) const
	{
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0;
	return EFalse;	// Not Ok to do
	}

// ---------------------------------------------------------
// CanForwardEntryL(...)
// Checks if it is possible to forward entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanForwardEntryL( const TMsvEntry& /*aContext*/, 
                                     TInt& aReasonResourceId ) const
	{
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0;
	return EFalse;	// Not Ok to do
	}

// ---------------------------------------------------------
// CheckEntry(...)
// Checks is the entry valid.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CheckEntry( const TMsvEntry& /*aContext*/ ) const
	{
    FLOG( _L( " CBtMtmUiData: CheckEntry\t" ) );
    return EFalse;
	}

// ---------------------------------------------------------
// CanCancelL(...)
// Checks if it is possible to cancel entry.
// ---------------------------------------------------------
//
TBool CBtMtmUiData::CanCancelL( const TMsvEntry& /*aContext*/,
                               TInt& aReasonResourceId ) const
    {
    // No way of sending a message from the outbox, so no need to cancel.
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0;
	return EFalse;
    }

// ---------------------------------------------------------
// StatusTextL(...)
// Returns status text when sending message(outbox).
// ---------------------------------------------------------
//
HBufC* CBtMtmUiData::StatusTextL( const TMsvEntry& /*aContext*/ ) const
    {  
    FLOG( _L( " CBtMtmUiData: StatusTextL\t" ) );
    TBuf<80> buffer;
    //todo: Need to use localised string.
    _LIT(KText, "Sending");
    buffer.Copy(KText);
    return buffer.AllocL();
    }

//  End of File  
