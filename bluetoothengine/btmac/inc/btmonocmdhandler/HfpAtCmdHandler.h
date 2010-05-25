/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  AT extension API
*
*/


#ifndef BTATEXTAGENT_H
#define BTATEXTAGENT_H

//  INCLUDES
#include <e32base.h>
#include <atext.h>
#include "btmcactive.h"
#include "HFPAtEcomListen.h"
#include "HFPAtUrcHandler.h"

class CBtmcActive;

class MATExtObserver
    {
public:
    virtual void ATExtHandleReplyReceivedL(TInt aErr, const TDesC8& aReply) = 0;

    virtual void UnsolicitedResultFromATExtL(TInt aErr, const TDesC8& aAT) = 0;
    };

NONSHARABLE_CLASS( CHFPAtCmdHandler ) : public CBase, 
                                        public MBtmcActiveObserver,
                                        public MHFPAtEcomListen
	{
public:

    static CHFPAtCmdHandler* NewL(MATExtObserver& aObserver);

public:

	~CHFPAtCmdHandler();
	
	void HandleCommand(const TDesC8& aAT, const TDesC8& aReply = KNullDesC8);
	

	
private:

    void RequestCompletedL(CBtmcActive& aActive, TInt aErr);

    void CancelRequest(TInt aServiceId);

    TInt HandleRunError(TInt aErr);
    


private:

	CHFPAtCmdHandler(MATExtObserver& aObserver);
	
    /**
     * Creates plugin handlers for this class
     *
     * @since S60 5.0
     * @return None
     */
    void CreatePluginHandlersL();
	
    /**
     * Instantiates one URC message handling class instance and adds it to
     * the URC message handler array
     *
     * @since S60 3.2
     * @return None
     */
    CHFPAtUrcHandler* AddOneUrcHandlerL();

    /**
     * Deletes all instantiated URC message handlers
     *
     * @since S60 5.0
     * @return None
     */
    void DeletePluginHandlers();
    
    // from base class MHFPAtEcomListen

    /**
      * From MHFPAtEcomListen.
      * Notifies about new plugin installation
      *
      * @since S60 5.0
      * @return None
      */
    TInt NotifyPluginInstallation( TUid& aPluginUid );

    /**
      * From MHFPAtEcomListen.
      * Notifies about existing plugin uninstallation
      *
      * @since S60 5.0
      * @return None
      */
    TInt NotifyPluginUninstallation( TUid& aPluginUid );
    
    /**
     * Starts URC message handling
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StartUrc();

    /**
     * Stops URC message handling
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StopUrc();
	
	void ConstructL();
	
private:
    MATExtObserver& iObserver;
    
    RATExt iATExtClient;
    TBuf8<KDefaultCmdBufLength> iCmdBuffer;
    TBuf8<KDefaultCmdBufLength> iRecvBuffer;
    TBuf8<KDefaultCmdBufLength> iReplyBuffer;
    TBuf8<512> iSystemReply;
    
    TInt iRemainingReplyLength;
    TPckg<TInt> iRemainingReplyLengthPckg;
    TATExtensionReplyType iReplyType;
    TPckg<TATExtensionReplyType> iReplyTypePckg;
    CBtmcActive* iCommander; // for command handling
   
        /**
     * URC message handlers
     * Own.
     */
    RPointerArray<CHFPAtUrcHandler> iUrcHandlers;

    /**
     * ECOM plugin interface status change listener
     * Own.
     */
    CHFPAtEcomListen* iEcomListen;
	};

#endif      // BTATEXTAGENT_H
            
// End of File
