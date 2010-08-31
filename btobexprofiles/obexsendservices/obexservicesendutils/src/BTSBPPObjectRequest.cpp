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
* Description:  GetReferencedObjects -operation implementation
*
*/



// INCLUDE FILES
#include "BTServiceUtils.h"
#include "BTSBPPObjectRequest.h"
#include "BTSUDataConverter.h"
#include "BTSUImageConverter.h"
#include "BTSUDebug.h"

#include <f32file.h>    // RFile
#include <s32mem.h>     // RBufWriteStream
#include <s32file.h>    // RFileReadStream

// CONSTANTS
const TInt KBTSBPPFileSizeNotSet         = 0;
const TInt KBTSBPPFileSizeRequested      = 1;
const TInt KBTSBPPFileSizeUnknown        = -1;
const TInt KBTSBPPRestOfTheFileRequested = -1;

// The field lengths of Application Parameters -header in bytes
//
const TInt KBTSBPPFieldTag       = 1;
const TInt KBTSBPPFieldLength    = 1;
const TInt KBTSBPPValue          = 4;
const TInt KBTSBPPAppParamLength = KBTSBPPFieldTag + KBTSBPPFieldLength + KBTSBPPValue;

_LIT8(KBTSBPPRefObjectOper, "x-obex/referencedobject\0");

static const TUint8 KBTSBPPFileSizeTag = 0x04;
static const TUint8 KBTSBPPFileSizeLength = 0x04;

// MODULE DATA STRUCTURES
enum KBTSBPPAppParamTags
    {
    KBTSBPPTagOffset = 1,
    KBTSBPPTagCount,
    KBTSBPPTagJobId, // used only in JobBased-printing
    KBTSBPPTagFileSize
    };

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::CBTSBPPObjectRequest
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSBPPObjectRequest::CBTSBPPObjectRequest( CObexBaseObject* aGetOperation,
    const CDesCArray* aRefObjectList ) : iRequest( aGetOperation ), 
                                         iObjectList( aRefObjectList )
    {
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::ConstructL()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::ConstructL()"));

    User::LeaveIfError( iFileSession.Connect() );

    // Check the validity of the operation and it's paramters.
    //
    CheckGetRequestL();

    // Execute any tasks needed and create the requested object.
    //
    ExecuteGetRequestL();

    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSBPPObjectRequest* CBTSBPPObjectRequest::NewL( CObexBaseObject* aGetOperation,
                                              const CDesCArray* aRefObjectList )
    {
    CBTSBPPObjectRequest* self = new( ELeave ) CBTSBPPObjectRequest( aGetOperation,
                                                                 aRefObjectList );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

    
// Destructor
CBTSBPPObjectRequest::~CBTSBPPObjectRequest()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::~CBTSBPPObjectRequest()"));
    iFileSession.Close();

    delete iResponse;
    delete iResponseBuffer;

    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::~CBTSBPPObjectRequest() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::GetResponse
// -----------------------------------------------------------------------------
//
CObexBufObject* CBTSBPPObjectRequest::GetResponse()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::GetResponse()"));
    return iResponse;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::CheckGetRequestL
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::CheckGetRequestL()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckGetRequestL()"));

    CheckTypeHeaderL();
    CheckNameHeaderL();
    CheckAppParamsHeaderL();
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::CheckTypeHeaderL
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::CheckTypeHeaderL()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckTypeHeaderL()"));

    __ASSERT_DEBUG( iRequest, BTSUPanic( EBTSUPanicNullPointer ) );

    const TDesC8& typeHeader = iRequest->Type();

    if ( typeHeader.Length() == 0 )
        {
        FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckTypeHeaderL() ERROR, no type header"));
        User::Leave( KErrNotFound );
        }
    else if ( typeHeader.Length() > KBTSUMaxStringLength )
        {
        FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckTypeHeaderL() ERROR, type header too long"));
        User::Leave( KErrTooBig );
        }
    else
        {
        FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckTypeHeaderL() type header '%S'"), &typeHeader ));
        // The only supported operation is GetReferencedObjects
        //
        if ( typeHeader != KBTSBPPRefObjectOper() )
            {
            FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckTypeHeaderL() ERROR, unsupported operation"));
            User::Leave( KErrNotSupported );
            }
        }

    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckTypeHeaderL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::CheckNameHeaderL
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::CheckNameHeaderL()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckNameHeaderL()"));

    __ASSERT_DEBUG( iRequest, BTSUPanic( EBTSUPanicNullPointer ) );

    const TDesC& nameHeader = iRequest->Name();

    if ( nameHeader.Length() == 0 )
        {
        FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckNameHeaderL() ERROR, no name header"));
        User::Leave( KErrNotFound );
        }
    else
        {
        FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckNameHeaderL() filename '%S'"), &nameHeader ));

        // Check are we allowed to send this file
        //
        TInt dummy;
        if ( iObjectList->Find( nameHeader, dummy ) != KBTSUEqualStrings )
            {
            FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckNameHeaderL() ERROR, the asked object is not referenced in the document"));
            User::Leave( KErrPermissionDenied );
            }

        iRequestParams.iName = &nameHeader;
        }
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::CheckAppParamsHeaderL
//
// This method expects a Tag-Length-Value format. The method is as defensive 
// as possible, for example unknown or illegal values are discarded.
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::CheckAppParamsHeaderL()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckAppParamsHeaderL()"));

    __ASSERT_DEBUG( iRequest, BTSUPanic( EBTSUPanicNullPointer ) );

    TPtrC8 header( iRequest->AppParam() );    

    if ( header.Length() == 0 )
        {
        FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckAppParamsHeaderL() ERROR, no AppParam header"));
        User::Leave( KErrNotFound );
        }
    else if ( header.Length() < KBTSBPPAppParamLength * 2)
        {
        // There has to be at least offset and count -parameters.
        //
        FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckAppParamsHeaderL() ERROR, incomplete AppParam header"));
        User::Leave( KErrArgument );
        }
    else
        {
        // Parse the header
        //
        while( header.Length() >= KBTSBPPAppParamLength )
            {
            // Extract tag number and remove it from the header
            //
            TInt tagNumber = TBTSUDataConverter::ConvertByteL(
                header.Left( KBTSBPPFieldTag ) );
            header.Set( header.Mid( KBTSBPPFieldTag ) );

            // Extract value length and remove it from the header
            //
            TInt valueLength = TBTSUDataConverter::ConvertByteL( 
                header.Left( KBTSBPPFieldLength ) );
            header.Set( header.Mid( KBTSBPPFieldLength ) );

            if ( valueLength != KBTSBPPValue )
                {
                FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckAppParamsHeaderL() WARNING, illegal length %d"), valueLength ));
                valueLength = KBTSBPPValue;
                // This is an interesting choice of what to do, I would argue that for future compatibility you'll probably want to accept the length
                // provided as it might be for a new tag that is not yet published.  (Checking of the size for known tags is handled by the conversion
                // functions anyway).
                // So, in summary I wouldn't change valueLength from what it is in the payload.  But I've left it in for now in case it was added for
                // interoperability reasons with some existing device.
                }

            // Extract value...
            //
            switch ( tagNumber )
                {
                case KBTSBPPTagOffset:
                    {
                    iRequestParams.iOffset = 
                        TBTSUDataConverter::ConvertDataUnsignedL( 
                        header.Left( valueLength ) );

                    FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckAppParamsHeaderL() offset '%d'"), iRequestParams.iOffset ));
                    break;
                    }
                case KBTSBPPTagCount:
                    {
                    iRequestParams.iCount = 
                        TBTSUDataConverter::ConvertDataSignedL( 
                        header.Left( valueLength ) );

                    FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckAppParamsHeaderL() count '%d'"), iRequestParams.iCount ));
                    break;
                    }
                case KBTSBPPTagFileSize:
                    {
                    // The value is not interesting, we just need to know whether 
                    // the parameter was received (and thus requested)
                    //
                    iRequestParams.iOffset = KBTSBPPFileSizeRequested;
                    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::ParseHeadersL() fileSize parameter received"));
                    break;
                    }

                case KBTSBPPTagJobId:
                default:
                    {
                    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckAppParamsHeaderL() WARNING, illegal tag"));
                    break;
                    }
                }

            // ...and remove it from the header
            //
            header.Set( header.Mid( valueLength ) );
            }
        }

    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CheckAppParamsHeaderL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::ExecuteGetRequestL
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::ExecuteGetRequestL()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::ExecuteGetRequestL()"));

    // If iCount is 0, an empty body will be sent
    //
    if ( iRequestParams.iCount != 0 )
        {
        ResolveGetRequestL();
        ReadFileToBufferL();      
        }

    CreateResponseObjectL();

    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::ExecuteGetRequestL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::ResolveGetRequestL
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::ResolveGetRequestL()
    {
    FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::ResolveGetRequestL()"));

    // Get the file size
    //
    RFile file;
    TInt size = 0;
    User::LeaveIfError( file.Open( iFileSession, *iRequestParams.iName, 
                                   EFileShareReadersOnly | EFileStream ) );

    TInt error = file.Size( size );
    
    if ( error )
        {
        FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::ResolveGetRequestL() error %d in getting the size"), error ) );
        size = KBTSBPPFileSizeUnknown;
        }

    file.Close();

    // Store the file size if it was requested
    //
    if ( iRequestParams.iFileSize == KBTSBPPFileSizeRequested )
        {
        iRequestParams.iFileSize = size;
        }

    // Resolve the actual amount of data that needs to be read.
    // 
    TInt dataLeft = size - iRequestParams.iOffset;

    if ( iRequestParams.iCount == KBTSBPPRestOfTheFileRequested )
        {
        if ( size == KBTSBPPFileSizeUnknown )
            {
            // The size of the file is unknown, so read as much 
            // as would fit into the buffer
            //
            iRequestParams.iCount = KBTSUDataBufferMaxSize;
            }        
        else
            {
            // The rest of the file is requested, so correct the count
            //
            iRequestParams.iCount = dataLeft;
            }
        }
    else if ( iRequestParams.iCount > dataLeft )
        {
        // There is less data left that was requested, so correct the count
        //
        iRequestParams.iCount = dataLeft;
        }

    if ( iRequestParams.iCount > KBTSUDataBufferMaxSize )
        {
        // The requested count is too big
        //
        FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::ResolveGetRequestL() iCount too big %d"), iRequestParams.iCount ) );
        User::Leave( KErrTooBig );
        }

    FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::ResolveGetRequestL() completed, requested %d bytes"), iRequestParams.iCount ) );
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::ReadFileToBufferL
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::ReadFileToBufferL()
    {
    __ASSERT_DEBUG( !iResponseBuffer, BTSUPanic( EBTSUPanicExistingObject ) );

    // Create a buffer for the object and reserve space according to the request
    //
    iResponseBuffer = CBufFlat::NewL( KBTSUDataBufferExpandSize );
    iResponseBuffer->SetReserveL( iRequestParams.iCount );
    iResponseBuffer->ResizeL( iRequestParams.iCount );

    // Open the file
    //
    RFile file;
    User::LeaveIfError( file.Open( iFileSession, *iRequestParams.iName, 
                                   EFileShareReadersOnly | EFileStream ) );
    CleanupClosePushL( file );

    // Create a stream for reading from the file
    //
    RFileReadStream readStream( file, iRequestParams.iOffset );

    // Create a stream for writing into the buffer
    //
    RBufWriteStream writeStream( *iResponseBuffer );

    // Read the data from file to the buffer
    //
    readStream.ReadL( writeStream, iRequestParams.iCount );

    FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::ReadFileToBufferL() space reserved '%d'"), iRequestParams.iCount ));
    FTRACE(FPrint(_L("[BTSBPP]\t CBTSBPPObjectRequest::ReadFileToBufferL() data read '%d'"), iResponseBuffer->Size() ));

    CleanupStack::PopAndDestroy(&file);
    }


// -----------------------------------------------------------------------------
// CBTSBPPObjectRequest::CreateResponseObjectL
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectRequest::CreateResponseObjectL()
    {
    __ASSERT_DEBUG( iResponseBuffer || (iRequestParams.iCount == 0), BTSUPanic( EBTSUPanicNoBufferEvenThoughCountNotZero ) );
    __ASSERT_DEBUG( !iResponse, BTSUPanic( EBTSUPanicResponseAlreadyPresent ) );
    
    // Create the OBEX response object using the buffer already created (or no buffer if appropriate)
    //
    iResponse = CObexBufObject::NewL( iResponseBuffer );
    
    // Generate FileSize parameter if requested
    //
    if ( iRequestParams.iFileSize != KBTSBPPFileSizeNotSet )
        {
        FLOG(_L("[BTSBPP]\t CBTSBPPObjectRequest::CreateResponseObjectL() generate file size param"));
        TBuf8<KBTSBPPAppParamLength> params;
        params.SetMax();
        TUint8* ptr = const_cast<TUint8*>(params.Ptr());
        *ptr++ = KBTSBPPFileSizeTag;
        *ptr++ = KBTSBPPFileSizeLength;
        BigEndian::Put32(ptr, *reinterpret_cast<TUint32*>(&iRequestParams.iFileSize)); // reinterpret cast to retain signed nature...
        iResponse->SetAppParamL( params );
        }
    }

//  End of File
