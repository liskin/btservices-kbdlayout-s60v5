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
*     Contains Bluetooth MTM Ui part.
*     
*
*/


#ifndef CBTMTMUI_H
#define CBTMTMUI_H

//  INCLUDES
#include <btdevice.h>
#include <mtmuibas.h>
#include <btengdiscovery.h>
#include <obexutilsdialog.h>

const TUint KBTSdpObjectPush            = 0x1105;

// FORWARD DECLARATIONS
class CClientMtmRegistry;

/**
*  CBtMtmUi
*  Bluetooth messaging UI Module.
*/
class CBtMtmUi : public CBaseMtmUi, public MBTEngSdpResultReceiver, 
                 public MObexUtilsDialogObserver
	{
    public: // Constructors and destructor
       /**
       * Two-phased constructor.
       */
	   static CBtMtmUi* NewL( CBaseMtm& aBaseMtm, CRegisteredMtmDll& aRegisteredMtmDll );

        /**
        * Destructor.
        */
	    virtual ~CBtMtmUi();


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
	
        //
	    // --- Message responding ---
	    // --- Default implementation calls CBaseMtm::ReplyL() or ForwardL(), 
        //     followed by EditL() ---
        // --- Takes ownership of the passed entry 
        //     (which the context is switched to) ---

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

	    //
	    // Actions upon message selections ---
	    // Selections must be in same folder and all of the correct MTM type
	    // Context may change after calling these functions

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

        //
        // Copy and move functions:
        // Context should be set to folder or entry of this MTM 
        // Default implementations imply call the relevant CMsvEntry functions
	
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
	
	    // Interpret transfer progress 

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
	
        //	
        // --- RTTI functions ---

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
            
            
         /**
        * Provides notification of the result of a service search that matches 
        * the requested UUID (through CBTEngDiscovery::RemoteSdpQuery).
        * This method indicates that the search has completed, and returns 
        * all the results to the caller at once.
        *
        * @since S60 v3.2
        * @param aResult Array of record handles that match the requested UUID.
        *                Note: the array will not be available anymore after 
        *                this method returns.
        * @param aTotalRecordsCount The total number of records returned.
        * @param aErr Error code of the service search operation; KErrNone if 
        *             sucessful, KErrEof if no record matched the requested UUID, 
        *             KErrCouldNotConnect and KErrCouldDisconnected in case of 
        *             Bluetooth connection errors; otherwise one of the 
        *             system-wide error codes.
        */
        void ServiceSearchComplete( const RSdpRecHandleArray& aResult, 
                                    TUint aTotalRecordsCount, TInt aErr );

        /**
        * Provides notification of the result of an attribute search that matches 
        * the requested attribute (through CBTEngDiscovery::RemoteSdpQuery).
        * This method indicates that the search has completed, and returns 
        * all the results to the caller at once.
        *
        * @since S60 v3.2
        * @param aHandle Record handle of the service record containing the result.
        * @param aAttr Array containing the attribute that matches the 
        *              requested attribute.
        *              Note: the array will not be available anymore after 
        *              this method returns.
        * @param aErr Error code of the service search operation; KErrNone if 
        *             sucessful, KErrEof if the requested attribute was not 
        *             contained in the specified service record, 
        *             KErrCouldNotConnect and KErrCouldDisconnected in case of 
        *             Bluetooth connection errors; otherwise one of the 
        *             system-wide error codes.
        */
        void AttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                           const RSdpResultArray& aAttr, 
                                           TInt aErr );

        /**
        * Provides notification of the result of an combination of a service 
        * and attribute search (through CBTEngDiscovery::RemoteSdpQuery).
        * This method is called for each service and attribute combination for 
        * which a match was found. The last result (which could be empty if no 
        * match was found) contain error code KErrEof to indicate that the 
        * search has completed.
        *
        * @since S60 v3.2
        * @param aHandle Record handle of the service record containing the result.
        * @param aAttr Array containing the attribute that matches the 
        *              requested attribute.
        *              Note: the array will not be available anymore after 
        *              this method returns.
        * @param aErr Error code of the service search operation; KErrNone if 
        *             sucessful and more results follow, KErrEof indicates that 
        *             this is the last result (which could be empty if no match 
        *             was found), KErrCouldNotConnect and KErrCouldDisconnected 
        *             in case of Bluetooth connection errors; otherwise one of 
        *             the system-wide error codes.
        */
        void ServiceAttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                                      const RSdpResultArray& aAttr, 
                                                      TInt aErr );

        /**
        * Provides notification of the result of the discovery of nearby 
        * Bluetooth devices.
        *
        * @since S60 v3.2
        * @param aDevice The data structure encapsulates all information 
        *                about the selected device. Ownership of the data 
        *                structure has not been transfered and is still with
        *                the API client.
        * @param aErr Error code of the device search operation; KErrNone if 
        *             sucessful, KErrCancel if the user cancelled the 
        *             dialog, KErrAbort if CBTEngDiscovery::CancelSearchRemoteDevice
        *             was called; otherwise one of the system-wide error codes.
        */
        void DeviceSearchComplete( CBTDevice* aDevice, TInt aErr );    
        
        /**
         * Callback function from base class MObexUtilsDialogObserver.
         */
        void DialogDismissed(TInt aButtonId);

    protected:

        /**
        * C++ default constructor.
        */ 
        CBtMtmUi( CBaseMtm& aBaseMtm, CRegisteredMtmDll& aRegisteredMtmDll );

        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

    protected: // Functions from base classes

        /**
        * From CBaseMtmUi return resource filename.
        * @param aProgress reference to summary text.
        * @return TInt error code.
        */
        void GetResourceFileName( TFileName& aFileName ) const;

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

        CClientMtmRegistry*		iClientRegistry;    // pointer to registration.

    private:
        TBTDeviceName        iBTDeviceName;                        // Bluetooth device name.
        CBTDevice*           iDevice;
        CBTEngDiscovery*     iDiscovery;
        CActiveSchedulerWait iWaiter;
        TInt                 iClientChannel;
        TInt                 iState;  
        CObexUtilsDialog*  iDialog;
	};

#endif      // CBTMTMUI_H  
            
// End of File
