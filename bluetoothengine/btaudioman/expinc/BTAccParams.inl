/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*             Parameter provided to BT Accessory Server Plugins
*
*/


// INCLUDE FILES


inline TPluginParams::TPluginParams(const TUid aImplementationUid, MBTAccObserver& aObserver)
:    iImplementationUid(aImplementationUid),
    iObserver(aObserver)
    {
    
    }

inline TPluginParams::~TPluginParams()
    {

    }

inline TUid TPluginParams::ImplementationUid() const
    {
    //TRACE_OPT(KPRINTFTRACE, DebugPrint(_L("[BTAccServer]\t TPluginParams::ImplementationUid() ")));
    return iImplementationUid;
    }

inline MBTAccObserver& TPluginParams::Observer() const
    {
    return iObserver;
    }

//
// End of file
