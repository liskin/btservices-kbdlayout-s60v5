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
*  Version     : %version:  1.1.1.2.5 %
*
* Contributors:
*
* Description: 
*     Loads/unloads plugins and handles messaging between plugins and server class.
*
*/


#ifndef BTACCMANAGER_H
#define BTACCMANAGER_H

//  INCLUDES
#include <ecom/ecom.h>        // RImplInfoPtrArray

#include "BTAccClientSrv.h" // TBTAccCmd
#include "btaccTypes.h"     // BT Acc Server defines type of audio accessories


// CLASS DECLARATION
class CBTAccPlugin;
class MBTAccObserver;

/**
*  CBTSession class represent session on server side
*
*/
NONSHARABLE_CLASS(CBasrvPluginMan) : public CBase
                                    
    {
public:  // Constructors and destructor

    /**
    * Constructor.
    */
    static CBasrvPluginMan* NewL();

    /**
    * Destructor.
    */
    ~CBasrvPluginMan();

    void AccInUse();
    
	TInt AudioLinkLatency();
    void AccOutOfUse();
    
    TInt AvailablePlugins();
    
public:

    /**
    * 
    * 
    * 
    * @param    
    * @return   None
    */
    void LoadPluginsL(MBTAccObserver& aObserver);
    
    /**
    * 
    * 
    * 
    * @param    
    * @return   None
    */
    void UnloadPlugins();

    CBTAccPlugin* Plugin(TProfiles aProfile);

private: // New Functions
    CBasrvPluginMan();
    
private:    // Data
     RPointerArray<CBTAccPlugin> iPlugins;
    };

#endif      // BTACCMANAGER_H

// End of File
