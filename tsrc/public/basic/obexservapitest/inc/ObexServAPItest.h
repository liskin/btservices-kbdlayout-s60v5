/*
* Copyright (c) 2002 - 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ?Description
*
*/



#ifndef OBEXSERVAPITEST_H
#define OBEXSERVAPITEST_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>
#include <ObexUtilsMessageHandler.h>
#include "testlogger.h"


// FORWARD DECLARATIONS
class CObexServAPItest;


// CLASS DECLARATION

/**
*  CObexServAPItest test class for STIF Test Framework TestScripter.
*/
NONSHARABLE_CLASS(CObexServAPItest) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CObexServAPItest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CObexServAPItest();

    public: // New functions

       

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

    

    private:

        /**
        * C++ default constructor.
        */
        CObexServAPItest( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // Prohibit copy constructor if not deriving from CBase.
        // ?classname( const ?classname& );
        // Prohibit assigment operator if not deriving from CBase.
        // ?classname& operator=( const ?classname& );

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();
        
        void TestCompleted( TInt aErr, const TUint8* aFunc, const TDesC& aArg );

        /**
        * Test methods are listed below. 
        */

        /**
        * Test code for getting file system status.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt GetFileSystemStatusL( CStifItemParser& aItem );
        
        /**
        * Test code for getting mmc file system status.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt GetMmcFileSystemStatusL( CStifItemParser& aItem );
        
        /**
        * Test code for getting message center drive. 
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt GetMessageCentreDriveL( CStifItemParser& aItem );
        
        /**
        * Test code for creating Default MTM services.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt CreateDefaultMtmServiceL( CStifItemParser& aItem );
        
        /**
        * Test code for Getting CenRep key.    .    
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt GetCenRepKeyIntValueL( CStifItemParser& aItem );
        
        /**
        * Test code for Getting PubSub key.    
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt GetPubSubKeyIntValueL( CStifItemParser& aItem );
        
        /**
        * Test code for Creating outbox entry.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt CreateOutBoxEntryL( CStifItemParser& aItem );
        
        /**
        * Test code for removing outbox entry.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt RemoveOutBoxEntryL( CStifItemParser& aItem );
        
        /**
        * Test code for save received file to inbox. 
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt SaveObjToInboxL( CStifItemParser& aItem );
        
        /**
        * Test code for creating entry to inbox.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt CreateEntryToInboxL( CStifItemParser& aItem );
        
        /**
        * Test code for creating entry to inbox.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt SaveRFileObjectToInboxL( CStifItemParser& aItem );
        
        /**
        * Test code for removing entry to inbox.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt RemoveObjectL( CStifItemParser& aItem );
        
     

    private:    // Data
        
        
        
         TMsvId           iMessageServerIndex;
         CObexBufObject*  iObexObject; 
         RFile            iFile;
         CBufFlat*        iBuf; 
         CObexTestLogger* iTestLogger;

    };

#endif      // OBEXSERVAPITEST_H

// End of File
