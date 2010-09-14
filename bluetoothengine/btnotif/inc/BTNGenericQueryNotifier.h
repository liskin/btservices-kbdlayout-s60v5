/*
 * Copyright (c) 2002, 2010 Nokia Corporation and/or its subsidiary(-ies).
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
 * Description:  Declares Generic Query Notifier Class.
 *
 */

#ifndef BTNGENERICQUERYNOTIFIER_H
#define BTNGENERICQUERYNOTIFIER_H

// INCLUDES

#include "btnotifier.h" // Base class
#include "btnotifactive.h"

// FORWARD DECLARATIONS

// CLASS DECLARATION
NONSHARABLE_CLASS(CBTGenericQueryNotifier): public CBTNotifierBase,
		public MBTNotifActiveObserver
	{
public:
	// Constructors and destructor

	/**
	 * Two-phased constructor.
	 */
	static CBTGenericQueryNotifier* NewL();

	/**
	 * Destructor.
	 */
	virtual ~CBTGenericQueryNotifier();

private:
	// Functions from base classes

	/**
	 * From CBTNotifierBase Called when a notifier is first loaded
	 * to allow any initial construction that is required.
	 * @param None.
	 * @return A structure containing priority and channel info.
	 */
	TNotifierInfo RegisterL();

	/** From CBTNotifierBase Synchronic notifier launch.
	 * @param aBuffer Received parameter data.
	 * @return A pointer to return value.
	 */
	TPtrC8 StartL(const TDesC8& aBuffer);

    /**
    * From CBTNotifierBase
    * Used in asynchronous notifier launch to start the actual processing
    * of parameters received in StartL. StartL just stores a copy of the
    * parameter buffer, schedules a callback to call this method and returns
    * ASAP, so the actual work begins here, safely outside of the StartL
    * context so that waiting dialogs can be freely used if need be.
    * @return None.
    */
	void ProcessStartParamsL();

	/**
	 * From CBTNotifierBase Updates a currently active notifier.
	 * @param aBuffer The updated data.
	 * @return A pointer to return value.
	 */
	TPtrC8 UpdateL(const TDesC8& aBuffer);

	/**
	 * From MBTNotifActiveObserver
	 * Gets called when P&S key notifies change.
	 */
	void RequestCompletedL(CBTNotifActive* aActive, TInt aId, TInt aStatus);

	/**
	 * From MBTNotifActiveObserver
	 */
	void HandleError(CBTNotifActive* aActive, TInt aId, TInt aError);

	/**
	 * From MBTNotifActiveObserver
	 */
	void DoCancelRequest(CBTNotifActive* aActive, TInt aId);

private:

	/**
	 * Parse the data out of the message that is sent by the client of the notifier.
	 * @param aBuffer A package buffer containing received parameters.
	 * @return None.
	 */
	void ProcessParamBufferL(const TDesC8& aBuffer);

	/**
	 * C++ default constructor.
	 */
	CBTGenericQueryNotifier();

	/**
	 * Symbian 2nd phase constructor.
	 */
	void ConstructL();

	/**
	 * Show query and complete message.
	 */
	void ShowQueryAndCompleteL();

private:

	RBuf iQueryMessage;
	HBufC* iQueryHeader;
	HBufC* iName;

	TInt iMessageResourceId;
	TSecondaryDisplayBTnotifDialogs iSecondaryDisplayCommand;
	TBool iIsMessageQuery;
	TBool iIsNameQuery;
	RProperty iPhonePowerKey;
	CBTNotifActive* iPhonePowerKeyWatcher;
	};

#endif

// End of File
