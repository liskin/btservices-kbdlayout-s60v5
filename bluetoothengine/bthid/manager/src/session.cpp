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


#include <e32svr.h>

#include "layoutmgr.h"
#include "server.h"
#include "session.h"
#include "decode.h"
#include "library.h"
#include "client.h"
#include "debug.h"

// ----------------------------------------------------------------------

CLayoutSession::CLayoutSession()
    {
    // nothing else to do
    }

CLayoutSession* CLayoutSession::NewL()
    {
    CLayoutSession* self = CLayoutSession::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CLayoutSession* CLayoutSession::NewLC()
    {
    CLayoutSession* self = new (ELeave) CLayoutSession;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

void CLayoutSession::ConstructL()
    {
    iDecoder = CKeyboardDecoder::NewL();
    }

CLayoutSession::~CLayoutSession()
    {
    Server()->SessionClosed();

    delete iDecoder;
    iLayoutLibrary = 0; // no ownership
    }

// ----------------------------------------------------------------------

void CLayoutSession::ServiceL(const RMESSAGE& aMessage)
    {
    TRAPD(err, DispatchMessageL(aMessage));
    aMessage.Complete(err);
    }

void CLayoutSession::DispatchMessageL(const RMESSAGE& aMessage)
    {
    TRACE_INFO( (_L("[HID]\tCLayoutSession::DispatchMessageL(%d)\r\n"),
                 aMessage.Function()));

    // Extract the service code from the message
    TLayoutManagerService service =
        static_cast<TLayoutManagerService>(aMessage.Function());

    switch (service)
        {
        case EKeyEvent:
            {
            TPckgBuf<TKeyEventInfo> eventPkg;
            aMessage.ReadL(0, eventPkg);
            TRACE_INFO( (
                            _L("[HID]\tKey event message: 0x%04x, 0x%02x, %d, %d"),
                            eventPkg().iHidKey, eventPkg().iModifiers.Value(),
                            eventPkg().iLockKeys.iCapsLock,
                            eventPkg().iLockKeys.iNumLock));

            TPckgBuf<TDecodedKeyInfo> keyPkg;
            iDecoder->Event(eventPkg(), keyPkg());
            aMessage.WriteL(1, keyPkg);
            }
            break;

        case ESetInitialLayout:
            {
            TRACE_INFO( (_L("[HID]\tSet initial layout message\r\n")));
            Server()->SetInitialLayoutL(aMessage.Int0(),
                                        aMessage.Int1(), aMessage.Int2());
            }
            break;

        case ESetLayout:
            {
            TRACE_INFO( (_L("[HID]\tSet layout ALL message\r\n")));
            User::LeaveIfError(Server()->SetLayout(aMessage.Int0()));
            }
            break;

        case EGetInitialLayout:
            {
            TRACE_INFO( (_L("[HID]\tGet initial layout message\r\n")));
            TPckgBuf<TInt> idPkg(Server()->InitialLayout());
            aMessage.WriteL(0, idPkg);
            }
            break;

        case EGetLayout:
            {
            TRACE_INFO( (_L("[HID]\tGet layout message\r\n")));
            TInt layout = Server()->Layout();
            TRACE_INFO( (_L("  Layout library ID is %d\r\n"), layout));
            TPckgBuf<TInt> idPkg(layout);
            aMessage.WriteL(0, idPkg);
            }
            break;

        case EResetDecoder:
            {
            TRACE_INFO( (_L("[HID]\tReset decoder message\r\n")));
            iDecoder->Reset();
            }
            break;

        case EGetDeviceInfo:
            {
            TRACE_INFO( (_L("[HID]\tGet device info message\r\n")));
            TPckgBuf<TBool> isNokiaSu8Pkg(Server()->IsNokiaSu8());
            aMessage.WriteL(0, isNokiaSu8Pkg);
            TPckgBuf<TBool> foundLayoutPkg(Server()->FoundLayout());
            aMessage.WriteL(1, foundLayoutPkg);
            }
            break;

        default:
            {
            TRACE_INFO( (_L("[HID]\tUnknown service request %d\r\n"), service));
            CLayoutServer::PanicClient(aMessage, EBadRequest);
            break;
            }
        }
    }

// ----------------------------------------------------------------------

void CLayoutSession::SetLayout(CLayoutLibrary* aLayoutLibrary)
    {
    TRACE_INFO( (_L("[HID]\tCLayoutSession::SetLayout(0x%08x)\n"),
                 aLayoutLibrary));
    TRACE_INFO( (_L("[HID]\t  Layout ID is %d (0x%x)\n"),
                 aLayoutLibrary->Id()));

    iLayoutLibrary = aLayoutLibrary;
    iDecoder->SetLayout(iLayoutLibrary->Layout());
    }

// ----------------------------------------------------------------------

CLayoutServer* CLayoutSession::Server()
    {
    return const_cast<CLayoutServer*>(static_cast<const CLayoutServer*>
                                      (CSession2::Server()));
    }

// ----------------------------------------------------------------------
