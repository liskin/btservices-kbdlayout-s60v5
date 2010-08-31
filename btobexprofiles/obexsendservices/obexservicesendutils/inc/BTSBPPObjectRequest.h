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
* Description:  GetReferencedObjects -operation controller
*
*/



#ifndef BTSBPP_OBJECT_REQUEST_H
#define BTSBPP_OBJECT_REQUEST_H

//  INCLUDES
#include <obex.h>   // Obex
#include <badesca.h>// CDesCArray

// CLASS DECLARATION
/**
*  A class handling GetReferencedObjects -operation.
*/
NONSHARABLE_CLASS (CBTSBPPObjectRequest) : public CBase
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTSBPPObjectRequest* NewL( CObexBaseObject* aGetOperation,
                                           const CDesCArray* aRefObjectList );
        
        /**
        * Destructor.
        */
        virtual ~CBTSBPPObjectRequest();

    public: // Functions from base classes

        /**
        * Returns the requested Object.
        * @param None.
        * @return A pointer to the requested object. NULL if the request 
        *         cannot be fullfilled.
        */
        CObexBufObject* GetResponse();

    private:

        /**
        * C++ default constructor.
        */
        CBTSBPPObjectRequest( CObexBaseObject* aGetOperation,
                            const CDesCArray* aRefObjectList );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // CHECK REQUEST ------------------------------------------------------

        /**
        * Checks the validity of the Get-operation and it's parameters.
        * @return None.
        */
        void CheckGetRequestL();

        /**
        * Checks the existence, validity and contents of Type header.        
        * @return None.
        */
        void CheckTypeHeaderL();

        /**
        * Checks the existence, validity and contents of Name header.        
        * @return None.
        */
        void CheckNameHeaderL();

        /**
        * Checks the existence, validity and contents of 
        * Application Parameters header.        
        * @return None.
        */
        void CheckAppParamsHeaderL();


        // EXECUTE REQUEST ----------------------------------------------------

        /**
        * Executes any tasks needed and creates the requested object.        
        * @return None.
        */
        void ExecuteGetRequestL();
        
        /**
        * Resolves the actual part of data that is requested.
        * @return None.
        */
        void ResolveGetRequestL();

        /**
        * Reads the requested part of a file into a buffer.        
        * @return None.
        */
        void ReadFileToBufferL();

        /**
        * Creates the response object with correct data and headers.        
        * @return None.
        */
        void CreateResponseObjectL();

    private: // Data definitions

        struct TBTSBPPObjectParams
            {
            const TDesC* iName; // The name of the object.
            TInt  iFileSize; // The total size of the object.
            TInt  iOffset;   // The offset to the the object (starting point).
            TInt  iCount;    // The number of bytes to be sent.
            };

    private:    // Data

        RFs                     iFileSession;
        TBTSBPPObjectParams     iRequestParams;
        CObexBufObject*         iResponse;
        CBufFlat*               iResponseBuffer;

        // Not owned
        //
        CObexBaseObject*        iRequest;
        const CDesCArray*       iObjectList;
    };

#endif      // BTSBPP_OBJECT_REQUEST_H
            
// End of File
