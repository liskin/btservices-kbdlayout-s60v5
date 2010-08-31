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
* Description:  XML parser implementation
*
*/


// INCLUDE FILES
#include <xml/parser.h>
#include "BTSUXmlParser.h"
#include "BTServiceUtils.h"
#include "BTSUCapabilityResolver.h"
#include "BTSURefObjectResolver.h"
#include "BTSUDebug.h"



// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSUXmlParser::CBTSUXmlParser
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSUXmlParser::CBTSUXmlParser()
    {
    }

// -----------------------------------------------------------------------------
// CBTSUXmlParser::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSUXmlParser::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTSUXmlParser::ConstructL()"));

    User::LeaveIfError( iFileSession.Connect() );    

    FLOG(_L("[BTSU]\t CBTSUXmlParser::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUXmlParser::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSUXmlParser* CBTSUXmlParser::NewL()
    {
    CBTSUXmlParser* self = new( ELeave ) CBTSUXmlParser;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
  
// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CBTSUXmlParser::~CBTSUXmlParser()
    {
    FLOG(_L("[BTSU]\t CBTSUXmlParser::~CBTSUXmlParser()"));

    iFileSession.Close();

    FLOG(_L("[BTSU]\t CBTSUXmlParser::~CBTSUXmlParser() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUXmlParser::GetCapabilityListL
// -----------------------------------------------------------------------------
//
CDesCArrayFlat* CBTSUXmlParser::GetCapabilityListL( const TDesC& /*aFileName */)
    {
    return NULL;
    }
    
// -----------------------------------------------------------------------------
// CBTSUXmlParser::GetCapabilityListL
// -----------------------------------------------------------------------------
//
RArray<TBTSUImageCap>* CBTSUXmlParser::GetImgCapabilityListL( const TDesC& aFileName )
    {
    FLOG(_L("[BTSU]\t CBTSUXmlParser::GetCapabilityListL()"));

    if ( &aFileName == NULL || 
         aFileName.Length() == 0 ||
         aFileName.Length() > KMaxFileName )
        {
        User::Leave( KErrArgument );
        }

    // Create result array
    //
    RArray<TBTSUImageCap>* array = new (ELeave) RArray<TBTSUImageCap>();
    CleanupStack::PushL( array );

    // Set content handler
    //
    CBTSUCapabilityResolver *resolver = CBTSUCapabilityResolver::NewL( array );
    CleanupStack::PushL(resolver);
    // Create parser
    //
    Xml::CParser *parser=Xml::CParser::NewL( _L8( "text/xml" ), *resolver );
    CleanupStack::PushL(parser);
    // Parse    
    //
    Xml::ParseL(*parser, iFileSession, aFileName);
    
    if(!resolver->IsCompleted())
        {
        User::Leave(KErrCompletion);
        }

    
    CleanupStack::PopAndDestroy(2); //parser & resolver  
    CleanupStack::Pop( array );    
    

    FLOG(_L("[BTSU]\t CBTSUXmlParser::GetCapabilityListL() completed"));

    return array;
    }

// -----------------------------------------------------------------------------
// CBTSUXmlParser::GetRefObjectListL
// -----------------------------------------------------------------------------
//
CDesCArrayFlat* CBTSUXmlParser::GetRefObjectListL( const TDesC& aFileName )
    {
    FLOG(_L("[BTSU]\t CBTSUXmlParser::GetRefObjectListL()"));

    if ( &aFileName == NULL || 
         aFileName.Length() == 0 ||
         aFileName.Length() > KMaxFileName )
        {
        User::Leave( KErrArgument );
        }

    // Create result array
    //
    CDesCArrayFlat* array = new (ELeave) CDesCArrayFlat( KBTSUArrayGranularity );
    CleanupStack::PushL( array );

    // Set content handler
    //
    CBTSURefObjectResolver *resolver = CBTSURefObjectResolver::NewL( array );
    CleanupStack::PushL(resolver);
    // Create parser
    //
    Xml::CParser *parser = Xml::CParser::NewL( _L8( "text/xml" ), *resolver );
    CleanupStack::PushL( parser );
    // Parse
    //
    Xml::ParseL(*parser, iFileSession, aFileName);
        
    CleanupStack::PopAndDestroy(2); // parser & resolver  
    CleanupStack::Pop( array );

    FLOG(_L("[BTSU]\t CBTSUXmlParser::GetRefObjectListL() completed"));

    return array;
    }

//  End of File  
