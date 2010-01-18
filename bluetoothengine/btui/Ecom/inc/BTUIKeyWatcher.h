/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Watcher for PubSub and CenRep keys.
*
*/


#ifndef CBTUIKEYWATCHER_H
#define CBTUIKEYWATCHER_H

#include <e32property.h>
#include <centralrepository.h>
#include "BTUIActive.h"

//  Identification for key and active object to be watched
const TInt KBTUIPHYCountWatcher = 0x01;

/**
 *  Class CBTUIKeyWatcher
 *
 *  Notify and handle the key values' change.
 *
 *  @since S60 v5.0
 */
class CBTUIKeyWatcher : public CBase, public MBTUIActiveObserver
    {

public:

    /**
     * Two-phase constructor
	 * 
	 * @param aCaller The one calls this class.
	 * @param aServiceID Identify which key to be listened to. 
	 *                   It can be a logical AND value when listening to multiple keys. 
     */
    static CBTUIKeyWatcher* NewL( TAny* aCaller, TInt aServiceID);

    /**
     * Destructor
     */
    virtual ~CBTUIKeyWatcher();

    /**
     * From MBTUIActiveObserver.
     * Notification that a key value has changed.
     *
     * @since S60 v5.0
     * @param aActive Active object for the key which value changed.
     * @param aId Identification for key, defined in this file.
     * @param aStatus Request status
     */
    void RequestCompletedL( CBTUIActive* aActive, TInt aId, TInt aStatus );

    /**
     * From MBTUIActiveObserver.
     * Notification that a key value has changed.
     *
     * @since S60 v5.0
     * @param aActive Active object for the key which value changed.
     * @param aId Identification for key, defined in this file.
     * @param aError Error occured in Active Object's RunL().
     */
    void HandleError( CBTUIActive* aActive, TInt aId, TInt aError );

private:

    /**
     * C++ default constructor
     * @param aCaller The one calls this class.
     */
    CBTUIKeyWatcher( TAny* aCaller );

    /**
     * Symbian 2nd-phase constructor
     * @param aServiceID Identify which key to be listened to. It can 
	 *        be a logical AND value when listening to multiple keys. 
     */
    void ConstructL( TInt aServiceID );

private: // data
   	
   	/**
     * Property containing count of physical BT connections.
     * When physical connections exists, specific BT UI indicator is shown.
     */
    RProperty iPHYCountKey;
          
    /**
     * Active object for listening to above keys' change.
     * Own.
     */    
    CBTUIActive* iPHYCountWatcher;
    
    /**
     * Our caller view.
     * Not own.
     */
    TAny* iCaller; 
   
    };

#endif // CBTUIKEYWATCHER_H
