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
* Description:  HID Keyboard Layout ECOM interface definition.
 *
*/


#ifndef __HIDLAYOUTPLUGININTERFACE_H__
#define __HIDLAYOUTPLUGININTERFACE_H__

#include <ecom/ecom.h>
#include "hiddebug.h"
#include "hiduids.h"

// Constant for plugin interface:
const TUid KHidLayoutPluginInterfaceUid =
    {
    LAYOUT_PLUGIN_IF
    };

/**
 * Interface class for HID Layout plugin. All plugins will implement this class.
 *
 * @lib kbdlayout.lib ?
 * @since ...
 */
class CHidLayoutPluginInterface : public CBase
    {
public:
    // Constructors & destructors

    /**
     * Creates new plugin having the given UID.
     * Uses Leave code KErrNotFound if implementation is not found.
     *
     * @param aImplementationUid Implementation UID of the plugin to be
     *        created.
     */
    inline static CHidLayoutPluginInterface* NewL(
            const TUid aImplementationUid);

    /**
     * Destructor
     */
    inline ~CHidLayoutPluginInterface();

protected:
    // New

    /**
     * C++ constructor.
     */
    inline CHidLayoutPluginInterface();

private:
    // Data

    /**
     * ECOM plugin instance UID.
     */
    TUid iDtor_ID_Key;
    };

inline CHidLayoutPluginInterface::CHidLayoutPluginInterface()
    {
    }

inline CHidLayoutPluginInterface::~CHidLayoutPluginInterface()
    {
    DBG(RDebug::Print(_L("[HID]\tCHidLayoutPluginInterface: destroying plugin 0x%08x"),iDtor_ID_Key));
    REComSession::DestroyedImplementation(iDtor_ID_Key);
    }

inline CHidLayoutPluginInterface* CHidLayoutPluginInterface::NewL(
        TUid aImplementationUid)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidLayoutPluginInterface: Loading plugin 0x%08x"),aImplementationUid.iUid));

    TAny* ptr;
    TInt32 keyOffset = _FOFF( CHidLayoutPluginInterface, iDtor_ID_Key );

    ptr = REComSession::CreateImplementationL(aImplementationUid, keyOffset);

    DEBUG_PRINT( _L( "[HID]\tCHidLayoutPluginInterface: Loading done." ) );

    return reinterpret_cast<CHidLayoutPluginInterface*> (ptr);
    }

#endif

