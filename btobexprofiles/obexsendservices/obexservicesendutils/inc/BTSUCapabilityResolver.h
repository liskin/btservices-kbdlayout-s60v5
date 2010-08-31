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
* Description:  Capability resolver
*
*/


#ifndef BTSU_CAPABILITY_RESOLVER_H
#define BTSU_CAPABILITY_RESOLVER_H

// INCLUDES
//#include <RXMLReader.h>
// new headers
#include <xml/contenthandler.h>
#include <xml/parser.h> 


#include <badesca.h>

// CLASS DECLARATION

/**
*  A content handler class implementing MXMLContentHandler interface.
*  Listens notifications and appends found capabilities to a given list.
*/
NONSHARABLE_CLASS (CBTSUCapabilityResolver) :public CBase, 
                                             public Xml::MContentHandler 
    {
    public:  // Constructors and destructor
    
         /**
        * Two-phased constructor.
        */
        static CBTSUCapabilityResolver* NewL( CDesCArrayFlat* aCapabilityList );
        
        
        static CBTSUCapabilityResolver* NewL(  RArray<TBTSUImageCap>* aCapabilityList );        
        
        /**
        * Destructor.
        */
        virtual ~CBTSUCapabilityResolver();
        
        /**
        * Return capability object resolving status         
        * @return A boolean according to status.
        */   
        TBool IsCompleted();

    private:

        /**
        * From MContentHandler A notification telling about the beginning 
        * of a document.
        * @param  aDocParam     Specifies the various parameters of the document.
        * @param  aErrorCode    is the error code. 
        */	        
        void OnStartDocumentL(const Xml::RDocumentParameters& aDocParam, TInt aErrorCode);        

        /**
        * From MContentHandler A notification telling about the end 
        * of a document.
        * @param  	aErrorCode is the error code. 
					If this is not KErrNone then special action may be required.        
        */		    
	    void OnEndDocumentL(TInt aErrorCode);

        /**
        * From MContentHandler A notification telling about the beginning 
        * of an element.
        * @param				aElement is a handle to the element's details.
        * @param				aAttributes contains the attributes for the element.
        * @param				aErrorCode is the error code.        
        * @return A boolean according to success.
        */
	    void OnStartElementL(const Xml::RTagInfo& aElement, const Xml::RAttributeArray& aAttributes, 
								 TInt aErrorCode);   
								 
        /**
        This method is a callback to indicate the end of the element has been reached.
        @param				aElement is a handle to the element's details.
        @param				aErrorCode is the error code.
        					If this is not KErrNone then special action may be required.
        */
        void OnEndElementL(const Xml::RTagInfo& aElement, TInt aErrorCode);
        
        /**
        This method is a callback that sends the content of the element.
        Not all the content may be returned in one go. The data may be sent in chunks.
        When an OnEndElementL is received this means there is no more content to be sent.
        @param				aBytes is the raw content data for the element. 
        					The client is responsible for converting the data to the 
        					required character set if necessary.
        					In some instances the content may be binary and must not be converted.
        @param				aErrorCode is the error code.
        					If this is not KErrNone then special action may be required.
        */
        void OnContentL(const TDesC8& aBytes, TInt aErrorCode);     
        /**
        This method is a notification of the beginning of the scope of a prefix-URI Namespace mapping.
        This method is always called before the corresponding OnStartElementL method.
        @param				aPrefix is the Namespace prefix being declared.
        @param				aUri is the Namespace URI the prefix is mapped to.
        @param				aErrorCode is the error code.
        					If this is not KErrNone then special action may be required.
        */
    	void OnStartPrefixMappingL(const RString& aPrefix, const RString& aUri, 
    									   TInt aErrorCode);           								                         

        /**
        This method is a notification of the end of the scope of a prefix-URI mapping.
        This method is called after the corresponding DoEndElementL method.
        @param				aPrefix is the Namespace prefix that was mapped.
        @param				aErrorCode is the error code.
        					If this is not KErrNone then special action may be required.
        */
       	void OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode);


        /**
        This method is a notification of ignorable whitespace in element content.
        @param				aBytes are the ignored bytes from the document being parsed.
        @param				aErrorCode is the error code.
        					If this is not KErrNone then special action may be required.
        */
        void OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode);


        /**
        This method is a notification of a skipped entity. If the parser encounters an 
        external entity it does not need to expand it - it can return the entity as aName 
        for the client to deal with.
        @param				aName is the name of the skipped entity.
        @param				aErrorCode is the error code.
        					If this is not KErrNone then special action may be required.
        */
        void OnSkippedEntityL(const RString& aName, TInt aErrorCode);


        /**
        This method is a receive notification of a processing instruction.
        @param				aTarget is the processing instruction target.
        @param				aData is the processing instruction data. If empty none was supplied.
        @param				aErrorCode is the error code.
        					If this is not KErrNone then special action may be required.
        */
        void OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, 
        										  TInt aErrorCode);


        /**
        This method indicates an error has occurred.
        @param				aError is the error code
        */
        void OnError(TInt aErrorCode);


        /**
        This method obtains the interface matching the specified uid.
        @return				0 if no interface matching the uid is found.
        					Otherwise, the this pointer cast to that interface.
        @param				aUid the uid identifying the required interface.
        */
        TAny* GetExtendedInterface(const TInt32 aUid);
        
        /**
        * C++ default constructor.
        */               
        CBTSUCapabilityResolver( CDesCArrayFlat* aCapabilityList );

		/**
        * C++ default constructor.
        */          
		CBTSUCapabilityResolver( RArray<TBTSUImageCap>* aCapabilityList );
		
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        

    private: // Data

        // Not owned.
        //
        RArray<TBTSUImageCap>* iCapabilityList;
        TBool                  iCompleted;
    };

#endif      // BTSU_CAPABILITY_RESOLVER_H
            
// End of File
