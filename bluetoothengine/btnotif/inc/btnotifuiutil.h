/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares Bluetooth notifiers UI utility class.
*
*/

#ifndef BTNOTIFUIUTIL_H
#define BTNOTIFUIUTIL_H
#include <eikenv.h>          // Eikon environment
#include <data_caging_path_literals.hrh> 
#include <AknQueryDialog.h>
#include <AknWaitDialog.h>
#include <secondarydisplay/BTnotifSecondaryDisplayAPI.h>

// CONSTANTS
// Literals for resource and bitmap files ( drive, directory, filename(s) )
_LIT(KFileDrive,"z:");
_LIT(KResourceFileName, "btnotif.rsc");

// FORWARD DECLARATIONS
class CEikonEnv;

NONSHARABLE_CLASS( CBTNotifUIUtil ) : public CBase
    {
    public: // Constructors and destructor

        static CBTNotifUIUtil* NewL( TBool aCoverDisplayEnabled );
        
        /**
        * Destructor.
        */
        virtual ~CBTNotifUIUtil();

    private: // Constructors and destructor

        /**
        * C++ default constructor.
        */
        CBTNotifUIUtil( TBool aCoverDisplayEnabled );

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();
        
    public:
        /**
        * A utility function for requesting the activation of lights.
        * @param None.
        * @return None.
        */
        void TurnLightsOn();
        
        /**
         * Get the local eikon environment.
         * @return the reference of the local Eikon Environment
         */
        CEikonEnv& LocalEikonEnv();
        
        /**
         * Common utility for user confirmation dialog.
         * @return the keypress value
         */
        TInt ShowQueryL(TInt aPromptResource, TInt aExecuteResource, 
                TSecondaryDisplayBTnotifDialogs aDialogId, 
                CAknQueryDialog::TTone aTone = CAknQueryDialog::ENoTone );
        
        /**
         * Common utility for user confirmation dialog.
         * @return the keypress value
         */
        TInt ShowQueryL(const TDesC& aPrompt, TInt aExecuteResource, 
                TSecondaryDisplayBTnotifDialogs aDialogId, 
                const TBTDeviceName& aDevNameInCoverUI,
                CAknQueryDialog::TTone aTone = CAknQueryDialog::ENoTone );
        
        /**
         * Common utility for user confirmation dialog.
         * @return the keypress value
         */
        TInt ShowMessageQueryL(TDesC& aMessage, const TDesC& aHeader,
                TInt aResourceId, 
                CAknQueryDialog::TTone aTone = CAknQueryDialog::ENoTone );        
        /**
         * Common utility for user confirmation dialog.
         * @return the keypress value
         */
        TInt ShowTextInputQueryL(TDes& aText, TInt aExecuteResource, 
                TSecondaryDisplayBTnotifDialogs aDialogId, 
                CAknQueryDialog::TTone aTone = CAknQueryDialog::ENoTone );        
        
        /**
         * Common utility for user confirmation dialog.
         * @return the keypress value
         */
        TInt ShowTextInputQueryL(TDes& aText, const TDesC& aPrompt, TInt aExecuteResource, 
                TSecondaryDisplayBTnotifDialogs aDialogId, 
                CAknQueryDialog::TTone aTone = CAknQueryDialog::ENoTone );        

        /**
         * Common utility for UI inforamtion note.
         */
        void ShowInfoNoteL( TInt aResourceId, TSecondaryDisplayBTnotifDialogs aDialogId  ); 
        
        /**
         * Common utility for UI inforamtion note.
         */
        void ShowInfoNoteL( const TDesC& aResource, TSecondaryDisplayBTnotifDialogs aDialogId  );
        
        /**
         * Common utility for Confirmation note.
         */
        void ShowConfirmationNoteL( TBool aWaitingDlg, const TDesC& aResource, 
                TSecondaryDisplayBTnotifDialogs aDialogId, const TBTDeviceName& aDevNameInCoverUi );
        
        /**
         * Common utility for Error note.
         */
        void ShowErrorNoteL( TBool aWaitingDlg, const TDesC& aResource, 
                TSecondaryDisplayBTnotifDialogs aDialogId, TBTDeviceName aDevNameInCoverUi );
                
        /**
         * Check if iYesNoDlg is NULL
         */
        TBool IsQueryReleased();

        /**
         * Update the dialog when UpdateL() is called by client.
         */
        void UpdateQueryDlgL( TDesC& aMessage );
        
        /**
         * Update the message dialog when UpdateL() is called by client.
         */
        void UpdateMessageQueryDlgL( TDesC& aMessage );
        
        /**
         * It is used for Secondary display to update. 
         */
        void UpdateCoverUiL( const TDesC8& aMessage );
        
        /**
         * Delete the dialog, mostly used by UpdateL()
         */
        void DismissDialog();
        
        TInt ShowWaitDlgL( TInt aResourceId );
        
        void CompleteWaitDlgL();
        
    private:
    
        void CoverUIDisplayL( const MObjectProvider* aMop, const TBTDeviceName& aDevNameInCoverUi );    
    
    private: //DATA
        CEikonEnv*          iEikEnv;    // Local eikonenv
        TInt                iResourceFileFlag;      // Flag for eikon env.
        CAknQueryDialog*    iQueryDlg;  // All kinds of query dialog 
        CAknWaitDialog*     iWaitDlg;   // WaitDialog
        TBool iCoverDisplayEnabled; // Flag that indicate cover UI featur
        TBool               iSystemCancel; // Used to check if iEikEnv is valid. 
        TBool               iAppKeyBlocked; // Apps key is deactivated.
    };
#endif //BTNOTIFUIUTIL_H


