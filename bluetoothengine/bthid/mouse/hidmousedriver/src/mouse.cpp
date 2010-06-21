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
#include <e32svr.h>
#include <w32std.h>
#include <coedef.h>
#include <eiksvdef.h>
#include <apgcli.h>
#include <apacmdln.h>
#include <apgtask.h>
#include <coemain.h>

#include <e32property.h>
#include "bthidPsKey.h"

#include "hidtranslate.h"
#include "finder.h"
#include "mouse.h"
#include "debug.h"


#ifndef DBG
#ifdef _DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif
#endif

// Application UIDs for PaintCursor app.
_LIT(KAppName, "PaintCursor.exe");
// ----------------------------------------------------------------------

CHidMouseDriver::CHidMouseDriver(MDriverAccess* aGenericHid)
        : iDriverState(EUninitialised), iGenericHid(aGenericHid),
        iButtonDown (EFalse), iButton2Down(EFalse)
    {
    DBG(RDebug::Print(
            _L("[HID]\tCHidMouseDriver::CHidMouseDriver ENTER")));
    DBG(RDebug::Print(
            _L("[HID]\tCHidMouseDriver::CHidMouseDriver(0x%08x)"), aGenericHid));
    }

CHidMouseDriver* CHidMouseDriver::NewLC(MDriverAccess* aGenericHid)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::NewLC(0x%08x)"), aGenericHid));
    CHidMouseDriver* self =
        new (ELeave) CHidMouseDriver(aGenericHid);
    CleanupStack::PushL(self);
    self->ConstructL();

    return self;
    }

CHidMouseDriver* CHidMouseDriver::NewL(MDriverAccess* aGenericHid)
    {
    CHidMouseDriver* self =
        CHidMouseDriver::NewLC(aGenericHid);
    CleanupStack::Pop();
    return self;
    }

void CHidMouseDriver::ConstructL()
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver{0x%08x}::ConstructL() BEGIN"), this));

    User::LeaveIfNull(iGenericHid);

    TInt err = iPointBufQueue.OpenGlobal(KMsgBTMouseBufferQueue);   
    if (err == KErrNotFound)
        {
        User::LeaveIfError(iPointBufQueue.CreateGlobal(KMsgBTMouseBufferQueue, KPointQueueLen));    
        }
    else
        {
        User::LeaveIfError( err );
        } 
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver{0x%08x}::ConstructL() END"), this));
    // Create a session with the window server:
    User::LeaveIfError(iWsSession.Connect());
    }

CHidMouseDriver::~CHidMouseDriver()
    {
    DBG(RDebug::Print(_L("[HID]\t~CHidMouseDriver() 0x%08x"), this));

    iPointBufQueue.Close();

    if (iDriverState == EInitialised || iDriverState == EDisabled )
        {
        RProperty::Set( KPSUidBthidSrv, KBTMouseCursorState, ECursorNotInitialized );
        }

    iWsSession.Close();
    }

void CHidMouseDriver::MoveCursor(const TPoint& aPoint)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::MoveCursor()")));
    PostPointer(aPoint);
    }

// ---------------------------------------------------------------------------
// CHidMouseDriver::PostPointer
// Save the event to the buffer
// ---------------------------------------------------------------------------
//
TInt CHidMouseDriver::PostPointer(const TPoint& aPoint)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::PostPointer()")));
    iPointerBuffer.iPoint[iPointerBuffer.iNum] = aPoint;
    iPointerBuffer.iType[iPointerBuffer.iNum]  = KBufferPlainPointer;
    iPointerBuffer.iNum++;
    TInt ret = KErrNone;

    if(iPointerBuffer.iNum > KPMaxEvent)
        {
        ret = iPointBufQueue.Send(iPointerBuffer);
        iPointerBuffer.iNum = 0;
        }
    return ret;
    }

TInt CHidMouseDriver::SendButtonEvent(TBool aButtonDown)
    {

    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::SendButtonEvent() %d"), aButtonDown));
    iPointerBuffer.iPoint[iPointerBuffer.iNum] = TPoint (0,0);
    iPointerBuffer.iType[iPointerBuffer.iNum]  = aButtonDown ? KBufferPenDown : KBufferPenUp;
    iPointerBuffer.iNum++;
    TInt ret = iPointBufQueue.Send(iPointerBuffer);
    iPointerBuffer.iNum = 0;
    return ret;
    }

void CHidMouseDriver::StartL(TInt /*aConnectionId*/)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::StartL()")));

    // Ready to process mouse events:
    iDriverState = EInitialised;
    LaunchApplicationL(KAppName);
    RProperty::Set( KPSUidBthidSrv, KBTMouseCursorState, ECursorShow );
    }



void CHidMouseDriver::InitialiseL(TInt aConnectionId)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::InitialiseL(%d)"),
                      aConnectionId));

    // Store the connection ID:
    iConnectionId = aConnectionId;
    }

void CHidMouseDriver::Stop()
    {
    iDriverState = EDisabled;
    RProperty::Set( KPSUidBthidSrv, KBTMouseCursorState, ECursorNotInitialized );
    }

//----------------------------------------------------------------------------
// CHidMouseDriver::LaunchApplicationL
//----------------------------------------------------------------------------
//
void CHidMouseDriver::LaunchApplicationL(const TDesC& aName)
    {
    //Check if application is already running in the background
    if (IsAlreadyRunning())
        {
        // Application is active, so just bring to foreground
        }
    else
        {
        // If application is not running, then create a new one
        CApaCommandLine* cmd = CApaCommandLine::NewLC();

        cmd->SetExecutableNameL(aName);
        cmd->SetCommandL(EApaCommandBackground); // EApaCommandRun

        RApaLsSession arcSession;
        //connect to AppArc server
        User::LeaveIfError(arcSession.Connect());
        CleanupClosePushL(arcSession);
        User::LeaveIfError( arcSession.StartApp(*cmd) );
        arcSession.Close();

        CleanupStack::PopAndDestroy(2);
        }
    }

TBool CHidMouseDriver::IsAlreadyRunning()
    {
    TFindProcess processFinder(_L("*[2001fe5c]*")); //search by paintcursor.exe UID3  

    TBool found = EFalse;
    TFullName result;
    if (processFinder.Next(result) == KErrNone)
        {
        DBG(RDebug::Print(_L("[BTHID] CHidMouseDriver::IsAlreadyRunning - process found Inside while")) );
        found = ETrue;
        }

    if (found)
        {
        DBG(RDebug::Print(_L("[BTHID] CHidMouseDriver::IsAlreadyRunning - Process found ")) );
        }
    else
        {
        DBG(RDebug::Print(_L("[BTHID] CHidMouseDriver::IsAlreadyRunning - Process was never found")) );
        }
        
    return found;
}


// ----------------------------------------------------------------------
// CHidDriver mandatory functions:

TInt CHidMouseDriver::DataIn(
    CHidTransport::THidChannelType aChannel, const TDesC8& aPayload)
    {
    TInt err = KErrNone;
    switch (aChannel)
        {
        case CHidTransport::EHidChannelInt:
        if (EInitialised == iDriverState)
            {
            TInt mouseStatus;
            TInt err = RProperty::Get( KPSUidBthidSrv, KBTMouseCursorState, mouseStatus );
            if ( !err &&
                ((static_cast<THidMouseCursorState>(mouseStatus) == ECursorRedraw)|| 
                 (static_cast<THidMouseCursorState>(mouseStatus) == ECursorReset)) )
                {
                err = RProperty::Set( KPSUidBthidSrv, KBTMouseCursorState, ECursorShow );
                DBG(RDebug::Print(
                         _L("[BTHID]\tCHidMouseDriver::DataIn() ECursorRedraw ||ECursorReset ")) );
                }

            LaunchApplicationL(KAppName);

            CursorRedraw();

            InterruptData(aPayload);
            }
        break;

        case CHidTransport::EHidChannelCtrl:
        break;

        default:
        break;
        }
    return err;
    }

void CHidMouseDriver::CommandResult(TInt /*aCmdAck*/)
    {
    // No implementation as we don't issue any requests to be acknowledged
    }

void CHidMouseDriver::Disconnected(TInt aReason)
    {
    RDebug::Print(_L("[HID]\tCHidMouseDriver::Disconnected(%d)"), aReason);
    Stop();
    }

// ----------------------------------------------------------------------

void CHidMouseDriver::InterruptData(const TDesC8& aPayload)
    {
    // If the report has a report ID, it is in the first byte.
    // If not, this value is ignored (see CField::IsInReport()).
    //
    TInt firstByte = aPayload[0];

    DBG(for (TInt ii = 0; ii < aPayload.Length(); ii++)
        {
        TInt nextByte = aPayload[ii];
        DBG(RDebug::Print(
                _L("[HID]\tCHidKeyboardDriver::InterruptData()  report[%d] =  %d"),
                ii, nextByte));
        })

    DBG(RDebug::Print(
            _L("[HID]\tCHidMouseDriver::InterruptData(), report %d, length %d"),
            firstByte, aPayload.Length()));

    if (iField[EXY] && iField[EXY]->IsInReport(firstByte))
        {
        UpdateXY(EXY, aPayload);
        }

    if (iField[EButtons] && iField[EButtons]->IsInReport(firstByte))
        {
        UpdateButtons(EButtons, aPayload);
        }
    if (iField[EXY] && iField[EXY]->IsInReport(firstByte))
        {
        UpdateWheel(EWheel, aPayload);
        }
    }

// ----------------------------------------------------------------------

void CHidMouseDriver::UpdateXY(TInt aFieldIndex,
                               const TDesC8& aReport)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateModifiers()")));

    // Translate the HID usage values into a boot protocol style
    // modifier bitmask:
    //
    TReportTranslator report(aReport, iField[aFieldIndex]);

    TInt Xvalue = 0;
    TInt Yvalue = 0;

    TInt errX = report.GetValue( Xvalue, EGenericDesktopUsageX);
    TInt errY = report.GetValue( Yvalue, EGenericDesktopUsageY);

    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateXY (%d,%d)"), Xvalue, Yvalue));
    if ( ( Xvalue != 0 ) || (Yvalue != 0))
        {
        MoveCursor(TPoint(Xvalue, Yvalue));
        }

    }

void CHidMouseDriver::UpdateWheel(TInt aFieldIndex,
                                  const TDesC8& aReport)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateModifiers()")));

    // Translate the HID usage values into a boot protocol style
    // modifier bitmask:
    //
    TReportTranslator report(aReport, iField[aFieldIndex]);

    TInt Yvalue = 0;

    TInt errY = report.GetValue( Yvalue, EGenericDesktopUsageWheel);
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateWheel (%d)"), Yvalue));

    TInt absValue(Abs(Yvalue));
    if ( (errY == KErrNone) && (absValue >= 1) )
        {
        TRawEvent rawEvent;
        for (TInt ii = 0; ii < absValue; ii++)
            {
            rawEvent.Set(TRawEvent::EKeyDown, (Yvalue > 0) ? EStdKeyUpArrow : EStdKeyDownArrow);
            UserSvr::AddEvent(rawEvent);
            rawEvent.Set(TRawEvent::EKeyUp, (Yvalue > 0) ? EStdKeyUpArrow : EStdKeyDownArrow);
            UserSvr::AddEvent(rawEvent);
            }
        }
    }

void CHidMouseDriver::UpdateButtons(TInt aFieldIndex,
                                    const TDesC8& aReport)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateButtons()")));

    // Translate the HID usage values into a boot protocol style
    // modifier bitmask:
    //
    TReportTranslator report(aReport, iField[aFieldIndex]);

    TInt button1 = 0;
    TInt button2 = 0;
    TInt button3 = 0;

    const TInt KButton1 = 1;
    const TInt KButton2 = 2;
    const TInt KButton3 = 3;
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateButtons() %d, %d, %d"),
                      iField[aFieldIndex]->UsagePage(),
                      iField[aFieldIndex]->UsageMin(),
                      iField[aFieldIndex]->UsageMax()));

    TBool buttonPressed(EFalse);
    if (report.GetValue( button1, KButton1) == KErrNone && button1)
        {
        DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateButtons() Button1")));
        buttonPressed = ETrue;
        }

    if (report.GetValue( button2, KButton2) == KErrNone && button2)
        {
        DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateButtons() Button2")));
        if (! iButton2Down )
            {
            iButton2Down = ETrue;
            TRawEvent rawEvent;
            rawEvent.Set(TRawEvent::EKeyDown, EStdKeyApplication0);
            CursorRedraw();
            UserSvr::AddEvent(rawEvent);
            }
        }
    else
        {
        if (iButton2Down )
            {
            iButton2Down = EFalse;
            TRawEvent rawEvent;
            rawEvent.Set(TRawEvent::EKeyUp, EStdKeyApplication0);
            CursorRedraw();
            UserSvr::AddEvent(rawEvent);
            }
        }

    if (report.GetValue( button3, KButton3) == KErrNone && button3)
        {
        DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::UpdateButtons() Button3")));
        buttonPressed = ETrue;
        }

    if (buttonPressed)
        {
        if ( !iButtonDown )
            {
            iButtonDown = ETrue;
            SendButtonEvent(ETrue);//Send Mouse Button Down
            }
        }
    else
        {
        if ( iButtonDown )
            {
            iButtonDown = EFalse;
            SendButtonEvent(EFalse);//Send Mouse Button Up
            }
        }
    }

TInt CHidMouseDriver::CanHandleReportL(CReportRoot* aReportRoot)
    {
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::CanHandleReport(0x%08x)"),
                      aReportRoot));
    iSupportedFieldCount = 0;
    // Look for mouse reports:

    THidFieldSearch search;

    TMouseFinder finder;
    search.SearchL(aReportRoot, &finder);
    DBG(RDebug::Print(_L("[HID]\tCHidMouseDriver::CanHandleReport(): root parsed")));
    iField[EButtons] = finder.ButtonsField();
    iField[EXY] = finder.XYField();
    iField[EWheel] = finder.WheelField();

    for (TInt i=0; i<KNumInputFieldTypes; ++i)
        {
        if (iField[i])
            {
            iSupportedFieldCount++;
            }
        }
    
    TInt valid = KErrHidUnrecognised;

    // We only require buttons and xy field reports; the
    // Wheel, LED and consumer keys are optional:

    if ((iField[EButtons] != 0) && (iField[EXY] != 0))
        {
        valid = KErrNone;
        }

    DBG(RDebug::Print(
            _L("[HID]\tCHidMouseDriver::CanHandleReport() returning %d"), valid));

    return valid;
    }


//----------------------------------------------------------------------------
// CHidMouseDriver::SupportedFieldCount
//----------------------------------------------------------------------------
//
TInt CHidMouseDriver::SupportedFieldCount()
    {
    return iSupportedFieldCount;
    }
//----------------------------------------------------------------------------
// CHidMouseDriver::SetInputHandlingReg
//----------------------------------------------------------------------------
//
void CHidMouseDriver::SetInputHandlingReg(CHidInputDataHandlingReg* aHandlingReg)
    {
    iInputHandlingReg = aHandlingReg;
    }

void CHidMouseDriver::CursorRedraw()
    {
    TInt mouseStatus;

    TInt err = RProperty::Get( KPSUidBthidSrv, KBTMouseCursorState, mouseStatus );
    if ( !err )
        {
        err = RProperty::Set( KPSUidBthidSrv, KBTMouseCursorState, ECursorRedraw );
        DBG(RDebug::Print(
             _L("[BTHID]\tCHidMouseDriver::ForegroundEventL() X->ECursorRedraw") ) );
        }
    }
// ----------------------------------------------------------------------
