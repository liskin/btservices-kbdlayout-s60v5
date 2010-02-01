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
* Description:  AT command URC handler
*
*/


#ifndef C_CHFPATURCHANDLER_H
#define C_CHFPATURCHANDLER_H

#include <atext.h>
#include "HFPAtEcomListen.h"

/**
 *  Class for AT command URC handler
 *
 *  @lib HFPatext.lib
 *  @since S60 v5.0
 */
class MATExtObserver;

NONSHARABLE_CLASS( CHFPAtUrcHandler ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aStreamCallback Callback to stream
     * @return Instance of self
     */
	static CHFPAtUrcHandler* NewL( RATExt* aAtCmdExt,
	                               MATExtObserver& aObserver );

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aStreamCallback Callback to stream
     * @return Instance of self
     */
	static CHFPAtUrcHandler* NewLC( RATExt* aAtCmdExt,
	                                MATExtObserver& aObserver );

    /**
    * Destructor.
    */
    virtual ~CHFPAtUrcHandler();

    /**
     * Resets data to initial values
     *
     * @since S60 5.0
     * @return None
     */
    void ResetData();

    /**
     * Starts waiting for an incoming URC message
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops waiting for an incoming URC message
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

    /**
     * UID of the owning plugin
     *
     * @since S60 5.0
     * @return UID of the owning plugin
     */
    TUid OwnerUid();

private:

    CHFPAtUrcHandler( RATExt* aAtCmdExt,
                      MATExtObserver& aObserver );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

// from base class CActive

    /**
     * From CActive.
     * Gets called when URC command received
     *
     * @since S60 3.2
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Gets called on cancel
     *
     * @since S60 3.2
     * @return None
     */
    void DoCancel();


private:  // data

    /**
     * AT command extension
     * Not own.
     */
    RATExt* iAtCmdExt;

    MATExtObserver& iObserver;

    /**
     * Current state of URC message handling: active or inactive
     */
    THFPState iUrcHandleState;

    /**
     * Buffer for receiving
     */
    TBuf8<KDefaultUrcBufLength> iRecvBuffer;

    /**
     * UID of the responsible ATEXT plugin
     */
    TUid iOwnerUid;

    /**
     * Package for owner UID
     */
    TPckg<TUid> iOwnerUidPckg;

    /**
     * Flag to indicate start of receiving (for ownership marking)
     */
    TBool iStarted;

    };

#endif  // C_CHFPATURCHANDLER_H
