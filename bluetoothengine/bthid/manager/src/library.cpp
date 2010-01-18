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
* Description:  This is the implementation of application class
*
*/


#include <e32std.h>
#include <e32uid.h>
#include <e32svr.h>
#include <ecom/ecom.h>
#include <ecom/implementationinformation.h>

#include "hidinterfaces.h"
#include "decode.h"
#include "layout.h"
#include "library.h"
#include "debug.h"


CLayoutLibrary* CLayoutLibrary::NewL()
    {
    CLayoutLibrary* self = new (ELeave) CLayoutLibrary;

    // No two phase construction required

    return self;
    }

CLayoutLibrary::CLayoutLibrary()
        : iId(0), iLoaded(EFalse), iLayout(0)
    {
    // Nothing else to do
    }

CLayoutLibrary::~CLayoutLibrary()
    {
    TRACE_INFO( (_L("[HID]\t~CLayoutLibrary() 0x%08x"), this));
    delete iLayout;
    if (iLoaded)
        {
        iLoaded = EFalse;
        iId = 0;
        }
    REComSession::FinalClose();
    }

// ----------------------------------------------------------------------

TInt CLayoutLibrary::SetLayout(TInt aId)
    {
    TInt err = KErrNone;
    TBool foundLayout = EFalse;

    // Implementation info array
    RImplInfoPtrArray implInfoArray;

    TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: id=%d"), aId));

    if( aId == iId )
        {
        TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: Same layout id=%d, don't load twice"), aId));
        return KErrNone;
        }

    // Plain ignore is enough. If listing leaves, the array is just empty.
    TRAP_IGNORE( REComSession::ListImplementationsL( KHidLayoutPluginInterfaceUid, implInfoArray ) );

    TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: %d plugins listed"),implInfoArray.Count() ));

    if ( 0 == implInfoArray.Count())
        {
        TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: No Drivers found")));
        err = KErrNotFound;
        }
    else
        {
        for (TInt i=0; i<implInfoArray.Count() && !foundLayout; i++)
            {
            // parse implementation UID
            CImplementationInformation* info = implInfoArray[ i ];
            TUid implUid = info->ImplementationUid();
            TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: load plugin 0x%08x"),implUid ));
            // load driver
            // Trap so other drivers will be enumerated even if
            // this fails:
            CHidLayoutPluginInterface* plugin = NULL;
            TRAPD( err,
                   plugin = CHidLayoutPluginInterface::NewL( implUid );
                   TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: plugin loaded")));
                 );

            if( err == KErrNone)
                {
                CKeyboardLayout* layout = reinterpret_cast<CKeyboardLayout*>(plugin);
                if (aId == layout->LayoutId())
                    {
                    TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: Found layout %d!"), aId));
                    foundLayout = ETrue;
                    // Swap the current layout object:
                    CKeyboardLayout* tmpLayout = iLayout;
                    iLayout = layout;
                    iId = aId;
                    iLoaded = ETrue;
                    TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: delete old layout")));
                    delete tmpLayout;
                    }
                else
                    {
                    TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: wrong layout, delete plugin")));
                    delete plugin;
                    }
                }
            else
                {
                TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: plugin creation failed")));
                }
            }// end of for loop
        }

    if( !foundLayout )
        {
        TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: NO PLUGIN FOUND")));
        }
    TRACE_INFO( (_L("[HID]\tCLayoutLibrary::SetLayout: Plugin listing ready, delete info array")));
    implInfoArray.ResetAndDestroy();
    implInfoArray.Close();
    return err;
    }

// End of file

