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
*     Class CIrMtmUiData
*
*/


// INCLUDE FILES
#include "irmtmuidata.h"
#include "irmtmui.h"
#include <app/irmsgtypeuid.h>
#include <mtclbase.h>
#include <msvuids.h>
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <mtmdef.hrh> //kuidmtmquerymaxbodysizevalue etc
#else
#include <mtmdef.hrh> //kuidmtmquerymaxbodysizevalue etc
#include <mtmuidsdef.hrh> //kuidmtmquerymaxbodysizevalue etc
#endif
#include <obexutilsmessagehandler.h>
#include "debug.h"

//CONSTANTS
const TInt KIrMtmUiDataMsgEditorAppValue      = 0x00000000;

// ================= MEMBER FUNCTIONS =======================

// Two-phased constructor.
CIrMtmUiData* CIrMtmUiData::NewL(CRegisteredMtmDll& aRegisteredDll)
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: CIrMtmUiData\t" ) );
	CIrMtmUiData* self=new(ELeave) CIrMtmUiData(aRegisteredDll);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

// Symbian OS default constructor can leave.
void CIrMtmUiData::ConstructL()
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: ConstructL\t" ) );
    CBaseMtmUiData::ConstructL();
    TObexUtilsMessageHandler::CreateDefaultMtmServiceL(KUidMsgTypeIrUID);
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: ConstructL done\t" ) );
	}

// C++ default constructor can NOT contain any code, that
// might leave.
//
CIrMtmUiData::CIrMtmUiData(CRegisteredMtmDll& aRegisteredDll)
	:       CBaseMtmUiData(aRegisteredDll)
	{   
	}


// Destructor
CIrMtmUiData::~CIrMtmUiData()
	{ 
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: ~CIrMtmUiData\t" ) );
	}


// ---------------------------------------------------------
// OperationSupportedL(...)
// No Ir-specific operations added to MS menu.
// ---------------------------------------------------------
//
TInt CIrMtmUiData::OperationSupportedL(
    TInt /*aOperationId*/, 
    const TMsvEntry& /*aContext*/) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: OperationSupportedL\t" ) );
    //todo: This was returning avkon resource Id of sting "This item is not available".
    return 0;
	}

// ---------------------------------------------------------
// QueryCapability(...)
// MTM query capability
// ---------------------------------------------------------
//
TInt CIrMtmUiData::QueryCapability(
    TUid aCapability, 
    TInt& aResponse ) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: QueryCapability\t" ) );
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
		    aResponse = KIrMtmUiDataMsgEditorAppValue;
		    break;
            }
        /* Infrared is not supported anymore. Implementation for sending over Infrared is left for reference:
        ( add #include <app/extendedmtmids.hrh> to the included files )
        code starts here:

        case KUidMsvMtmQuerySupportLinks:
            {
            aResponse = ETrue;
            break;
            }

        End of reference implementation for sending over Infrared:
        */
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
// CBaseMtmUiData::CBitmapArray& CIrMtmUiData::ContextIcon(...)
// Handles received msg icon.
// ---------------------------------------------------------
//
const CBaseMtmUiData::CBitmapArray& CIrMtmUiData::ContextIcon(
     const TMsvEntry& /*aContext*/, 
     TInt /*aStateFlags*/) const

	{
	TInt icon = 0;
	//todo: This function will not work as iIconArrays is not populated might cause panic.
	return *iIconArrays->At( icon ); 
	}

// ---------------------------------------------------------
// PopulateArraysL()
// Populates bitmap array 
// ---------------------------------------------------------
//
void CIrMtmUiData::PopulateArraysL()
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: PopulateArraysL\t" ) );

    //todo: Was populating iIconArrays which should be removed.
	}

// ---------------------------------------------------------
// GetResourceFileName(TFileName& aFileName) const
// Gives resource filename.
// ---------------------------------------------------------
//
void CIrMtmUiData::GetResourceFileName(TFileName& /*aFileName*/) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: GetResourceFileName\t" ) );
    //todo: This was returning avkon resource file, which is not required now.
	}

// ---------------------------------------------------------
// CanCreateEntryL(...)
// Checks if it is possible to create entry.
// ---------------------------------------------------------
//
TBool CIrMtmUiData::CanCreateEntryL(
    const TMsvEntry& aParent, 
    TMsvEntry& aNewEntry, 
    TInt& aReasonResourceId ) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: CanCreateEntryL\t" ) );
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
TBool CIrMtmUiData::CanOpenEntryL(
    const TMsvEntry& aContext, 
    TInt& aReasonResourceId ) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: CanOpenEntryL\t" ) );
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
TBool CIrMtmUiData::CanCloseEntryL(
    const TMsvEntry& aContext, 
    TInt& aReasonResourceId ) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: CanCloseEntryL\t" ) );
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
TBool CIrMtmUiData::CanViewEntryL(
    const TMsvEntry& aContext, 
    TInt& aReasonResourceId ) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: CanViewEntryL\t" ) );
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
TBool CIrMtmUiData::CanEditEntryL(
    const TMsvEntry& aContext, 
    TInt& aReasonResourceId ) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: CanEditEntryL\t" ) );
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
TBool CIrMtmUiData::CanDeleteServiceL(
    const TMsvEntry& /*aService*/, 
    TInt& aReasonResourceId ) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: CanDeleteServiceL\t" ) );
    //todo: This was using avkon resource Id of sting "This item is not available".
	aReasonResourceId = 0;
    return EFalse; 
	}

// ---------------------------------------------------------
// CanDeleteFromEntryL(...)
// Checks if it is possible to delete from entry.
// ---------------------------------------------------------
//
TBool CIrMtmUiData::CanDeleteFromEntryL(
    const TMsvEntry& /*aContext*/, 
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
TBool CIrMtmUiData::CanCopyMoveToEntryL(
    const TMsvEntry& /*aContext*/, 
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
TBool CIrMtmUiData::CanCopyMoveFromEntryL(
    const TMsvEntry& /*aContext*/, 
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
TBool CIrMtmUiData::CanReplyToEntryL(
    const TMsvEntry& /*aContext*/, 
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
TBool CIrMtmUiData::CanForwardEntryL(
    const TMsvEntry& /*aContext*/, 
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
TBool CIrMtmUiData::CheckEntry(const TMsvEntry& aContext) const
	{
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: CheckEntry\t" ) );
	return( ( aContext.iType.iUid == KUidMsvMessageEntryValue ) && 
            ( aContext.iMtm       == KUidMsgTypeIrUID ) );
	}

// ---------------------------------------------------------
// CanCancelL(...)
// Checks if it is possible to cancel entry.
// ---------------------------------------------------------
//
TBool CIrMtmUiData::CanCancelL(
    const TMsvEntry& /*aContext*/, TInt& aReasonResourceId ) const
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
HBufC* CIrMtmUiData::StatusTextL( const TMsvEntry& /*aContext*/ ) const
	{  
    FLOG( _L( "[CIrMtmUiData] CIrMtmUiData: StatusTextL\t" ) );
    TBuf<80> buffer;
    //todo: Need to use localised string.
    _LIT(KText, "Sending");
    buffer.Copy(KText);
    HBufC* ptr;
    ptr = buffer.AllocL();
    return ptr;
	}

//  End of File  
