/*
* Copyright (c) 2007-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the Disconnection helper class
*
*/


#ifndef DISCONNECTHELPER_H
#define DISCONNECTHELPER_H

#include <e32base.h>

/**
 *  Callback interface for completing disconnection helper
 *
 *  @since S60 v3.2
 */
class MDisconnectionCallback
    {

public:

    /**
     * From MDisconnectionCallback.
     * Callback funtion for completing asynchronous disconnection request
     *
     * @since S60 v3.2
     */
    virtual void CompleteDisconnection()=0;
    };


/**
 *  Helper class for handling asynchronous disconnection request
 *
 *  @since S60 v3.2
 */
class CDisconnectHelper : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aParams the LocodServicePluginParams
     */
    static CDisconnectHelper* NewL( MDisconnectionCallback* aObserver );

    /**
     * Destructor.
     */
    ~CDisconnectHelper();

    /**
     * Activate helper class for handling asynchronous disconnection
     *
     * @since S60 v3.2
     * @return Error code
     */
     TInt Activate();

    /**
     * From CActive
     *
     * @since S60 v3.2
     */
     void RunL();

    /**
     * From CActive
     *
     * @since S60 v3.2
     */
     void DoCancel();

private:

    CDisconnectHelper( MDisconnectionCallback* aObserver );

    void ConstructL();

private: // data

    /*
     * Callback interface, which is used when disconnect helper
     * is completed.
     * Not own.
     */
    MDisconnectionCallback* iObserver;

    };

#endif // BTPBAPPLUGIN
