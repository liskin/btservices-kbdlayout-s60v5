/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*  Version     : %version:  1.1.2.2.7 %
*
* Contributors:
*
* Description: 
*     Loads/unloads plugins and handles messaging between plugins and server class.
*
*/


// INCLUDE FILES
#include <e32std.h>             // User:: ( User class declaration )
#include <btaccPluginUid.h>
#include "btaccTypes.h"
#include "basrvpluginman.h"
#include "btaccObserver.h"
#include "btaccPlugin.h"        // Plugin interface
#include "debug.h"
#include "btaccParams.h"        // T-class consisting of parameteres sent to plugins
#include "utils.h"

//  CONSTANTS

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CBasrvPluginMan* CBasrvPluginMan::NewL()
    {
    CBasrvPluginMan* self=new(ELeave) CBasrvPluginMan();
    return self;
    }

// Destructor
CBasrvPluginMan::~CBasrvPluginMan()
    {
    TRACE_FUNC
    UnloadPlugins();
    iPlugins.Close();
   }

void CBasrvPluginMan::AccInUse()
    {
    TRACE_FUNC    
    TInt count = iPlugins.Count();
    for (TInt i = 0; i < count; i++)
        {
        iPlugins[i]->AccInUse();
        }
    }
    
void CBasrvPluginMan::AccOutOfUse()
    {
    TRACE_FUNC    
    TInt count = iPlugins.Count();
    for (TInt i = 0; i < count; i++)
        {
        iPlugins[i]->AccOutOfUse();
        }
    }

TInt CBasrvPluginMan::AvailablePlugins()
    {
    TInt avai = 0;
    TInt count = iPlugins.Count();
    for (TInt i = 0; i < count; i++)
        {
        avai |= iPlugins[i]->PluginType();
        }
    return avai;
    }
    
void CBasrvPluginMan::LoadPluginsL(MBTAccObserver& aObserver)
    {
    TRACE_FUNC
    if (iPlugins.Count())
        {
        return;
        }
    const TUid KUidBTAccPluginInterface = TUid::Uid(KBTAccPluginInterfaceUid);
    RImplInfoPtrArray implementations;
    const TEComResolverParams noResolverParams;
    REComSession::ListImplementationsL(KUidBTAccPluginInterface, 
        noResolverParams, 
        KRomOnlyResolverUid, 
        implementations);
    CleanupResetDestroyClosePushL(implementations);
    const TUint count = implementations.Count();
    
    TRACE_INFO((_L("number of implementations of plugin interface: %d"), count))
    CBTAccPlugin* plugin( NULL );
    TInt err;
    for (TUint i = 0 ; i < count; ++i)
        {
        CImplementationInformation* impl = implementations[i];
        TPluginParams params(impl->ImplementationUid(), aObserver); 
        // If loading a plugin failed,
        // the consequence is the service provided by this plugin is not available.
        // Other services that have been successfully loaded are still usable.
        TRAP( err, plugin = CBTAccPlugin::NewL(params) );
        if ( !err )
            {
            CleanupStack::PushL(plugin);
            iPlugins.AppendL(plugin);
            CleanupStack::Pop(plugin);
            }
        TRACE_INFO((_L("Load plugin \'%S\', err %d"), &(impl->DisplayName()), err ) );
        }
    
    CleanupStack::PopAndDestroy(&implementations);
    if (!iPlugins.Count())
        {
        LEAVE(KErrNotFound);
        }
    }

// ---------------------------------------------------------
// CBasrvPluginMan::UnloadPlugins
// ---------------------------------------------------------
//
void CBasrvPluginMan::UnloadPlugins()
    {
    TRACE_FUNC
    iPlugins.ResetAndDestroy();
    REComSession::FinalClose();
    }

CBTAccPlugin* CBasrvPluginMan::Plugin(TProfiles aProfile)
    {
    TInt count = iPlugins.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iPlugins[i]->PluginType() & aProfile)
            {
            return iPlugins[i];
            }
        }
    return NULL; 
    }

TInt CBasrvPluginMan::AudioLinkLatency()
	{
	TRACE_FUNC
	CBTAccPlugin* plugin = Plugin(EAnyMonoAudioProfiles);
	
	if (plugin)
		{
		return plugin->AudioLinkLatency();
		}
	else
		{
		return 0;
		}
	}

CBasrvPluginMan::CBasrvPluginMan()
    {
    }

// End of file
