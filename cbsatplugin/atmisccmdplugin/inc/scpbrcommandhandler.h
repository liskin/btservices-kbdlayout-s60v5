/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 * 
 * Description :
 * 
 * CSCPBRCommandHandler class declaration for AT^SCPBR command
 * 
 */

#ifndef SCPBRCOMMANDHANDLER_H_
#define SCPBRCOMMANDHANDLER_H_

#include <e32base.h>
#include <etelmm.h>
#include <e32cmn.h>
#include <rmmcustomapi.h>

#include "atcmdasyncbase.h"
#include "atmisccmdpluginconsts.h"

class CPhoneBookBuffer;

/**
 *  AT^SCPBR command handler implementation class
 */
NONSHARABLE_CLASS( CSCPBRCommandHandler ) : public CATCmdAsyncBase
	{
private:

	/**
	 * SCPBR States
	 */
	enum TSCPBRState
	    {
        ESCPBRStateIdle,                  // Idle
        ESCPBRStateRead,                  // Read
        ESCPBRStateGetPhoneBookInfo,      // Get phone book info.
        ESCPBRStateGet3GPBInfo,           // Get 3G phone book info.
        ESCPBRStateNotSupported           // phone book store not supported
	    };
private:
	/**
	 *  The data structure of phone book entry.
	 *  Inner class used by CSCPBRCommandHandler only.
	 */
	class TPhoneBookEntry
	    {
    public:
	    TPhoneBookEntry(): iIndex(-1)
            {
            }
	    
	    void Externalize( TDes8& aDes ) const;

        TInt iIndex;

        TBuf8<KSCPBRMaxNumberLength> iNumber1;
        TBuf8<KSCPBRMaxNumberLength> iNumber2;
        TBuf8<KSCPBRMaxNumberLength> iNumber3;
        TBuf8<KSCPBRMaxNumberLength> iNumber4;

        TBuf8<KSCPBRMaxNameLength> iName;

        TBuf8<KSCPBRMaxEmailLength> iEmail;
	    };
	
public:
	static CSCPBRCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone );
	
	virtual ~CSCPBRCommandHandler();

private: // from CATCmdAsyncBase
	virtual void HandleCommand(const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded);
	
private: // from CActive
	virtual void RunL();
	virtual void DoCancel();
	virtual TInt RunError(TInt aError);
	
private:	
	CSCPBRCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone );
	void ConstructL();

	void ExtractEntriesL();
	void CopyToPhonebookEntryField(TDes8& aDest, const TDesC16& aSrc);
	void FormatReplyL();
	TInt ParseParameters();
	void AppendEntryL(const TPhoneBookEntry& aEntry);
	
private:
	/**
	 * The first index to be read from phonebook store.
	 */
    TInt iIndex1;

	/**
	 * The last index to be read from phonebook store.
	 */
    TInt iIndex2;

	/**
	 * The reply
	 */
    RBuf8 iReply;

	/**
	 * The phone book store. used to get entries information and entries.
	 */
    RMobilePhoneBookStore iPhoneBookStore;
    
	/**
	 * used to save the entries information.
	 */
    RArray<TPhoneBookEntry> iEntries;

	/**
	 * Used to parse the content get from PhoneBookStore.
	 */
    CPhoneBookBuffer* iPhoneBookBuffer;
    
	/**
	 * The buffer to get phonebookstore entries content.
	 */
    RBuf8 iContactsBuf; 
    
	/**
	 * Current state
	 */
    TSCPBRState iState;
    
	/**
	 * The PhonebookInfo, used to get phone book entries information.
	 */
    RMobilePhoneBookStore::TMobilePhoneBookInfoV1 iPhoneBookInfo;
    
    /**
     * Total entries supported by phonebook store.
     */
    TInt iTotalEntries;
	/**
	 * The max number length supported
	 */
    TInt iNLength;

	/**
	 * The max email length supported.
	 */
    TInt iMLength;

	/**
	 * The max text length supported.
	 */
    TInt iTLength;
    
	/**
	 * Used to get 3GPBInfo.
	 */ 
    RMmCustomAPI iCustomApi;
    
	/**
	 * Used to get 3GPBInfo.
	 */
    RMmCustomAPI::T3GPBInfo i3GPBInfo;

    };

#endif /* SCPBRCOMMANDHANDLER_H_ */
