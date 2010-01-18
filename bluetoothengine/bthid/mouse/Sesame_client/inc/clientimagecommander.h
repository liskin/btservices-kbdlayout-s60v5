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


#ifndef __CLIENTIMAGE_COMMANDER_H__
#define __CLIENTIMAGE_COMMANDER_H__

// INCLUDES
#include <w32adll.h>

#include "pointmsgqueue.h"
// CLASS DECLARATION

#define KWakeUpNeeded (5)

class CPointQueue;

class MPointHandler
    {
public:
    /**
     * Handle Points from AnimDll
     */
    virtual void HandlePointL(const TPoint& aPoint) = 0;
    virtual void SendEventL(TPointerEvent::TType aEventType) = 0;
    };

/**
 * RImageCommander
 * An instance of RImageCommander is an Animation Client Commander which
 * sends commands to the Animation Server
 */
class RImageCommander : public RAnim, public MPointHandler
    {
public:
    // Constructors

    /**
     * RImageCommander.
     * Construct an Animation Client object for Animation Client DLL aAnimDll
     * @param aAnimDll the Animation Client DLL to use
     */
    IMPORT_C RImageCommander(RAnimDll& aAnimDll, RWsSession& aWsSession);

    /**
     * ImageConstruct.
     * Complete Animation Client construction
     */
    IMPORT_C void ImageConstructL(RWindowGroup& aWindowGroup, TSize aSize);

    /**
     * ~RImageCommander
     */
    IMPORT_C ~RImageCommander();

public:
    // from MPointHandler

    void HandlePointL(const TPoint& aPoint);
    void SendEventL(TPointerEvent::TType aEventType);
private:
    TInt SendPointerEvent(TPointerEvent::TType aEventType);
    TRawEvent::TType PointerEventToRawEvent(TPointerEvent::TType aEventType);
    void CreateSpriteL(TSize aSize, RWindowGroup& aGroup);
    void FillInSpriteMember(TSpriteMember& aMember, CFbsBitmap* icon,
            CFbsBitmap* iconMask);
    TBool CheckCurrentPoint();
    TInt SendWakeUp();
    TBool RestrictPos();

public:
    // New functions

    /**
     * ImageCommand.
     * Send aCommand to the Animation Server object;
     * aCommand == KChangeCursor implies "change animation cursor".
     * Note! Because there is no way to return an error from the server side
     * using this method, it is important that any server side code for these
     * commands should not be able to fail or leave.
     * This command is also buffered and may not be performed immediately.
     * @param aCommand the enumerated code for the option selected
     */
    IMPORT_C void ImageCommand(TInt aCommand);

public:
    // Enumerations

    /**
     * KAnimationTypes.
     * Constant used to indicate the animation of a bouncing square should
     * be created, enumeration can be expanded
     */
    enum KAnimationTypes
        {
        KAnimationSquare = 1
        };

    
private:

    RWsSession& iSession;
    RWsSprite iSprite;
    CFbsBitmap* iBaseBitmap;
    CFbsBitmap* iBaseMaskBitmap;

    TPoint iCurrentPoint;
    CPointQueue* iPointBufferQueue;
    TBool iMouseButtonPressed;
    TInt iWakeUpCalculator;
    };

/**
 * Class CPointQueue
 * Event message queue. It's an active object.
 *
 * @since S60 v4.0
 */

class CPointQueue : public CActive
    {
public:
    /**
     * factory constructor. 
     *
     * @since S60 v4.0
     * @param aHandler The event handler.
     * @param aName The event queue global name
     * @return The created messge queue object
     */
    static CPointQueue* NewL(MPointHandler* aHandler, const TDesC& aName);
    /**
     * desstructor. 
     *
     * @since S60 v4.0
     */
    ~CPointQueue();

    /**
     * Request event.
     *
     * @since S60 v4.0
     */
    void GetPoint();

protected:
    /**
     * 2nd phase constructor.
     *
     * @since S60 v4.0
     * @aName The message queue name
     */
    void ConstructL(const TDesC& aName);
    /**
     * Default construcotr.
     *
     * @since S60 v4.0
     * @aHandler The event handler
     */
    CPointQueue(MPointHandler* aHandler);

    //From CActive
    /**
     * From CActive
     * Called when there is event in the queue
     *
     * @since S60 v4.0
     */
    void RunL();

    /**
     * From CActive
     * will be called if RunL leaves
     *
     * @since S60 v4.0
     * @param aError The error number
     */
    TInt RunError(TInt aError);

    /**
     * From CActive
     * will be called when Cancel is issued
     *
     * @since S60 v4.0
     */
    void DoCancel();

private:
    /**
     * Message buffer queue
     */
    RMsgQueue<TPointBuffer> iPointBufQueue;

    /**
     * Event handler
     */
    MPointHandler* iHandler;
    };

#endif // __CLIENTIMAGE_COMMANDER_H__
// End of File
