/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


#ifndef COBEXUTILSDIALOG_H
#define COBEXUTILSDIALOG_H

#include    <e32base.h>

#include <hbdeviceprogressdialogsymbian.h>
#include <hbdevicedialogsymbian.h>
#include <hbdevicemessageboxsymbian.h>
#include <hbsymbianvariant.h>

class CObexUtilsDialogTimer;

enum TButtonId
    {
    ECancelButton = -1,
    EYesButton,
    ENoButton
    };
/**
*  An observer interface for asking progress status of an operation using 
*  a progress dialog.
*/
NONSHARABLE_CLASS(  MObexUtilsProgressObserver )
    {
    public:

        /**
        * Returns the progress status of the operation.
        * @since 2.6
        * @return A progress value relative to final value.
        */
        virtual TInt GetProgressStatus() = 0;
    };

// CLASS DECLARATION

/**
*  An observer interface for informing about dialog events.
*/
NONSHARABLE_CLASS(  MObexUtilsDialogObserver )
    {
    public:

        /**
        * Informs the observer that a dialog has been dismissed.
        * @since 2.6
        * todo check whether the parameter is required
        * @param aButtonId The button that was used to dismiss the dialog.
        */
        virtual void DialogDismissed(TInt aButtonId) = 0; //TInt aButtonId
    };


// CLASS DECLARATION

/**
*  A class for launching and managing dialogs.
*/
NONSHARABLE_CLASS( CObexUtilsDialog ) : public CBase, 
                                        public MHbDeviceProgressDialogObserver,
                                        public MHbDeviceDialogObserver,
                                        public MHbDeviceMessageBoxObserver
    {
    public:// Constructors and destructor

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CObexUtilsDialog* NewL( 
            MObexUtilsDialogObserver* aObserverPtr );        
        
        IMPORT_C static CObexUtilsDialog* NewLC( 
            MObexUtilsDialogObserver* aObserverPtr );

        /**
        * Destructor.
        */
        virtual ~CObexUtilsDialog();
   
    public: // New functions
        
        
        /**
        * Launches a progress dialog.
        * @param aObserverPtr A pointer to progress observer. A NULL pointer if 
                              the progress dialog is updated manually.
        * @param aFileCount Total number of files to be sent.
        * @param aDeviceName Device Name to which files to be sent.
        * @param aTimeoutValue A value telling how often should the dialog be
                               updated. Relevant only if observer is given.
        */
        IMPORT_C void LaunchProgressDialogL( 
            MObexUtilsProgressObserver* aObserverPtr, TInt aFileCount, 
            const TDesC& aDeviceName, TInt aTimeoutValue );
        
        /**
        * Updates the progress dialog with new file information when multiples files are sent.
        * @param aFileSize Size of the file to be sent
        * @param aFileIndex Index of the file to be sent
        * @param aFileName Name of the file to be sent.
        */
        IMPORT_C void UpdateProgressNoteL( TInt aFileSize,TInt aFileIndex, const TDesC& aFileName );    
        
        /**
        * Launches a wait dialog.
        * @param aDisplayText Text that needs to be displayed.
        */
        IMPORT_C void LaunchWaitDialogL( const TDesC& aDisplayText );
        
        /**
        * Cancels a wait dialog if one exists.
        * @since 2.6        
        * @return None.
        */
        IMPORT_C void CancelWaitDialog();
        
        /**
        * Cancels a wait progress dialog if one exists.
        * @since 2.6       
        * @return None.
        */
        IMPORT_C void CancelProgressDialog();
        
        /**
        * Updates a progress dialog with the latest progress value 
        * @param aValue A progress value relative to final value.
        * @return None
        */
        IMPORT_C void UpdateProgressDialogL( TInt aProgressValue );
        
        /**
        * Show a query note
        * @param aConfirmText text for the note.
        * @return None		
        */
        IMPORT_C void LaunchQueryDialogL( const TDesC& aConfirmText );

        /**
        * Shows an error note.
        * @param aTextId A resource id for the note.
        * @return None.
        */
        IMPORT_C void ShowErrorNoteL( const TDesC& aErrorText );

        /**
        * Shows an information note.
        * @param aTextId A resource id for the note.
        * @return None.
        */
        IMPORT_C void ShowInformationNoteL( const TDesC& aInfoText );

    public: // New functions (not exported)

        /**
        * Updates the progress dialog.
        * @return None.
        */
        void UpdateProgressDialog();

    private: // Functions from base classes
        
        /**
        * From MHbDeviceProgressDialogObserver called when dialog is closed by pressing the "cancel" button
        * @param aDialog Pointer to dialog that was cancelled.
        * @return None.
        */
        void ProgressDialogCancelled(const CHbDeviceProgressDialogSymbian* aDialog);
        
        /**
        * From MHbDeviceProgressDialogObserver called when a device progress dialog is has closed
        * @param aDialog Pointer to dialog instance that has closed.
        * @return None.
        */
        void ProgressDialogClosed(const CHbDeviceProgressDialogSymbian* aDialog )  ; 
        
        /**
          * From MHbDeviceDialogObserver called when data is received from a device dialog
          * @param aDialog Pointer to dialog instance that has closed.
          * @return None.
          */
        void DataReceived(CHbSymbianVariantMap& aData);
       
        /**
          * From MHbDeviceDialogObserver called when a device dialog is closed
          * @param  aData contains data from the dialog plugin.
          * @return None.
          */
         void DeviceDialogClosed(TInt aCompletionCode);
        
        /**
        * from base class MHbDeviceMessageBoxObserver
        * @param aMessageBox Pointer to dialog instance that has closed.
        * @param aButton the id of the button the user pressed
        */    
        void  MessageBoxClosed(const CHbDeviceMessageBoxSymbian *aMessageBox, 
                CHbDeviceMessageBoxSymbian::TButtonId aButton); 

    private:

        /**
        * C++ default constructor.
        */
        CObexUtilsDialog( MObexUtilsDialogObserver* aObserverPtr );
	
        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

        /**
         * Add a data item into the given CHbSymbianVariantMap.
         * @param aMap the instance to which the data item will be added.
         * @param aKey the key of the data item.
         * @param aData the value of the data item
         * @param aDataType the data-type of the data item
         */
        void AddDataL(CHbSymbianVariantMap* aMap, const TDesC& aKey, 
                const TAny* aData, CHbSymbianVariant::TType aDataType);
        
        /**
         * Creates and shows a message box.
         * @param aType the type of the message box to create.
         * @param aText the text to be shown in the message box
         * @param aObserver the observer that receives the events from the 
         *         message box
         * @param aTimeout the timeout value of the message box 
         */
        CHbDeviceMessageBoxSymbian* CreateAndShowMessageBoxL(
                CHbDeviceMessageBoxSymbian::TType aType,
                const TDesC& aText, 
                MHbDeviceMessageBoxObserver* aObserver,
                TInt aTimeout );
        
    private: // Data

        CHbDeviceDialogSymbian *iProgressDialog;
        CHbDeviceProgressDialogSymbian* iWaitDialog;
        CHbDeviceMessageBoxSymbian* iMessageBox;
        
        CObexUtilsDialogTimer*      iObexDialogTimer;
        TInt                        iFileIndex;
        TInt                        iFileCount;
        RBuf                        iDeviceName;

        // Not Owned
        //
        MObexUtilsProgressObserver* iProgressObserverPtr;
        MObexUtilsDialogObserver*   iDialogObserverPtr;
    };

#endif      // COBEXUTILSDIALOG_H
            
// End of File
