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
* Description:  Capability resolver implementation
*
*/


// INCLUDE FILES
#include "BTServiceUtils.h"
#include "BTSUCapabilityResolver.h"
#include "BTSUDebug.h"
#include <utf.h>


// CONSTANTS

_LIT8( KBTSUImageTag, "image-formats" );
_LIT8( KBTSUAttributeEncoding, "encoding" );
_LIT8( KBTSUAttributePixel, "pixel" );
_LIT8( KBTSUAttributeMaxSize,"maxsize");
_LIT(minus,"-");
_LIT(star,"*");




#define  INVALIDVALUE   -1
// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSBIPController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSUCapabilityResolver* CBTSUCapabilityResolver::NewL(CDesCArrayFlat* aCapabilityList)
    {
    CBTSUCapabilityResolver* self = new( ELeave ) CBTSUCapabilityResolver( aCapabilityList );
    CleanupStack::PushL( self );
    self->ConstructL(  );
    CleanupStack::Pop(self);
    return self;
    }
    
// -----------------------------------------------------------------------------
// CBTSBIPController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSUCapabilityResolver* CBTSUCapabilityResolver::NewL(RArray<TBTSUImageCap>* aCapabilityList)
    {
    CBTSUCapabilityResolver* self = new( ELeave ) CBTSUCapabilityResolver( aCapabilityList );
    CleanupStack::PushL( self );
    self->ConstructL(  );
    CleanupStack::Pop(self);
    return self;
    }
// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::CBTSUCapabilityResolver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSUCapabilityResolver::CBTSUCapabilityResolver( CDesCArrayFlat* aCapabilityList )
	{	
	aCapabilityList=aCapabilityList;
	}
// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::CBTSUCapabilityResolver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//	
CBTSUCapabilityResolver::CBTSUCapabilityResolver( 
    RArray<TBTSUImageCap>* aCapabilityList ) : iCapabilityList( aCapabilityList )
    {
    }
    
// -----------------------------------------------------------------------------
// CBTSBIPController::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSUCapabilityResolver::ConstructL ()
    {
    }   
  
// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CBTSUCapabilityResolver::~CBTSUCapabilityResolver()
    {
    }

// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::StartDocument
// -----------------------------------------------------------------------------
//
void CBTSUCapabilityResolver::OnStartDocumentL(const Xml::RDocumentParameters& /*aDocParam*/, TInt aErrorCode)
    {
    FLOG(_L("[BTSU]\t CBTSUCapabilityResolver::OnStartDocumentL()"));
    if(aErrorCode!=KErrNone)
        {
        FLOG(_L("[BTSU]\t Error OnStartDocument"));            
        }    
    iCompleted=EFalse;
    }

// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::EndDocument
// -----------------------------------------------------------------------------
//
void CBTSUCapabilityResolver::OnEndDocumentL(TInt aErrorCode)
    {
    FLOG(_L("[BTSU]\t CBTSUCapabilityResolver::OnEndDocumentL()"));
    if(aErrorCode!=KErrNone)
        {
        FLOG(_L("[BTSU]\t Error EndDocument"));            
        }            
    }

// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::StartElement
// -----------------------------------------------------------------------------
//
void CBTSUCapabilityResolver::OnStartElementL(const Xml::RTagInfo& aElement, const Xml::RAttributeArray& aAttributes, 
								 TInt /*aErrorCode*/)  
    {    
 
    TBool status = EFalse;
    TBTSUImageCap param;
    TLex lex;
    TPtrC sizemaxwidth;
    TPtrC sizemaxheight;
    TPtrC sizeminwidth;
    TPtrC sizeminheight;
    TPtrC sizemin;
    TPtrC sizemax;	
    TInt index;
    HBufC* enc;	 
    param.iMaxByteSize=0;                

    TBuf<KBTSUMaxStringLength> conversionBuf;
    
    if( aElement.LocalName().DesC() != KBTSUImageTag )
        {        
        return;
        }
    
  
    if(aAttributes.Count()>0)
        {		
	
        param.iEncoding=NULL; 
        param.iMaxByteSize=-1;
        param.iMinPixelSize.iHeight=INVALIDVALUE;
        param.iMinPixelSize.iWidth=INVALIDVALUE;		 
        param.iMaxPixelSize.iWidth=INVALIDVALUE;
        param.iMaxPixelSize.iHeight=INVALIDVALUE;
		 
    	//Check and add to attribute list if there is encoding    	    	    	
        for ( TInt iCounter = 0 ; iCounter < aAttributes.Count() ; iCounter++ )
            {
            if ( aAttributes[iCounter].Attribute().LocalName().DesC() == KBTSUAttributeEncoding )
                {                              
                User::LeaveIfError( CnvUtfConverter::ConvertToUnicodeFromUtf8( conversionBuf, aAttributes[iCounter].Value().DesC()));                                                    
                
                if (conversionBuf.Length()>0)
                    {	                                        		
                    enc=conversionBuf.Alloc();
                    TPtr rwdata(enc->Des());     
                    rwdata.Copy(conversionBuf);                 
                    param.iEncoding=enc;         	                    
                    if (param.iEncoding!=NULL)
                    	{                    	
                    	status=ETrue;
                    	}         		
                    }
                }
            }          			
       	conversionBuf = KNullDesC;
        for ( TInt iCounter = 0 ; iCounter < aAttributes.Count() ; iCounter++ )        
            {                
            if ( aAttributes[iCounter].Attribute().LocalName().DesC() == KBTSUAttributePixel )
                {                    ;
                User::LeaveIfError( CnvUtfConverter::ConvertToUnicodeFromUtf8( conversionBuf, aAttributes[iCounter].Value().DesC()));
                }
            }            
            
		if(conversionBuf.Length()>1)
			{				
			index=conversionBuf.Find(minus);	
			if(index!=KErrNotFound) //max size is given
				{					
				sizemin.Set(conversionBuf.Left(index));					
				sizemax.Set(conversionBuf.Right(conversionBuf.Length()-index-1));
				//solve min size
				index=sizemin.Find(star); 	
				if(index!=KErrNotFound)
					{
					sizeminwidth.Set(sizemin.Left(index));
					sizeminheight.Set(sizemin.Right(sizemin.Length()-index-1));
													
					lex.Assign(sizeminwidth);
					lex.Val(param.iMinPixelSize.iHeight);
						
					lex.Assign(sizeminheight);
					lex.Val(param.iMinPixelSize.iWidth);						
					}
				//solve max size	
				index=sizemax.Find(star);	
				if(index!=KErrNotFound)
					{
					sizemaxwidth.Set(sizemax.Left(index));
					sizemaxheight.Set(sizemax.Right(sizemax.Length()-index-1));
						
					lex.Assign(sizemaxwidth);
					lex.Val(param.iMaxPixelSize.iWidth);
						
					lex.Assign(sizemaxheight);
					lex.Val(param.iMaxPixelSize.iHeight);
					status=ETrue;
					}						
				}
			else	//if max size not given
				{						
				index=conversionBuf.Find(star);	
				if(index!=KErrNotFound) 
					{
					//solve min size
					TPtrC sizeminwidth=conversionBuf.Left( index );						
					TPtrC sizeminheight=conversionBuf.Right( conversionBuf.Length() - index-1);		
					
					lex.Assign(sizeminwidth);
					lex.Val(param.iMinPixelSize.iWidth);
					
					lex.Assign(sizeminheight);
					lex.Val(param.iMinPixelSize.iHeight);
					//set max size
					param.iMaxPixelSize.iWidth=param.iMinPixelSize.iWidth;
					param.iMaxPixelSize.iHeight=param.iMinPixelSize.iHeight;
                    status=ETrue;							
					}						
				}				
		    }        	


        	//Check and add to attribute list if there is maxsize
            TPtrC maxsizevalue;
            conversionBuf = KNullDesC;
            for ( TInt iCounter = 0 ; iCounter < aAttributes.Count() ; iCounter++ )        
                {                
                if ( aAttributes[iCounter].Attribute().LocalName().DesC() == KBTSUAttributeMaxSize )
                    {                                            
                    User::LeaveIfError( CnvUtfConverter::ConvertToUnicodeFromUtf8( conversionBuf, aAttributes[iCounter].Value().DesC()));                                    
                    }
                }         
            
            
            if (conversionBuf.Length()>0)
            	{			
    			lex.Assign(conversionBuf);
    			lex.Val(param.iMaxByteSize);			
            	}           
            	
            if(status)
            	{
            	iCapabilityList->Append(param); //add to list
            	}   

        }     
   	 
    }
    
// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::OnEndElementL
// -----------------------------------------------------------------------------
//    
void CBTSUCapabilityResolver::OnEndElementL(const Xml::RTagInfo& /*aElement*/, TInt aErrorCode)
  {
    FLOG(_L("[BTSU]\t CBTSUCapabilityResolver::EndElement()"));    
    if(aErrorCode==KErrNone)
        {        
        iCompleted=ETrue;       
        }
    FLOG(_L("[BTSU]\t CBTSUCapabilityResolver::EndElement()"));    
        
    }
    
// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::OnContentL
// -----------------------------------------------------------------------------
//     
void CBTSUCapabilityResolver::OnContentL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
    {   
    }

// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::OnStartPrefixMappingL
// -----------------------------------------------------------------------------
// 
void CBTSUCapabilityResolver::OnStartPrefixMappingL(const RString& /*aPrefix*/, const RString& /*aUri*/, 
								   TInt /*aErrorCode*/)
    {    
    }

// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::OnEndPrefixMappingL
// -----------------------------------------------------------------------------
// 
void CBTSUCapabilityResolver::OnEndPrefixMappingL(const RString& /*aPrefix*/, TInt /*aErrorCode*/)
    {
    }

// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::OnIgnorableWhiteSpaceL
// -----------------------------------------------------------------------------
// 
void CBTSUCapabilityResolver::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
    {
    }

// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::OnSkippedEntityL
// -----------------------------------------------------------------------------
// 
void CBTSUCapabilityResolver::OnSkippedEntityL(const RString& /*aName*/, TInt /*aErrorCode*/)
    {
    }
    
// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::OnProcessingInstructionL
// -----------------------------------------------------------------------------
//     
void CBTSUCapabilityResolver::OnProcessingInstructionL(const TDesC8& /*aTarget*/, const TDesC8& /*aData*/, 
										  TInt /*aErrorCode*/)
    {
    }
    
// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::OnError
// -----------------------------------------------------------------------------
//    
void CBTSUCapabilityResolver::OnError(TInt /*aErrorCode*/)
    {
    }

// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::GetExtendedInterface
// -----------------------------------------------------------------------------
// 
TAny* CBTSUCapabilityResolver::GetExtendedInterface(const TInt32 /*aUid*/)
    {
        return NULL;
    }    
    
// -----------------------------------------------------------------------------
// CBTSUCapabilityResolver::IsCompleted
// -----------------------------------------------------------------------------
    
TBool CBTSUCapabilityResolver::IsCompleted()
    {
    return iCompleted;
    }

//  End of File  
