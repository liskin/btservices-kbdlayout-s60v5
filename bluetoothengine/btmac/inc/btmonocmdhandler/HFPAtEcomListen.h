/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ECom plugin install/uninstall/version listener
*
*/


#ifndef C_CHFPATECOMLISTEN_H
#define C_CHFPATECOMLISTEN_H

#include <atext.h>
#include <atextpluginbase.h>


/**
 *  Notification interface class for ECOM plugin interface status changes
 *
 *  @lib HFPatext.lib
 *  @since S60 v5.0
 */
 
 enum THFPState
    {
    EHFPStateIdle,
    EHFPStateAtCmdHandling,    // ATEXT state for AT command handling
    EHFPStateAtUrcHandling,    // ATEXT state for URC handling
    EHFPStateEcomListening     // ATEXT state for ECOM change listening
    };

NONSHARABLE_CLASS( MHFPAtEcomListen )
    {

public:

    /**
     * Notifies about new plugin installation
     *
     * @since S60 5.0
     * @param aPluginUid UID of installed plugin
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginInstallation( TUid& aPluginUid ) = 0;

    /**
     * Notifies about existing plugin uninstallation
     *
     * @since S60 5.0
     * @param aPluginUid UID of uninstalled plugin
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginUninstallation( TUid& aPluginUid ) = 0;

    };

/**
 *  Class for ECom plugin install/uninstall/version listener
 *
 *  @lib HFPatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CHFPAtEcomListen ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aCallback Callback to mode status change
     * @return Instance of self
     */
	static CHFPAtEcomListen* NewL( RATExt* aAtCmdExt,
                                   MHFPAtEcomListen* aCallback );

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aCallback Callback to mode status change
     * @return Instance of self
     */
    static CHFPAtEcomListen* NewLC( RATExt* aAtCmdExt,
                                    MHFPAtEcomListen* aCallback );
	
    /**
    * Destructor.
    */
    virtual ~CHFPAtEcomListen();

    /**
     * Resets data to initial values
     *
     * @since S60 5.0
     * @return None
     */
    void ResetData();

    /**
     * Starts waiting for ECom plugin install/uninstall/version status changes
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops waiting for Ecom plugin install/uninstall/version status changes
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CHFPAtEcomListen( RATExt* aAtCmdExt,
                      MHFPAtEcomListen* aCallback );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 5.0
     * @return None
     */
    void Initialize();

// from base class CActive

    /**
     * From CActive.
     * Gets called when plugin installed, uninstalled or changed
     *
     * @since S60 5.0
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Gets called on cancel
     *
     * @since S60 5.0
     * @return None
     */
    void DoCancel();

private:  // data

    /**
     * AT command extension
     * Not own.
     */
    RATExt* iAtCmdExt;

    /**
     * Callback to call when plugin installed, uninstalled or changed
     * Not own.
     */
    MHFPAtEcomListen* iCallback;

    /**
     * Current state of ECom interface listening: active or inactive
     */
    THFPState iEcomListenState;

    /**
     * UID of the installed, uninstalled or changed plugin
     */
    TUid iPluginUid;

    /**
     * Package for plugin UID
     */
    TPckg<TUid> iPluginUidPckg;

    /**
     * Package for ECOM type
     */
    TPckg<TATExtensionEcomType> iEcomTypePckg;

    };

#endif  // C_CHFPATECOMLISTEN_H
