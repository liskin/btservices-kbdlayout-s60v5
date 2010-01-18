/*
* Copyright (c) 2004 - 2006 Nokia Corporation and/or its subsidiary(-ies).
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


// INCLUDE FILES


#include <eikenv.h>
#include <eikappui.h>
#include <eikapp.h>
#include <apparc.h>
#include <coemain.h>
#include <coecntrl.h>
#include <w32std.h>
#include <BTMouseBitMaps.mbg>
#include "clientimagecommander.h"

#ifndef DBG
#ifdef _DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif
#endif

_LIT( KBTMOUSEMBMFileName, "\\resource\\apps\\BTMouseBitMaps.mbm" );


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RImageCommander::RImageCommander()
// Create a new animation object.
// -----------------------------------------------------------------------------
//
EXPORT_C RImageCommander::RImageCommander( RAnimDll &aAnimDll, RWsSession& aWsSession ) :
        RAnim( aAnimDll ), iSession(aWsSession),
        iBaseBitmap(NULL),
        iBaseMaskBitmap(NULL),
        iMouseButtonPressed(EFalse),
        iWakeUpCalculator(0)
    {
    DBG(RDebug::Print(_L("RImageCommander::RImageCommander")));

    // No implementation required
    }

EXPORT_C RImageCommander::~RImageCommander( )
    {
    iSprite.Close();

    if ( iPointBufferQueue )
        {
        delete iPointBufferQueue;
        iPointBufferQueue = NULL;
        }

    if ( iBaseBitmap)
        {
        delete iBaseBitmap;
        iBaseBitmap = NULL;
        }

    if ( iBaseMaskBitmap )
        {
        delete iBaseMaskBitmap;
        iBaseMaskBitmap = NULL;
        }
    RAnim::Close();
    }

// -----------------------------------------------------------------------------
// RImageCommander::ImageConstruct()
// Construct a new animation object.
// -----------------------------------------------------------------------------
//
EXPORT_C void RImageCommander::ImageConstructL( RWindowGroup& aWindowGroup, TSize aSize )
    {
    // Send the parameters to the animation server object construction

    DBG(RDebug::Print(_L("RImageCommander::ImageConstructL IN")));

    iPointBufferQueue = NULL;
    iPointBufferQueue = CPointQueue::NewL(this,KMsgBTMouseBufferQueue);

    CreateSpriteL(aSize, aWindowGroup);

    DBG(RDebug::Print(_L("RImageCommander::ImageConstructL Construct Anim")));

    TPtrC8 des(NULL,0);
    RAnim::Construct( iSprite, 0, des );
    
    DBG(RDebug::Print(_L("RImageCommander::ImageConstructL OUT")));
    }

// ----------------------------------------------------------------------------
// RImageCommander::CreateSpriteL(TSize aSize, RWindowGroup& aGroup)
// ----------------------------------------------------------------------------
//
void RImageCommander::CreateSpriteL(TSize aSize, RWindowGroup& aGroup)
    {
    DBG(RDebug::Print(_L("RImageCommander::CreateSpriteL START")));
    TSpriteMember member;
    

    DBG(RDebug::Print(_L("RImageCommander::CreateSpriteL size %d,%d"), aSize.iWidth, aSize.iHeight));
    (void)aSize;

    iSprite=RWsSprite(iSession);
    TInt error = iSprite.Construct(aGroup, TPoint(),ESpriteNoChildClip|ESpriteNoShadows);
    User::LeaveIfError(error);
    iBaseBitmap = new (ELeave) CFbsBitmap;
    // Drive Letter:
    TFileName file(CEikonEnv::Static()->EikAppUi()->Application()->AppFullName().Left(2));
    // Path and file name:
    file.Append( KBTMOUSEMBMFileName ); // always safe

    User::LeaveIfError(iBaseBitmap->Load(file, EMbmBtmousebitmapsCursor));
    iBaseMaskBitmap = new (ELeave) CFbsBitmap;
    User::LeaveIfError(iBaseMaskBitmap->Load(file, EMbmBtmousebitmapsCursormask));
    FillInSpriteMember(member, iBaseBitmap, iBaseMaskBitmap);
    User::LeaveIfError(iSprite.AppendMember(member));
    
    DBG(RDebug::Print(_L("RImageCommander::CreateSpriteL END")));

    }

// ----------------------------------------------------------------------------
// RImageCommander::FillInSpriteMember(TSpriteMember& aMember, CFbsBitmap* icon, CFbsBitmap* iconMask)
// ----------------------------------------------------------------------------
//
void RImageCommander::FillInSpriteMember(TSpriteMember& aMember, CFbsBitmap* icon, CFbsBitmap* iconMask)
    {
    DBG(RDebug::Print(_L("RImageCommander::FillInSpriteMember"));

    if ( ( icon == NULL) || ( iconMask == NULL) )
        {
        RDebug::Print(_L("RImageCommander::FillInSpriteMember no icons !"));
        } 
    )
    aMember.iBitmap=icon;
    aMember.iMaskBitmap=iconMask;
    aMember.iInvertMask=EFalse;
    aMember.iDrawMode = CGraphicsContext::EDrawModeWriteAlpha;
    aMember.iOffset=TPoint(0,0);
    
    DBG(RDebug::Print(_L("RImageCommander::FillInSpriteMember END")));

    }

// -----------------------------------------------------------------------------
// RImageCommander::ImageCommand()
// Send a command (aCommand) to the animation server object.
// Because there is no way to return an error from the server side using this
// method, it is important that any server side code for these commands should
// not be able to fail or leave.  If the operation can fail, then the method
// TInt CommandReply(TInt aOpcode) should be used instead
// -----------------------------------------------------------------------------
//
EXPORT_C void RImageCommander::ImageCommand( TInt aCommand )
    {
    // Could optionally decode the commands here
    DBG(RDebug::Print(_L("RImageCommander::ImageCommand %d"), aCommand));

    RAnim::Command( aCommand );
    
    DBG(RDebug::Print(_L("RImageCommander::ImageCommand OUT")));

    }

TRawEvent::TType RImageCommander::PointerEventToRawEvent(TPointerEvent::TType aEventType)
    {
    TRawEvent::TType aType;
    switch(aEventType)
        {
        case TPointerEvent::EButton1Down:
            {
            aType=TRawEvent::EButton1Down;
            break;
            }
        case TPointerEvent::EButton1Up:
            {
            aType=TRawEvent::EButton1Up;
            break;
            }
        case TPointerEvent::EMove:
            {
            aType=TRawEvent::EPointerMove;
            break;
            }
        case TPointerEvent::ESwitchOn:
            {
            aType=TRawEvent::EPointerSwitchOn;
            break;
            }
        default:
            {
            aType=TRawEvent::ENone;
            break;
            }
        }
    return aType;
    }

TInt RImageCommander::SendPointerEvent(TPointerEvent::TType aEventType)
    {
    DBG(RDebug::Print(_L("RImageCommander::SendPointerEvent")));

    TRawEvent rawEvent;
    TPoint point(iCurrentPoint);
    TRawEvent::TType type = PointerEventToRawEvent(aEventType);
    rawEvent.Set(type, point.iX, point.iY, 30);
    typedef TPckgBuf< TRawEvent >  TRawEventDataPckg;
    TRawEventDataPckg rawEventPckg(rawEvent);
    RAnim::Command( KSendRawEvent, rawEventPckg );
    return KErrNone;
    }


TBool RImageCommander::RestrictPos()
    {
    TBool notInScreen(EFalse);
    TPixelsAndRotation sizeAndRotation;
    sizeAndRotation.iPixelSize.SetSize(360,640); // Just a guess
    CWsScreenDevice* screen(CEikonEnv::Static()->ScreenDevice());
    if (screen)
        {
        TInt mode(screen->CurrentScreenMode());
        screen->GetScreenModeSizeAndRotation(mode, sizeAndRotation);
        }
    TRect validRect(sizeAndRotation.iPixelSize);
    if (! validRect.Contains(iCurrentPoint))
        {
        notInScreen = ETrue;
        if (iCurrentPoint.iX<0)
            iCurrentPoint.iX=0;
        else if (iCurrentPoint.iX>=validRect.iBr.iX)
            iCurrentPoint.iX=validRect.iBr.iX-1;

        if (iCurrentPoint.iY<0)
            iCurrentPoint.iY=0;
        else if (iCurrentPoint.iY>=validRect.iBr.iY)
            iCurrentPoint.iY=validRect.iBr.iY-1;
        }
    return notInScreen;
    }

inline TBool RImageCommander::CheckCurrentPoint()
    {
    DBG(TPoint pos(iCurrentPoint));

    TBool outSide(RestrictPos());
    
    DBG(
    if (outSide)
        {
       RDebug::Print(_L("RImageCommander::CheckCurrentPoint not in screen (%d,%d)"), pos.iX, pos.iY);
        }
    )
    return outSide;
    }

TInt RImageCommander::SendWakeUp()
    {
    TRawEvent rawEvent;
    rawEvent.Set(TRawEvent::ESwitchOn);
    return UserSvr::AddEvent(rawEvent);
    }

void RImageCommander::HandlePointL(const TPoint& aPoint)
    {
    DBG(RDebug::Print(_L("RImageCommander::HandlePointL pos (%d,%d)"),
                  aPoint.iX, aPoint.iY));

    if ( ( aPoint.iX != 0) || ( aPoint.iY != 0) )
        {
        iCurrentPoint += aPoint;
        TBool notInScreen(CheckCurrentPoint());
        if (iMouseButtonPressed)
            {
            // Dragging possible now:
            if (notInScreen)
                {
                SendPointerEvent(TPointerEvent::EButton1Up);
                iMouseButtonPressed = EFalse;
                }
            else
                {
                SendPointerEvent(TPointerEvent::EMove);
                }
            }
        else
            {
            typedef TPckgBuf< TPoint >  TPointDataPckg;
            TPointDataPckg pointPckg(iCurrentPoint);
            RAnim::Command( KChangeCursor, pointPckg );
            }
        }
    iWakeUpCalculator++;
    if (iWakeUpCalculator > KWakeUpNeeded)
        {
        SendWakeUp();
        iWakeUpCalculator = 0;
        }
    }

void RImageCommander::SendEventL(TPointerEvent::TType aEventType)
    {
    DBG(RDebug::Print(_L("RImageCommander::SendEventL (%d)"), aEventType));

    if (aEventType == TPointerEvent::EButton1Down)
        {
        iMouseButtonPressed = ETrue;
        SendWakeUp();
        }
    else if (aEventType == TPointerEvent::EButton1Up)
        {
        iMouseButtonPressed = EFalse;
        }
    SendPointerEvent(aEventType);
    }

// ======== class CPointQueue========
//
// ---------------------------------------------------------------------------
// CPointQueue::CPointQueue
// Constructor. Adds the active object to the active scheduler
// and issues a request for server notification.
// ---------------------------------------------------------------------------
//

CPointQueue::CPointQueue(MPointHandler* aHandler):CActive( EPriorityHigh ),
        iHandler(aHandler)
    {
    // issue asynchronous request and set this object active
    DBG(RDebug::Print(_L("(CCPointQueue::CPointQueue")));

    CActiveScheduler::Add( this );

    DBG( RDebug::Print(_L("(CCPointQueue::CPointQueue Out")));

    }

// ---------------------------------------------------------------------------
// CPointQueue::NewL
// factory constructor.
// ---------------------------------------------------------------------------
//
CPointQueue* CPointQueue::NewL(MPointHandler* aHandler,const TDesC& aName)
    {
    DBG(RDebug::Print(_L("(CCPointQueue::NewL")));

    CPointQueue* queue = new(ELeave)CPointQueue(aHandler);
    CleanupStack::PushL(queue);
    queue->ConstructL(aName);
    CleanupStack::Pop(queue);
    // Start receiving:
    queue->GetPoint();
    
    DBG(RDebug::Print(_L("(CCPointQueue::NewL Out")));

    return queue;
    }

// ---------------------------------------------------------------------------
// CPointQueue::ConstructL
// 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CPointQueue::ConstructL(const TDesC& aName)
    {
    (void) aName;
    //Open message queue
    TInt err = iPointBufQueue.OpenGlobal(KMsgBTMouseBufferQueue);   
    if (err == KErrNotFound)
        {
        User::LeaveIfError(iPointBufQueue.CreateGlobal(KMsgBTMouseBufferQueue, KPointQueueLen));    
        }
    else 
        {
        User::LeaveIfError( err );
        } 
    }
// ---------------------------------------------------------------------------
// CPointQueue::CPointQueue
// Destructor. Cancels any outstanding requests
// ---------------------------------------------------------------------------
//
CPointQueue::~CPointQueue()
    {
    Cancel();
    iPointBufQueue.Close();
    }

// ---------------------------------------------------------------------------
// CPointQueue::DoCancel
// Cancels the notification requests
// ---------------------------------------------------------------------------
//
void CPointQueue::DoCancel()
    {
    iPointBufQueue.CancelDataAvailable();
    }

// ---------------------------------------------------------------------------
// CPointQueue::RunL
// ---------------------------------------------------------------------------
//
void CPointQueue::RunL()
    {
    DBG(RDebug::Print(_L("CCPointQueue::RunL")));

    TInt ret;
    //iHandler is always there. No need to check it
    TPointBuffer points;
    ret = iPointBufQueue.Receive(points);
    DBG(RDebug::Print(_L("CCPointQueue::RunL , ret = %d, points %d"), ret, points.iNum));

    if (ret == KErrNone)
        {
        TPoint pos(0,0);
        for(TInt ii = 0; ii < points.iNum; ii++)
            {
            pos += points.iPoint[ii];
            if ( ( points.iType[ii] == KBufferPenDown ) ||
                    ( points.iType[ii] == KBufferPenUp ) )
                {
                iHandler->SendEventL( ( points.iType[ii] == KBufferPenDown ) ?
                                      TPointerEvent::EButton1Down :
                                      TPointerEvent::EButton1Up );
                iHandler->HandlePointL(pos);
                pos.SetXY(0,0);
                }
            }
        iHandler->HandlePointL(pos);
        }
    // re-issue request
    GetPoint();
    }

// ---------------------------------------------------------------------------
// CPointQueue::RunError
// ---------------------------------------------------------------------------
//
TInt CPointQueue::RunError(TInt /*aError*/)
    {
    DBG(RDebug::Print(_L("CCPointQueue::RunError")));
    // re-issue request
    GetPoint();
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CPointQueue::GetPoint
// Reissue request for point
// ---------------------------------------------------------------------------
//
void CPointQueue::GetPoint()
    {
    DBG(RDebug::Print(_L("CCPointQueue::GetPoint")));

    if(!IsActive()) 
        {
        iPointBufQueue.NotifyDataAvailable(iStatus);
        SetActive();
        }
    DBG(RDebug::Print(_L("CCPointQueue::GetPoint Out")));
    }
// End of File
