/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of CBIPImageHandler class
*
*/


#define __OBEX_USER_DEFINED_HEADERS__

// INCLUDE FILES
#include "BIPImageHandler.h"
#include <obexheaders.h>

const TInt KImageHandleWidth=7;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// AddImageHandleHeaderL()
// ---------------------------------------------------------
//
void CBIPImageHandler::AddImageHandleHeaderL( CObexServer* aObexServer )
    {
    TRACE_FUNC_ENTRY    
    
    CObexHeaderSet* currentHeaders = CObexHeaderSet::NewL();
    CleanupStack::PushL( currentHeaders );  // 1. push
        
    CObexHeader* header = CObexHeader::NewL();
    CleanupStack::PushL( header ); //2. push

    TBuf16<KImageHandleWidth> imageHandle;
    iImageHandleValue++;
    imageHandle.AppendNumFixedWidth( iImageHandleValue, EDecimal, KImageHandleWidth ); 
    header->SetUnicodeL( 0x30, imageHandle );

    User::LeaveIfError( currentHeaders->AddHeader( header ) );
    
    CleanupStack::Pop( header); 
    if( aObexServer )
        {
        TRACE_INFO( _L( "[MtmUiServerBip] CBIPImageHandler: SetPutFinalResponseHeaders \t" ) );
        User::LeaveIfError( aObexServer->SetPutFinalResponseHeaders( currentHeaders ) );
        CleanupStack::Pop( currentHeaders);     
        }
    else
        {
        CleanupStack::PopAndDestroy(currentHeaders);    
        }
    
    TRACE_FUNC_ENTRY    
    }

// ---------------------------------------------------------
// CBIPImageHandler()
// ---------------------------------------------------------
//
CBIPImageHandler::CBIPImageHandler()
    {
    }

// ---------------------------------------------------------
// ~CBIPImageHandler()
// ---------------------------------------------------------
//
CBIPImageHandler::~CBIPImageHandler()
    {
    TRACE_FUNC_ENTRY    
    }
// ---------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------
//
void CBIPImageHandler::ConstructL()
    {
    TRACE_FUNC
    iImageHandleValue = 0;
    }

// -----------------------------------------------------------------------------
// NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBIPImageHandler* CBIPImageHandler::NewL()
    {
    TRACE_FUNC_ENTRY
    CBIPImageHandler* self = new ( ELeave ) CBIPImageHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return( self );
    }

//  End of File
