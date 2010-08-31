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
* Description:  Definition of the global progress dialog class
*
*/



#ifndef OBEXUTILSENTRYHANDLER_H_
#define OBEXUTILSENTRYHANDLER_H_

#include <e32base.h>
#include <cmsvattachment.h>

/**
 *  Obexutils link hander
 *  Maintain the links in inbox.
 * 
 *  @since S60 v5.0
 */

NONSHARABLE_CLASS( CObexutilsEntryhandler ) : public CActive
{

public:

    static CObexutilsEntryhandler* NewL();
    static CObexutilsEntryhandler* NewLC();    

    /**
     * Destructor.
     */
    virtual ~CObexutilsEntryhandler();

    /**
     * Add an linked attachment to entry 
     *
     * @since S60 v5.0
     * @param aFilePath The absolute file path of the linked attachment file. 
     * @param anAttachInfo The attachment info associated with the file.
     * @param aStore An interface over the message store that is associated with a message entry.
     * @return error code
     */
    TInt AddEntryAttachment(const TDesC &aFilePath, CMsvAttachment* anAttachInfo, CMsvStore* aStore );
    
   
    // from base class CActive
         
    /**
     * From CActive.
     * RunL
     *
     * @since S60 v5.0
     */
    void RunL();
         
    /**
     * From CActive.
     * DoCancel
     *
     * @since S60 v5.0
     */
    void DoCancel();
    
    
private:
    /**
     * Default C++ constructor.
     */ 
    CObexutilsEntryhandler();
    
    /**
     * Symbian two-phase constructor.
     */ 
    void ConstructL();

    
private: // member data

     
    /**
     * Sync waiter object 
     */
    CActiveSchedulerWait    iSyncWaiter;


};


#endif /*OBEXUTILSENTRYHANDLER_H_*/
