/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Browsing adapter between RemCon an MPX Framework. 
*
*/


#ifndef BTRCCBROWSINGADAPTER_H_
#define BTRCCBROWSINGADAPTER_H_

//  INCLUDES
#include <remcondatabaseawaremedialibrarybrowseobserver.h>
#include <remcondatabaseawarenowplayingbrowseobserver.h>
#include <remcondatabaseawaremediabrowsetarget.h>
#include <mpxcollectionutility.h>
#include <mpxcollectionpath.h>

// DATA TYPES

// FORWARD DECLARATIONS
class CRemConInterfaceSelector; 

// CLASS DECLARATION


NONSHARABLE_CLASS(CBTRCCBrowsingAdapter) :
        public MRemConDatabaseAwareMediaLibraryBrowseObserver,
        public MRemConDatabaseAwareNowPlayingBrowseObserver,
        public MMPXCollectionObserver
    {
    public:  // Constructors and destructor
        /**
        * Two-phased constructor.
        *      
        */
        static CBTRCCBrowsingAdapter* NewL(CRemConInterfaceSelector& aInterfaceSelector);
    
        /**
        * Destructor.
        */
        virtual ~CBTRCCBrowsingAdapter();


    private:
        /**
        * C++ default constructor.
        */
        CBTRCCBrowsingAdapter();
        
        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL(CRemConInterfaceSelector& aInterfaceSelector);
    
    private: // From MRemConDatabaseAwareMediaLibraryBrowseObserver
        void MrcdamlboGetFolderListing(TRemConFolderScope aScope, TUint aStartItem, TUint aEndItem);
        TInt MrcdamlboGetItem(TRemConFolderScope aScope,
                              const TRemConItemUid& aItemId, 
                              TMediaAttributeIter& aIter, 
                              TUint16 aMediaLibraryStateCookie);
        void MrcdamlboFolderUp(TUint16 aMediaLibraryStateCookie);
        void MrcdamlboFolderDown(const TRemConItemUid& aFolder, TUint16 aMediaLibraryStateCookie);
        void MrcdamlboGetPath(RPointerArray<HBufC8>& aPath);
        void MrcdamlboSearch(const TDesC8& aSearch);
        
    private: // From MRemConDatabaseAwareNowPlayingBrowseObserver
        void MrcdanpboGetFolderListing(TUint aStartItem, TUint aEndItem);
        TInt MrcdanpboGetItem(const TRemConItemUid& aItemId, TMediaAttributeIter& aIter, TUint16 aMediaLibraryStateCookie);
    
    private: // From MMPXCollectionObserver
        void HandleCollectionMessage(CMPXMessage* aMsg, TInt /*aErr*/);
        void HandleOpenL(const CMPXMedia& aEntries, TInt aIndex,TBool aComplete,TInt aError);
        void HandleOpenL(const CMPXCollectionPlaylist& aPlaylist,TInt aError);
        void HandleCommandComplete(CMPXCommand* /*aCommandResult*/, TInt /*aError*/);
        
    private: // From MMPXCollectionMediaObserver
        void HandleCollectionMediaL(const CMPXMedia& aMedia, TInt aError);
        
    private:
        void SubscribeL();
        
    private:
        enum TMediaLOpcode
            {
            EMediaLOpIdle,
            EMediaLOpFolderListing
            };
        
        enum TOpenLOpcode
            {
            EOpenLOpIdle,
            EOpenLOpOpen
            };
        
    private: // Data
        CRemConDatabaseAwareMediaBrowseTarget* iMediaBrowseTarget;
        MRemConDatabaseAwareMediaLibraryBrowse* iMlInterface; 
        MRemConDatabaseAwareNowPlayingBrowse* iNpInterface;
        
        MMPXCollectionUtility* iCollectionUtility;
        
        TUint16 iCookie;
        TMediaLOpcode iCurrentMediaLOp;     // current MediaL operation
        TOpenLOpcode iCurrentOpenLOp;       // current OpenL operation
    };
    
#endif /* BTRCCBROWSINGADAPTER_H_ */
