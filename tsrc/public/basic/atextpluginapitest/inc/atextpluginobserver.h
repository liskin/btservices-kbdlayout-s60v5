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
* Description:  
*
*/


#ifndef ATEXTENSIONPLUGINOBSERVER_H
#define ATEXTENSIONPLUGINOBSERVER_H

#include <ecom/ecom.h>

class CATExtPluginBase;

/**
* The interface for AT Extension Plugin to interact with AG.
*/
class MATExtPluginObserver
    {
public:

    /** 
    * Call by extension plugins when there is an unsolicited result code 
    * should be sent to target headset.
    * 
    * Each AT Extension Plugin is responsible to format
    * result codes properly, e.g. in BT HFP case, the
    * format should be <cr><lf><result code><cr><lf>
    * 
    * @param aAT the unsolicited result code to be sent.
    */
    virtual TInt SendUnsolicitedResult(CATExtPluginBase& aPlugin, const TDesC8& aAT) = 0;

    /** 
    * Called by Extension Plugins to inform AG that a command
    * handling has been completed or rejected.
    *
    * @return KErrNotSupported if the AT Extension Plugin doesn’t
    *         support the command passed in CATExtPluginBase::HandleCommand();
    *         KErrNone otherwise. No other error code is allowed.
    */
    virtual void HandleCommandCompleted(CATExtPluginBase& aPlugin, TInt aErr) = 0;

    /**
    * Called by the destructor of CATExtPluginBase.
    * A concrete service provider implementation should not touch this.
    *
    * @param aATExt the reference of the service provider.
    */
    virtual void ATExtPluginClosed(CATExtPluginBase& aPlugin) = 0;
    
    };

#endif    