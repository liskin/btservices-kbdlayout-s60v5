/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Helper class for SDP database management.
*
*/



#include <btsdp.h>
#include <btengsdp.rsg>
#include <barsread.h>
#include <barsc.h>
#include <data_caging_path_literals.hrh>

#include "btengsdpdbhandler.h"
#include "btengsdp.hrh"
#include "debug.h"

_LIT( KBTEngSdpResourceFile, "btengsdp.rsc");
_LIT( KDriveZ, "z:");
_LIT8( KHex, "0x");
const TInt KMaxServiceDesLength = 64;
const TInt KNumberOfChars = 8;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSdpDbHandler::CBTEngSdpDbHandler()
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    User::LeaveIfError( iSdp.Connect() );
    User::LeaveIfError( iDb.Open( iSdp ) );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSdpDbHandler* CBTEngSdpDbHandler::NewL()
    {
    CBTEngSdpDbHandler* self = new( ELeave ) CBTEngSdpDbHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSdpDbHandler::~CBTEngSdpDbHandler()
    {
    iDb.Close();     // This should have no effect if the handle is null.
    iSdp.Close();
    }


// ---------------------------------------------------------------------------
// Register SDP record for the specified service containing the specified 
// protocol channel in the protocol descriptor list.
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::RegisterSdpRecordL( const TUUID& aService, 
    TInt aChannel, TSdpServRecordHandle& aHandle )
    {
    TRACE_FUNC_ARG( ( _L( "channel: %d" ), aChannel ) )
    iChannel = aChannel;
    RegisterSdpRecordL( aService, aHandle );
    }


// ---------------------------------------------------------------------------
// Register SDP record for the specified service containing the specified 
// VendorID and ProductID for the relevant attributes (mainly for DI profile).
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::RegisterSdpRecordL( const TUUID& aService, 
    TInt aVendorId, TInt aProductId, TSdpServRecordHandle& aHandle )
    {
    TRACE_FUNC_ARG( ( _L( "vendor ID: %d: product ID: %d" ), 
                          aVendorId, aProductId ) )
    iVendorId = aVendorId;
    iProductId = aProductId;
    RegisterSdpRecordL( aService, aHandle );
    }


// ---------------------------------------------------------------------------
// Register SDP record for the specified service.
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::RegisterSdpRecordL( const TUUID& aService, 
    TSdpServRecordHandle& aHandle )
    {
    TResourceReader reader;
    HBufC8* record = NULL;
    TBuf8<KMaxServiceDesLength> serviceBuf( KHex );
    TPtrC8 ptr = aService.ShortestForm();
    if( ptr.Length() <= 4 )
        {
            // Short form UUID (16 or 32 bytes).
        TUint service = SdpUtil::GetUint( ptr );
        serviceBuf.AppendNum( service, EHex );
        }
    else
        {
            // Long form UUID (128 bytes).
        TUint64 serviceLo = 0;
        TUint64 serviceHi = 0;
        SdpUtil::GetUint128( ptr, serviceLo, serviceHi );
            // The numbers need to have a fixed width (including leading zeros)
            // and AppendNumFixedWidth is only avaliable with TUint32.
        serviceBuf.AppendNumFixedWidth( I64HIGH(serviceHi), EHex, KNumberOfChars );
        serviceBuf.AppendNumFixedWidth( I64LOW(serviceHi), EHex, KNumberOfChars );
        serviceBuf.AppendNumFixedWidth( I64HIGH(serviceLo), EHex, KNumberOfChars );
        serviceBuf.AppendNumFixedWidth( I64LOW(serviceLo), EHex, KNumberOfChars );
        }

        // Read the record from resource file.
    ReadRecordResourceL( serviceBuf, reader, record );
    CleanupStack::PushL( record );

    iDb.CreateServiceRecordL( TUUID( 0 ), aHandle );
    CSdpAttrValue* attrVal = NULL;
    TUint attrCount = reader.ReadUint16();
    for( TInt i = 0; i < attrCount; i++ )
        {
        TUint16 attrId = reader.ReadUint16();
        BuildAttributeLC( attrVal, reader, attrId );
        if( attrVal )
            {
            iDb.UpdateAttributeL( aHandle, attrId, *attrVal );
            CleanupStack::PopAndDestroy( attrVal );
            attrVal = NULL;
            }
        }

    CleanupStack::PopAndDestroy( record );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Delete the SDP record for the specified record handle.
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::DeleteSdpRecordL( const TSdpServRecordHandle aHandle )
    {
    TRACE_FUNC_ENTRY
    if( aHandle )
        {
        iDb.DeleteRecordL( aHandle );
        }
    else
        {
        User::Leave( KErrBadHandle );
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Build an SDP attribute.
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::BuildAttributeLC( CSdpAttrValue*& aAttrVal,
    TResourceReader& aReader, TInt aAttrId )
    {
        // Read the attribute type and ID
    TUint attrType = aReader.ReadUint8();
    if( attrType == EElemTypeList )
        {
        BuildAttrDesLC( aAttrVal, aReader, aAttrId );
        }
    else
        {
        BuildAttrValueLC( aAttrVal, aReader, attrType, aAttrId );
        }
    }


// ---------------------------------------------------------------------------
// Build an SDP attribute for a concrete attribute type.
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::BuildAttrValueLC( CSdpAttrValue*& aAttrVal,
    TResourceReader& aReader, TUint aAttrType, TInt aAttrId )
    {
    TRACE_FUNC_ENTRY
    switch( aAttrType )
        {
        case EElemTypeWord:
            {
            TSdpIntBuf<TUint16> buf( aReader.ReadUint16() );
            if( iVendorId && aAttrId == EVendorID )
                {
                buf = TSdpIntBuf<TUint16>( iVendorId );
                iVendorId = 0;   // Reset vendor ID
                }
            if( iProductId && aAttrId == EProductID )
                {
                buf = TSdpIntBuf<TUint16>( iProductId );
                iProductId = 0;   // Reset vendor ID
                }
            aAttrVal = CSdpAttrValueUint::NewUintL( buf );
            }
            break;
        case EElemTypeLong:
            {
            TSdpIntBuf<TUint32> buf( aReader.ReadUint32() );
            aAttrVal = CSdpAttrValueUint::NewUintL( buf );
            }
            break;
        case EElemTypeUUID:
            {
            TUUID uuid( aReader.ReadUint32() );
            aAttrVal = CSdpAttrValueUUID::NewUUIDL( uuid );
            }
            break;
        case EElemTypeUUID128:
            {
            TUUID uuid;
            uuid.SetL( aReader.ReadTPtrC8() );
            aAttrVal = CSdpAttrValueUUID::NewUUIDL( uuid );
            }
            break;
        case EElemTypeText:
            {
            TPtrC8 ptr = aReader.ReadTPtrC8();
            aAttrVal = CSdpAttrValueString::NewStringL( ptr );
            }
            break;
        case EElemTypeByte:
            {
            TSdpIntBuf<TUint8> buf( aReader.ReadUint8() );
            if( iChannel && aAttrId == EProtocolDescriptorList )
                {
                buf = TSdpIntBuf<TUint8>( iChannel );
                iChannel = 0;   // Reset channel number
                }
            aAttrVal = CSdpAttrValueUint::NewUintL( buf );
            }
            break;
        case EElemTypeList:
            {
            BuildAttrDesLC( aAttrVal, aReader, aAttrId );
            }
            break;    
        case EElemTypeLong64:
            {
            TPtrC8 ptr = aReader.ReadTPtrC8();
            aAttrVal = CSdpAttrValueUint::NewUintL(ptr);
            break;
            }
        case EElemTypeBool:
            {
            TBool boolVal = (TBool) aReader.ReadUint8();
            aAttrVal = CSdpAttrValueBoolean::NewBoolL( boolVal );
            break;
            }
        case EElemTypeLink:
        default:
            break;
        }
    if( aAttrVal )
        {
        CleanupStack::PushL( aAttrVal );
        }
    }


// ---------------------------------------------------------------------------
// Build an SDP Data Elelement Sequence attribute (i.e. a sequence of 
// concrete attributes or lists).
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::BuildAttrDesLC( CSdpAttrValue*& aAttrVal, 
    TResourceReader& aReader, TInt aAttrId )
    {
    TRACE_FUNC_ENTRY
    TUint elementCount = aReader.ReadUint16();
    MSdpElementBuilder* builder = NULL;
    if( !aAttrVal )
        {
            // This is the first element of the attribute.
        aAttrVal = CSdpAttrValueDES::NewDESL( NULL );
        builder = ( (CSdpAttrValueDES*) aAttrVal )->StartListL();
        }
    else
        {
            // This is a nested DES. This means that the parent element is
            // also a DES. Append this DES to the parent.
        builder = ( (CSdpAttrValueDES*) aAttrVal )->BuildDESL();
        builder = builder->StartListL();
        }

    CleanupStack::PushL( aAttrVal );
    while( elementCount > 0 )
        {
            // Build the list; this can result in another call to this function 
            // to build a nested list.
        CSdpAttrValue* element = NULL;
        BuildAttributeLC( element, aReader, aAttrId );
        if( element )
            {
            CleanupStack::Pop( element );   // Ownership will be passed to DES.
            CSdpAttrValueDES* list = (CSdpAttrValueDES*) builder;
            list->AppendValueL( element );
            }
        elementCount--;
        }
    builder = builder->EndListL();
    }


// ---------------------------------------------------------------------------
// Read the service record from the resource file containing all BT Engine's
// service record definitions.
// ---------------------------------------------------------------------------
//
void CBTEngSdpDbHandler::ReadRecordResourceL( const TDesC8& aService, 
    TResourceReader& aReader, HBufC8*& aRecordBuf )
    {
    TRACE_FUNC_ENTRY
        // Find and open resource file
    TFileName fileName( KDriveZ );
    fileName.Append( KDC_RESOURCE_FILES_DIR );
    fileName.Append( KBTEngSdpResourceFile );
    RFs fsSession;
    User::LeaveIfError( fsSession.Connect() );
    CleanupClosePushL( fsSession );
    RResourceFile resourceFile;
    resourceFile.OpenL( fsSession, fileName );
    CleanupClosePushL( resourceFile );

        // Read the array containing the mappings of UUID to resource definitions.
    HBufC8* buf = resourceFile.AllocReadLC( R_SERVICE_RECORD_LIST );
    aReader.SetBuffer( buf );
    CDesC8ArrayFlat* serviceIdArray = aReader.ReadDesC8ArrayL();
    CleanupStack::PushL( serviceIdArray );

        // Find the requested service record.
    TInt pos = KErrNotFound;
    for( TInt i = 0; i < serviceIdArray->Count(); i++ )
        {
            // CompareF is case insensitive comparison
        if( aService.CompareF( (*serviceIdArray)[i] ) == 0 )
            {
            pos = i;
            break;
            }
        }
    if( pos < 0 )
        {
            // The record is not found from the resource file.
        User::Leave( pos );
        }

        // Read the service record resource in a buffer to pass back.
    aReader.Advance( 2 * pos + 2 ); // 16bit LINK
    TUint resourceId = aReader.ReadUint16();
    aRecordBuf = resourceFile.AllocReadL( resourceId );
    aReader.SetBuffer( aRecordBuf );

    CleanupStack::PopAndDestroy( serviceIdArray );
    CleanupStack::PopAndDestroy( buf );
    CleanupStack::PopAndDestroy( &resourceFile );
    CleanupStack::PopAndDestroy( &fsSession );
    TRACE_FUNC_EXIT
    }
