/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the BT bearer plugin header file.
*
*/


#ifndef __BTPLUGINPROVIDER_H__
#define __BTPLUGINPROVIDER_H__


#include <locodbearerplugin.h>


class CBTPluginNotifier;


/**
 *  BT Plug in module for local connectivity daemon.
 *
 *  This module is loaded by local connectivity daemon if the _BT feature flag 
 *  is defined. It checks BT state from central repository and starts watching 
 *  BT ON/OFF state. If BT is ON, it connects to BTEngine server. It continues 
 *  to watch the key and when it is changed, it notifies the daemon through 
 *  its callback function.
 *
 *  @lib btbearer.lib
 *  @since S60 v3.2
 */
        
NONSHARABLE_CLASS( CBTBearerPlugin ) :  public CLocodBearerPlugin
    {

public:

    /**
     * Two-phase constructor
     */
    static CBTBearerPlugin* NewL( TLocodBearerPluginParams& aParam );

    /**
     * Destructor
     */
    virtual ~CBTBearerPlugin();

private:

    /**
     * C++ default constructor
     */
    CBTBearerPlugin( TLocodBearerPluginParams& aParam );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

private: // data

    /**
     * Active object that monitors the BT state.
     * Own.
     */
    CBTPluginNotifier* iBTPluginNotifier;

    };


#endif      // __BTPLUGINPROVIDER_H__
