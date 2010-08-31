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
* Description:  Reference object resolver
*
*/


// INCLUDE FILES
#include "BTServiceUtils.h"
#include "BTSURefObjectResolver.h"
#include "BTSUDebug.h"
#include <utf.h>

// CONSTANTS
_LIT8( KBTSUElementImage,    "img" );
_LIT8( KBTSUElementLink,     "link" );
_LIT8( KBTSUElementObject,   "object" );

_LIT8( KBTSUAttributeSrc,    "src" );
_LIT8( KBTSUAttributeData,   "data" );
_LIT8( KBTSUAttributeHref,   "href" );

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSURefObjectResolver* CBTSURefObjectResolver::NewL(CDesCArrayFlat* aCapabilityList)
    {
    CBTSURefObjectResolver* self = new( ELeave ) CBTSURefObjectResolver( aCapabilityList );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop(self);
    return self;
    }
    
// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::CBTSURefObjectResolver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSURefObjectResolver::CBTSURefObjectResolver( 
    CDesCArrayFlat* aObjectList ) : iObjectList( aObjectList )
    {
    }
  
 // -----------------------------------------------------------------------------
// CBTSURefObjectResolver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::ConstructL ()
    {
    }   

 
// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CBTSURefObjectResolver::~CBTSURefObjectResolver()
    {
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnStartDocumentL
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnStartDocumentL(const Xml::RDocumentParameters& /*aDocParam*/, TInt aErrorCode)
    {
    FLOG(_L("[BTSU]\t CBTSURefObjectResolver::OnStartDocumentL()"));
    if(aErrorCode!=KErrNone)
        {
        FLOG(_L("[BTSU]\t Error OnStartDocument"));            
        }
   
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnEndDocumentL
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnEndDocumentL(TInt aErrorCode)
    {
    FLOG(_L("[BTSU]\t CBTSURefObjectResolver::EndDocument()"));
    if(aErrorCode!=KErrNone)
        {
        FLOG(_L("[BTSU]\t Error EndDocument"));            
        }    
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::StartElement
// If a certain element has a certain attribute, append the attribute's
// value into the object list.
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnStartElementL(const Xml::RTagInfo& aElement, const Xml::RAttributeArray& aAttributes, 
								 TInt aErrorCode)    
    {
    
    if(aErrorCode!=KErrNone)
        {
        FLOG(_L("[BTSU]\t Error OnStartElementL"));            
        }     
        
    RString aLocalName = aElement.LocalName();   

    TBuf<KBTSUMaxStringLength> conversionBuf;
    // Images
    //
    if ( aLocalName.DesC() == KBTSUElementImage /*|| aName == KBTSUElementImage()*/ )
    
        {                                                         
        // Check the attributes
        for ( TInt iCounter = 0 ; iCounter < aAttributes.Count() ; iCounter++ )
            {
            if ( aAttributes[iCounter].Attribute().LocalName().DesC() == KBTSUAttributeSrc )
                {                              
                User::LeaveIfError( CnvUtfConverter::ConvertToUnicodeFromUtf8( conversionBuf, aAttributes[iCounter].Value().DesC() )); 
                if(conversionBuf.Length())               
                    iObjectList->AppendL(conversionBuf );
                }
            }                                
           
        }
    else if ( aLocalName.DesC() == KBTSUElementObject() /*|| aName == KBTSUElementObject() */)
        {
        for ( TInt iCounter = 0 ; iCounter < aAttributes.Count() ; iCounter++ )
            {
            if ( aAttributes[iCounter].Attribute().LocalName().DesC() == KBTSUAttributeData )
                {                              
                User::LeaveIfError( CnvUtfConverter::ConvertToUnicodeFromUtf8( conversionBuf, aAttributes[iCounter].Value().DesC() ));   
                if(conversionBuf.Length())                         
                    iObjectList->AppendL(conversionBuf );
                }
            }                    
        }

    // Other objects
    //
    else if ( aLocalName.DesC() == KBTSUElementLink() /*|| aName == KBTSUElementLink() */)
        {
        for ( TInt iCounter = 0 ; iCounter < aAttributes.Count() ; iCounter++ )
            {
            if ( aAttributes[iCounter].Attribute().LocalName().DesC() == KBTSUAttributeHref )
                {                              
                User::LeaveIfError( CnvUtfConverter::ConvertToUnicodeFromUtf8( conversionBuf, aAttributes[iCounter].Value().DesC() ));                
                if(conversionBuf.Length())            
                    iObjectList->AppendL(conversionBuf );
                
                }
            }                      
        }

    }
    
// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnEndElementL
// -----------------------------------------------------------------------------
//    
void CBTSURefObjectResolver::OnEndElementL(const Xml::RTagInfo& /*aElement*/, TInt /*aErrorCode*/)
    {       
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnContentL
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnContentL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
    {   
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnStartPrefixMappingL
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnStartPrefixMappingL(const RString& /*aPrefix*/, const RString& /*aUri*/, 
								   TInt /*aErrorCode*/)
    {    
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnEndPrefixMappingL
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnEndPrefixMappingL(const RString& /*aPrefix*/, TInt /*aErrorCode*/)
    {        
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnIgnorableWhiteSpaceL
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
    {    
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnSkippedEntityL
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnSkippedEntityL(const RString& /*aName*/, TInt /*aErrorCode*/)
    {    
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnProcessingInstructionL
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnProcessingInstructionL(const TDesC8& /*aTarget*/, const TDesC8& /*aData*/, 
										  TInt /*aErrorCode*/)
    {    
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::OnError
// -----------------------------------------------------------------------------
//
void CBTSURefObjectResolver::OnError(TInt /*aErrorCode*/)
    {    
    }

// -----------------------------------------------------------------------------
// CBTSURefObjectResolver::GetExtendedInterface
// -----------------------------------------------------------------------------
//
TAny* CBTSURefObjectResolver::GetExtendedInterface(const TInt32 /*aUid*/)
    {
 
    return NULL;   
    }

//  End of File  
