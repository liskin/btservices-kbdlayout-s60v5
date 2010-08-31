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
* Description: 
*     Infrared Mtm Ui part.
*
*
*/


#ifndef CIRMTMUI_H
#define CIRMTMUI_H

//  INCLUDES
#include <mtmuibas.h>
// CLASS DECLARATION

// FORWARD DECLARATIONS
class CClientMtmRegistry;
class CMtmUiLayer;



/**
*  CCommonMtmUi
*/
class CIrMtmUi : public CBaseMtmUi
	{
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CIrMtmUi* NewL(
            CBaseMtm& aBaseMtm, 
            CRegisteredMtmDll& aRegisteredMtmDll );

        /**
        * Destructor.
        */
	    virtual ~CIrMtmUi();

    public: // Functions from base classes

        /**
        * From CBaseMtmUi Entry Creation.
        * @param aEntry reference to entry
        * @param aParent parent entry
        * @return CMsvOperation pointer to Msg operation.
        */
	    virtual CMsvOperation* CreateL(
            const TMsvEntry& aEntry, 
            CMsvEntry& aParent, 
            TRequestStatus& aStatus );

        /**
        * From CBaseMtmUi Open entry.
        * @param aStatus Status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
	    virtual CMsvOperation* OpenL( TRequestStatus& aStatus ); 

        /**
        * From CBaseMtmUi Close entry.
        * @param aStatus Status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
	    virtual CMsvOperation* CloseL( TRequestStatus& aStatus ); 

        /**
        * From CBaseMtmUi Launches editor/settings dialog as appropriate. 
        * @param aStatus Status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
	    virtual CMsvOperation* EditL( TRequestStatus& aStatus ); 
	
        /**
        * From CBaseMtmUi Launches viewer/settings dialog as appropriate
        * @param aStatus Status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* ViewL( TRequestStatus& aStatus );

        /**
        * From CBaseMtmUi Deletes selection from current context
	    * Deletes entries from the current context, 
        * which must be a folder or service of the relevant MTM  
        * @param aSelection reference to entry.
        * @param aStatus Status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
	    virtual CMsvOperation* DeleteFromL(
            const CMsvEntrySelection& aSelection, 
            TRequestStatus& aStatus );

        /**
        * From CBaseMtmUi Deletes service, which need not 
        * be the current context.
        * @param aService reference to entry.
        * @param aStatus Status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
	    virtual CMsvOperation* DeleteServiceL(
            const TMsvEntry& aService, 
            TRequestStatus& aStatus );

        /**
        * From CBaseMtmUi Remove remaining message of deleted service. 
        * @param aChildren reference to entry selection.
        * @return None.
        */
	    void DeleteRecursivelyL( CMsvEntrySelection& aChildren );
	

        /**
        * From CBaseMtmUi Reply entry
        * @param aDestination id for msv.
        * @param aPartlist partlist id.
        * @param aCompletionStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* ReplyL(
            TMsvId aDestination, 
            TMsvPartList aPartlist, 
            TRequestStatus& aCompletionStatus );
	

        /**
        * From CBaseMtmUi forward entry
        * @param aDestination id for msv.
        * @param aPartlist partlist id.
        * @param aCompletionStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* ForwardL(
            TMsvId aDestination, 
            TMsvPartList aPartList, 
            TRequestStatus& aCompletionStatus );


        /**
        * From CBaseMtmUi Open entry
        * @param aStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* OpenL(
            TRequestStatus& aStatus, 
            const CMsvEntrySelection& aSelection ); 

        /**
        * From CBaseMtmUi Close entry
        * @param aStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* CloseL(
            TRequestStatus& aStatus, 
            const CMsvEntrySelection& aSelection ); 

        /**
        * From CBaseMtmUi Handles sending.
        * @param aStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* EditL(
            TRequestStatus& aStatus, 
            const CMsvEntrySelection& aSelection );

        /**
        * From CBaseMtmUi Launches viewer/settings dialog as appropriate
        * @param aStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* ViewL(
            TRequestStatus& aStatus, 
            const CMsvEntrySelection& aSelection );

	
        /**
        * From CBaseMtmUi Context should be MTM folder/service to copy to
        * @param aStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* CopyToL(
            const CMsvEntrySelection& aSelection, 
            TRequestStatus& aStatus );

        /**
        * From CBaseMtmUi Context should be MTM folder/service to Move to
        * @param aStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* MoveToL(
            const CMsvEntrySelection& aSelection, 
            TRequestStatus& aStatus );

        /**
        * From CBaseMtmUi Context should be MTM folder/service to copy from
        * @param aStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* CopyFromL(
            const CMsvEntrySelection& aSelection, 
            TMsvId aTargetId, 
            TRequestStatus& aStatus ); 

        /**
        * From CBaseMtmUi Context should be MTM folder/service to move from
        * @param aStatus status for operation.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* MoveFromL(
            const CMsvEntrySelection& aSelection, 
            TMsvId aTargetId, 
            TRequestStatus& aStatus );
	


        /**
        * From CBaseMtmUi Display progress summary.
        * @param aProgress reference to summary text.
        * @return TInt error code.
        */
        TInt DisplayProgressSummary( const TDesC8& aProgress ) const;

        /**
        * From CBaseMtmUi return progress status.
        * @param aProgress reference to progress packet.
        * @param aReturnString returned progress text.
        * @param aTotalEntryCount entry count.
        * @param aEntriesDone completed entries.
        * @param aCurrentEntrySize size of the entry.
        * @param aCurrentBytesTrans transferred bytes.
        * @return TInt error code.
        */
        virtual TInt GetProgress(
            const TDesC8& aProgress, 
            TBuf<EProgressStringMaxLen>& aReturnString, 
            TInt& aTotalEntryCount, 
            TInt& aEntriesDone,
		    TInt& aCurrentEntrySize, 
            TInt& aCurrentBytesTrans ) const;


        /**
        * From CBaseMtmUi Return capability.
        * @param aCapability UID for capability.
        * @return TInt error code.
        */
        virtual TInt QueryCapability( TUid aCapability, TInt& aResponse );

        /**
        * From CBaseMtmUi Synchronous MTM function operation.
        * @param aFunctionId Function ID.
        * @param CMsvEntrySelection entry reference.
        * @param aParam parameter for function.
        * @return None.
        */
        virtual void InvokeSyncFunctionL(
            TInt aFunctionId, 
            const CMsvEntrySelection& aSelection, 
            TDes8& aParameter );

        /**
        * From CBaseMtmUi Cancel sending.
        * @param aStatus Async status for operation.
        * @param CMsvEntrySelection entry reference.
        * @return CMsvOperation pointer to Msg operation.
        */
        virtual CMsvOperation* CancelL(
            TRequestStatus& aStatus, 
            const CMsvEntrySelection& aSelection );

    protected:

        /**
        * C++ default constructor.
        */ 
        CIrMtmUi( CBaseMtm& aBaseMtm, CRegisteredMtmDll& aRegisteredMtmDll );

        /**
        * By default Symbian OS constructor is private.
        */
        virtual void ConstructL();

    protected: // Functions from base classes

        /**
        * From CBaseMtmUi return resource filename.
        * @param aProgress reference to summary text.
        * @return TInt error code.
        */
        virtual void GetResourceFileName( TFileName& aFileName ) const;

        /**
        * From CBaseMtmUi Launch viewer for received message.
        * @param aStatus Async status for operation.
        * @param aSession session reference.
        * @return CMsvOperation pointer to Msg operation.
        */
        CMsvOperation*	LaunchEditorApplicationL(
            TRequestStatus& aStatus, 
            CMsvSession& aSession );

    protected:  // New functions

        /**
        * Register MTM.
        * @param aMtmType MTM UID.
        * @return CBaseMtm pointer.
        */
        CBaseMtm* GetClientMtmLC( TUid aMtmType );

        /**
        * Display progress summary.
        * @param aProgress reference to summary text.
        * @return TInt error code.
        */
        TInt DisplayProgressSummaryL( const TDesC8& aProgress ) const;
 
    protected: // data
        TBuf<EProgressStringMaxLen>     iConnectingText;
        TBuf<EProgressStringMaxLen>     iSendingText;
        CClientMtmRegistry*	        	iClientRegistry;    // pointer to registration.
        TBool				         	iCurrentlySending;  // Sending state
	};

#endif  // CIRMTMUI_H

// End of File
